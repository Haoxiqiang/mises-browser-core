// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/api/messaging/message_service.h"

#include <stdint.h>

#include <limits>
#include <memory>
#include <utility>

#include "base/bind.h"
#include "base/callback.h"
#include "base/containers/contains.h"
#include "base/json/json_writer.h"
#include "base/lazy_instance.h"
#include "base/metrics/field_trial_params.h"
#include "base/metrics/histogram_macros.h"
#include "base/values.h"
#include "build/build_config.h"
#include "components/back_forward_cache/back_forward_cache_disable.h"
#include "content/public/browser/back_forward_cache.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/site_instance.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/child_process_host.h"
#include "content/public/common/content_features.h"
#include "extensions/browser/api/extensions_api_client.h"
#include "extensions/browser/api/messaging/channel_endpoint.h"
#include "extensions/browser/api/messaging/extension_message_port.h"
#include "extensions/browser/api/messaging/message_port.h"
#include "extensions/browser/api/messaging/messaging_delegate.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_api_frame_id_map.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/extension_util.h"
#include "extensions/browser/extension_web_contents_observer.h"
#include "extensions/browser/extensions_browser_client.h"
#include "extensions/browser/guest_view/web_view/web_view_guest.h"
#include "extensions/browser/pref_names.h"
#include "extensions/browser/process_manager.h"
#include "extensions/common/api/messaging/messaging_endpoint.h"
#include "extensions/common/api/messaging/port_context.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"
#include "extensions/common/extension_features.h"
#include "extensions/common/manifest_constants.h"
#include "extensions/common/manifest_handlers/background_info.h"
#include "extensions/common/manifest_handlers/externally_connectable.h"
#include "extensions/common/manifest_handlers/incognito_info.h"
#include "extensions/common/permissions/permissions_data.h"
#include "net/http/http_response_headers.h"
#include "third_party/blink/public/common/origin_trials/trial_token_validator.h"
#include "url/gurl.h"

using content::BrowserContext;
using content::BrowserThread;
using content::SiteInstance;
using content::WebContents;

namespace extensions {

namespace {

const char kReceivingEndDoesntExistError[] =
    "Could not establish connection. Receiving end does not exist.";
#if BUILDFLAG(IS_WIN) || BUILDFLAG(IS_MAC) || BUILDFLAG(IS_LINUX) || \
    BUILDFLAG(IS_CHROMEOS) || BUILDFLAG(IS_ANDROID)
const char kMissingPermissionError[] =
    "Access to native messaging requires nativeMessaging permission.";
const char kProhibitedByPoliciesError[] =
    "Access to the native messaging host was disabled by the system "
    "administrator.";
#endif

LazyContextId LazyContextIdFor(content::BrowserContext* browser_context,
                               const Extension* extension) {
  if (BackgroundInfo::HasLazyBackgroundPage(extension))
    return LazyContextId(browser_context, extension->id());

  DCHECK(BackgroundInfo::IsServiceWorkerBased(extension));
  return LazyContextId(browser_context, extension->id(), extension->url());
}

const Extension* GetExtensionForNativeAppChannel(
    const ChannelEndpoint& source) {
  DCHECK(!source.is_for_native_host());

  if (source.is_for_service_worker()) {
    const ExtensionId& extension_id =
        source.port_context().worker->extension_id;
    return ExtensionRegistry::Get(source.browser_context())
        ->enabled_extensions()
        .GetByID(extension_id);
  }

  DCHECK(source.is_for_render_frame());
  content::RenderFrameHost* source_rfh = source.GetRenderFrameHost();
  if (!source_rfh)
    return nullptr;
  content::WebContents* web_contents =
      content::WebContents::FromRenderFrameHost(source_rfh);
  if (!web_contents)
    return nullptr;
  ExtensionWebContentsObserver* extension_web_contents_observer =
      ExtensionWebContentsObserver::GetForWebContents(web_contents);
  if (!extension_web_contents_observer)
    return nullptr;
  return extension_web_contents_observer->GetExtensionFromFrame(source_rfh,
                                                                true);
}

bool IsExtensionMessageSupportedInBackForwardCache() {
  if (!content::BackForwardCache::IsBackForwardCacheFeatureEnabled())
    return false;
  static const bool is_extension_message_supported =
      base::FeatureParam<bool>(&features::kBackForwardCache,
                               "extension_message_supported", true)
          .Get();
  return is_extension_message_supported;
}

// Disables the back forward for `host` if the current configuration does not
// support extension messaging APIs.
void MaybeDisableBackForwardCacheForMessaging(content::RenderFrameHost* host) {
  if (!host || IsExtensionMessageSupportedInBackForwardCache())
    return;
  content::BackForwardCache::DisableForRenderFrameHost(
      host, back_forward_cache::DisabledReason(
                back_forward_cache::DisabledReasonId::kExtensionMessaging));
}

}  // namespace

struct MessageService::MessageChannel {
  std::unique_ptr<MessagePort> opener;
  std::unique_ptr<MessagePort> receiver;
};

struct MessageService::OpenChannelParams {
  ChannelEndpoint source;
  std::unique_ptr<base::DictionaryValue> source_tab;
  ExtensionApiFrameIdMap::FrameData source_frame;
  std::unique_ptr<MessagePort> receiver;
  PortId receiver_port_id;
  MessagingEndpoint source_endpoint;
  std::unique_ptr<MessagePort> opener_port;
  std::string target_extension_id;
  GURL source_url;
  absl::optional<url::Origin> source_origin;
  std::string channel_name;
  bool include_guest_process_info;

  // Takes ownership of receiver.
  OpenChannelParams(const ChannelEndpoint& source,
                    std::unique_ptr<base::DictionaryValue> source_tab,
                    const ExtensionApiFrameIdMap::FrameData& source_frame,
                    MessagePort* receiver,
                    const PortId& receiver_port_id,
                    const MessagingEndpoint& source_endpoint,
                    std::unique_ptr<MessagePort> opener_port,
                    const std::string& target_extension_id,
                    const GURL& source_url,
                    absl::optional<url::Origin> source_origin,
                    const std::string& channel_name,
                    bool include_guest_process_info)
      : source(source),
        source_tab(std::move(source_tab)),
        source_frame(source_frame),
        receiver(receiver),
        receiver_port_id(receiver_port_id),
        source_endpoint(source_endpoint),
        opener_port(std::move(opener_port)),
        target_extension_id(target_extension_id),
        source_url(source_url),
        source_origin(source_origin),
        channel_name(channel_name),
        include_guest_process_info(include_guest_process_info) {}

  OpenChannelParams(const OpenChannelParams&) = delete;
  OpenChannelParams& operator=(const OpenChannelParams&) = delete;
};

MessageService::MessageService(BrowserContext* context)
    : context_(context),
      messaging_delegate_(ExtensionsAPIClient::Get()->GetMessagingDelegate()) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK_NE(nullptr, messaging_delegate_);
}

MessageService::~MessageService() {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  channels_.clear();
}

static base::LazyInstance<
    BrowserContextKeyedAPIFactory<MessageService>>::DestructorAtExit g_factory =
    LAZY_INSTANCE_INITIALIZER;

// static
BrowserContextKeyedAPIFactory<MessageService>*
MessageService::GetFactoryInstance() {
  return g_factory.Pointer();
}

// static
MessageService* MessageService::Get(BrowserContext* context) {
  return BrowserContextKeyedAPIFactory<MessageService>::Get(context);
}

void MessageService::OpenChannelToExtension(
    const ChannelEndpoint& source,
    const PortId& source_port_id,
    const MessagingEndpoint& source_endpoint,
    std::unique_ptr<MessagePort> opener_port,
    const std::string& target_extension_id,
    const GURL& source_url,
    const std::string& channel_name) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(source_port_id.is_opener);
  DCHECK(!target_extension_id.empty());
  DCHECK(source_endpoint.extension_id.has_value() ||
         source_endpoint.type == MessagingEndpoint::Type::kTab ||
         source_endpoint.type == MessagingEndpoint::Type::kNativeApp);
  DCHECK_EQ(source_endpoint.native_app_name.has_value(),
            source_endpoint.type == MessagingEndpoint::Type::kNativeApp);
  int source_process_id = source.render_process_id();
  DCHECK_EQ(source_process_id == content::ChildProcessHost::kInvalidUniqueID,
            source_endpoint.type == MessagingEndpoint::Type::kNativeApp);
  content::RenderFrameHost* source_render_frame_host =
      source.is_for_render_frame() ? source.GetRenderFrameHost() : nullptr;
  if (!source.IsValid())
    return;
  BrowserContext* context = source.browser_context();
  DCHECK(ExtensionsBrowserClient::Get()->IsSameContext(context, context_));

  MaybeDisableBackForwardCacheForMessaging(source_render_frame_host);

  if (!opener_port) {
    DCHECK(source_endpoint.type == MessagingEndpoint::Type::kTab ||
           source_endpoint.type == MessagingEndpoint::Type::kExtension);
    opener_port = ExtensionMessagePort::CreateForEndpoint(
        weak_factory_.GetWeakPtr(), source_port_id,
        source_endpoint.extension_id ? *source_endpoint.extension_id
                                     : ExtensionId(),
        source);
  }
  if (!opener_port->IsValidPort())
    return;

  ExtensionRegistry* registry = ExtensionRegistry::Get(context);
  const Extension* target_extension =
      registry->enabled_extensions().GetByID(target_extension_id);
  if (!target_extension) {
    opener_port->DispatchOnDisconnect(kReceivingEndDoesntExistError);
    return;
  }

  bool is_web_connection = false;

  if ((source_endpoint.type == MessagingEndpoint::Type::kTab ||
       source_endpoint.type == MessagingEndpoint::Type::kExtension) &&
      source_endpoint.extension_id != target_extension_id) {
    // It's an external connection. Check the externally_connectable manifest
    // key if it's present. If it's not, we allow connection from any extension
    // but not webpages.
    ExternallyConnectableInfo* externally_connectable =
        static_cast<ExternallyConnectableInfo*>(
            target_extension->GetManifestData(
                manifest_keys::kExternallyConnectable));
    bool is_externally_connectable = false;

    if (externally_connectable) {
      if (source_endpoint.extension_id) {
        // The source was an extension or a content script. Check that the
        // extension ID matches.
        is_externally_connectable =
            externally_connectable->IdCanConnect(*source_endpoint.extension_id);
      } else {
        DCHECK(source_render_frame_host);

        is_web_connection = true;

        // Sites can only connect to the CryptoToken component extension if it
        // has been enabled via feature flag or deprecation trial.
        // TODO(1224886): Delete together with CryptoToken code.
        if (target_extension_id == extension_misc::kCryptotokenExtensionId) {
          blink::TrialTokenValidator validator;
          const net::HttpResponseHeaders* response_headers =
              source_render_frame_host->GetLastResponseHeaders();
          const bool u2f_api_enabled =
              base::FeatureList::IsEnabled(
                  extensions_features::kU2FSecurityKeyAPI) ||
              (response_headers &&
               validator.RequestEnablesFeature(
                   source_render_frame_host->GetLastCommittedURL(),
                   response_headers,
                   extension_misc::kCryptotokenDeprecationTrialName,
                   base::Time::Now()));
          is_externally_connectable =
              u2f_api_enabled &&
              externally_connectable->matches.MatchesURL(
                  source_render_frame_host->GetLastCommittedURL());
        } else {
          // Check that the web page URL matches.
          is_externally_connectable =
              externally_connectable->matches.MatchesURL(
                  source_render_frame_host->GetLastCommittedURL());
        }
      }
    } else {
      // Default behaviour. Any extension or content script, no webpages.
      is_externally_connectable = source_endpoint.extension_id.has_value();
    }

    if (!is_externally_connectable) {
      // Important: use kReceivingEndDoesntExistError here so that we don't
      // leak information about this extension to callers. This way it's
      // indistinguishable from the extension just not existing.
      opener_port->DispatchOnDisconnect(kReceivingEndDoesntExistError);
      return;
    }
  }

  WebContents* source_contents = nullptr;
  if (source_render_frame_host) {
    source_contents =
        WebContents::FromRenderFrameHost(source_render_frame_host);
  }

  ExtensionApiFrameIdMap::FrameData source_frame;
  bool include_guest_process_info = false;

  // Get information about the opener's tab, if applicable.
  std::unique_ptr<base::DictionaryValue> source_tab =
      messaging_delegate_->MaybeGetTabInfo(source_contents);

  absl::optional<url::Origin> source_origin;
  if (source_render_frame_host)
    source_origin = source_render_frame_host->GetLastCommittedOrigin();

  if (source_tab.get()) {
    DCHECK(source_render_frame_host);
    source_frame = ExtensionApiFrameIdMap::Get()->GetFrameData(
        source_render_frame_host->GetGlobalId());
  } else {
    // Check to see if it was a WebView making the request.
    // Sending messages from WebViews to extensions breaks webview isolation,
    // so only allow component extensions to receive messages from WebViews.
    bool is_web_view = !!WebViewGuest::FromWebContents(source_contents);
    if (is_web_view &&
        Manifest::IsComponentLocation(target_extension->location())) {
      include_guest_process_info = true;
    }
  }

  std::unique_ptr<OpenChannelParams> params =
      std::make_unique<OpenChannelParams>(
          source, std::move(source_tab), source_frame, nullptr,
          source_port_id.GetOppositePortId(), source_endpoint,
          std::move(opener_port), target_extension_id, source_url,
          std::move(source_origin), channel_name, include_guest_process_info);
  pending_incognito_channels_[params->receiver_port_id.GetChannelId()] =
      PendingMessagesQueue();
  if (context->IsOffTheRecord() &&
      !util::IsIncognitoEnabled(target_extension_id, context)) {
    // Give the user a chance to accept an incognito connection from the web if
    // they haven't already, with the conditions:
    // - Only for non-split mode incognito. We don't want the complication of
    //   spinning up an additional process here which might need to do some
    //   setup that we're not expecting.
    // - Only for extensions that can't normally be enabled in incognito, since
    //   that surface (e.g. chrome://extensions) should be the only one for
    //   enabling in incognito. In practice this means platform apps only.
    if (!is_web_connection || IncognitoInfo::IsSplitMode(target_extension) ||
        util::CanBeIncognitoEnabled(target_extension)) {
      OnOpenChannelAllowed(std::move(params), false);
      return;
    }

    // If the target extension isn't even listening for connect/message events,
    // there is no need to go any further and the connection should be
    // rejected without showing a prompt. See http://crbug.com/442497
    EventRouter* event_router = EventRouter::Get(context);
    const char* const events[] = {
        "runtime.onConnectExternal", "runtime.onConnectNative",
        "runtime.onMessageExternal", "extension.onRequestExternal", nullptr};
    bool has_event_listener = false;
    for (const char* const* event = events; *event; event++) {
      has_event_listener |=
          event_router->ExtensionHasEventListener(target_extension_id, *event);
    }
    if (!has_event_listener) {
      OnOpenChannelAllowed(std::move(params), false);
      return;
    }

    // This check may show a dialog.
    messaging_delegate_->QueryIncognitoConnectability(
        context, target_extension, source_contents, source_url,
        base::BindOnce(&MessageService::OnOpenChannelAllowed,
                       weak_factory_.GetWeakPtr(), std::move(params)));
    return;
  }

  OnOpenChannelAllowed(std::move(params), true);
}

void MessageService::OpenChannelToNativeApp(
    const ChannelEndpoint& source,
    const PortId& source_port_id,
    const std::string& native_app_name) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(source_port_id.is_opener);

  if (!source.IsValid())
    return;
  const Extension* extension = GetExtensionForNativeAppChannel(source);

  if (!extension)
    return;

  std::unique_ptr<ExtensionMessagePort> opener_port =
      ExtensionMessagePort::CreateForEndpoint(
          weak_factory_.GetWeakPtr(), source_port_id, extension->id(), source);
  if (!opener_port->IsValidPort())
    return;

#if BUILDFLAG(IS_WIN) || BUILDFLAG(IS_MAC) || BUILDFLAG(IS_LINUX) || \
    BUILDFLAG(IS_CHROMEOS) || BUILDFLAG(IS_ANDROID)
  bool has_permission = extension->permissions_data()->HasAPIPermission(
      mojom::APIPermissionID::kNativeMessaging);
  if (!has_permission) {
    opener_port->DispatchOnDisconnect(kMissingPermissionError);
    return;
  }

  // Verify that the host is not blocked by policies.
  BrowserContext* source_context = source.browser_context();
  DCHECK(
      ExtensionsBrowserClient::Get()->IsSameContext(source_context, context_));
  MessagingDelegate::PolicyPermission policy_permission =
      messaging_delegate_->IsNativeMessagingHostAllowed(source_context,
                                                        native_app_name);
  if (policy_permission == MessagingDelegate::PolicyPermission::DISALLOW) {
    opener_port->DispatchOnDisconnect(kProhibitedByPoliciesError);
    return;
  }

  std::unique_ptr<MessageChannel> channel = std::make_unique<MessageChannel>();
  channel->opener = std::move(opener_port);
  channel->opener->OpenPort(source.render_process_id(), source.port_context());

  content::RenderFrameHost* source_rfh =
      source.is_for_render_frame() ? source.GetRenderFrameHost() : nullptr;

  MaybeDisableBackForwardCacheForMessaging(source_rfh);

  std::string error = kReceivingEndDoesntExistError;
  const PortId receiver_port_id = source_port_id.GetOppositePortId();
  // NOTE: We're creating |receiver| with nullptr |source_rfh|, which seems to
  // work for native messaging tests. This might need further checking in case
  // any issues arise from it.
  std::unique_ptr<MessagePort> receiver(
      messaging_delegate_->CreateReceiverForNativeApp(
          context_, weak_factory_.GetWeakPtr(), source_rfh, extension->id(),
          receiver_port_id, native_app_name,
          policy_permission == MessagingDelegate::PolicyPermission::ALLOW_ALL,
          &error));

  if (!receiver.get()) {
    // Abandon the channel.
    channel->opener->DispatchOnDisconnect(error);
    return;
  }

  channel->receiver = std::move(receiver);

  // Keep the opener alive until the channel is closed.
  channel->opener->IncrementLazyKeepaliveCount(
      true /* is_for_native_message_connect */);

  AddChannel(std::move(channel), receiver_port_id);
#else   // !(BUILDFLAG(IS_WIN) || BUILDFLAG(IS_MAC) || BUILDFLAG(IS_LINUX) ||
        // BUILDFLAG(IS_CHROMEOS))
  const char kNativeMessagingNotSupportedError[] =
      "Native Messaging is not supported on this platform.";
  opener_port->DispatchOnDisconnect(kNativeMessagingNotSupportedError);
#endif  // !(BUILDFLAG(IS_WIN) || BUILDFLAG(IS_MAC) || BUILDFLAG(IS_LINUX) ||
        // BUILDFLAG(IS_CHROMEOS))
}

void MessageService::OpenChannelToTab(const ChannelEndpoint& source,
                                      const PortId& source_port_id,
                                      int tab_id,
                                      int frame_id,
                                      const std::string& document_id,
                                      const std::string& extension_id,
                                      const std::string& channel_name) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK_GE(frame_id, -1);
  DCHECK(source_port_id.is_opener);

  // RenderFrameHost or the worker thread might be gone.
  if (!source.IsValid())
    return;

  std::unique_ptr<ExtensionMessagePort> opener_port =
      ExtensionMessagePort::CreateForEndpoint(
          weak_factory_.GetWeakPtr(), source_port_id, extension_id, source);
  if (!opener_port->IsValidPort())
    return;

  BrowserContext* source_context = source.browser_context();
  DCHECK(
      ExtensionsBrowserClient::Get()->IsSameContext(source_context, context_));
  content::WebContents* receiver_contents =
      messaging_delegate_->GetWebContentsByTabId(source_context, tab_id);
  if (!receiver_contents || receiver_contents->GetController().NeedsReload()) {
    // The tab isn't loaded yet. Don't attempt to connect.
    opener_port->DispatchOnDisconnect(kReceivingEndDoesntExistError);
    return;
  }

  MaybeDisableBackForwardCacheForMessaging(
      receiver_contents->GetPrimaryMainFrame());

  const PortId receiver_port_id = source_port_id.GetOppositePortId();
  std::unique_ptr<MessagePort> receiver =
      messaging_delegate_->CreateReceiverForTab(
          weak_factory_.GetWeakPtr(), extension_id, receiver_port_id,
          receiver_contents, frame_id, document_id);
  if (!receiver.get()) {
    opener_port->DispatchOnDisconnect(kReceivingEndDoesntExistError);
    return;
  }

  const Extension* extension = nullptr;
  if (!extension_id.empty()) {
    // Source extension == target extension so the extension must exist, or
    // where did the IPC come from?
    extension = ExtensionRegistry::Get(source_context)
                    ->enabled_extensions()
                    .GetByID(extension_id);
    DCHECK(extension);
  }

  BrowserContext* receiver_context = receiver_contents->GetBrowserContext();
  DCHECK(ExtensionsBrowserClient::Get()->IsSameContext(receiver_context,
                                                       context_));
  std::unique_ptr<OpenChannelParams> params =
      std::make_unique<OpenChannelParams>(
          source,
          std::unique_ptr<base::DictionaryValue>(),  // Source tab doesn't make
                                                     // sense
                                                     // for opening to tabs.
          ExtensionApiFrameIdMap::FrameData(),       // There is no frame.
          receiver.release(), receiver_port_id,
          MessagingEndpoint::ForExtension(extension_id), std::move(opener_port),
          extension_id,
          GURL(),         // Source URL doesn't make sense for opening to tabs.
          url::Origin(),  // Origin URL doesn't make sense for opening to tabs.
          channel_name,
          false);  // Connections to tabs aren't webview guests.
  OpenChannelImpl(receiver_context, std::move(params), extension,
                  false /* did_enqueue */);
}

void MessageService::OpenChannelImpl(BrowserContext* browser_context,
                                     std::unique_ptr<OpenChannelParams> params,
                                     const Extension* target_extension,
                                     bool did_enqueue) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(
      ExtensionsBrowserClient::Get()->IsSameContext(browser_context, context_));
  DCHECK_EQ(target_extension != nullptr, !params->target_extension_id.empty());

  // Check whether the source got closed while in flight.
  const ChannelEndpoint& source = params->source;

  bool will_open_channel = true;

  if (!source.IsValid()) {  // Closed while in flight.
    will_open_channel = false;
  }
  if (!params->opener_port->IsValidPort()) {
    will_open_channel = false;
  }
  if (!params->receiver || !params->receiver->IsValidPort()) {
    will_open_channel = false;
    params->opener_port->DispatchOnDisconnect(kReceivingEndDoesntExistError);
  }

  if (!will_open_channel) {
    // The channel won't open. If this was a pending channel, remove it,
    // because now it will never open. This prevents the pending message
    // from being re-added indefinitely.  See https://crbug.com/1231683.
    // TODO(crbug.com/1296492): This probably isn't the best solution.
    // Ideally, we should close the channel before we get to this point
    // if there's  no chance it will ever open, remove it from pending
    // channels, and then only try to open the pending channel if it's
    // still valid.
    pending_lazy_context_channels_.erase(
        params->receiver_port_id.GetChannelId());
    return;
  }

  const PortContext& port_context = source.port_context();
  params->opener_port->OpenPort(source.render_process_id(), port_context);
  params->opener_port->RevalidatePort();

  params->receiver->RemoveCommonFrames(*params->opener_port);
  if (!params->receiver->IsValidPort()) {
    params->opener_port->DispatchOnDisconnect(kReceivingEndDoesntExistError);
    return;
  }

  std::unique_ptr<MessageChannel> channel_ptr =
      std::make_unique<MessageChannel>();
  MessageChannel* channel = channel_ptr.get();
  channel->opener = std::move(params->opener_port);
  channel->receiver = std::move(params->receiver);
  AddChannel(std::move(channel_ptr), params->receiver_port_id);

  int guest_process_id = content::ChildProcessHost::kInvalidUniqueID;
  int guest_render_frame_routing_id = MSG_ROUTING_NONE;
  if (params->include_guest_process_info &&
      // TODO(lazyboy): Investigate <webview> SW messaging.
      source.is_for_render_frame()) {
    guest_process_id = params->source.render_process_id();
    DCHECK(port_context.frame);
    guest_render_frame_routing_id = port_context.frame->routing_id;

    DCHECK(WebViewGuest::FromWebContents(
        WebContents::FromRenderFrameHost(source.GetRenderFrameHost())));
  }

  // Send the connect event to the receiver.  Give it the opener's port ID (the
  // opener has the opposite port ID).
  channel->receiver->DispatchOnConnect(
      params->channel_name, std::move(params->source_tab), params->source_frame,
      guest_process_id, guest_render_frame_routing_id, params->source_endpoint,
      params->target_extension_id, params->source_url, params->source_origin);

  // Report the event to the event router, if the target is an extension.
  //
  // First, determine what event this will be (runtime.onConnect vs
  // runtime.onMessage etc), and what the event target is (view vs background
  // page etc).
  //
  // Yes - even though this is opening a channel, they may actually be
  // runtime.onRequest/onMessage events because those single-use events are
  // built using the connect framework (see messaging.js).
  if (target_extension) {
    events::HistogramValue histogram_value = events::UNKNOWN;
    bool is_external =
        (params->source_endpoint.type == MessagingEndpoint::Type::kExtension ||
         params->source_endpoint.type == MessagingEndpoint::Type::kTab) &&
        params->source_endpoint.extension_id != params->target_extension_id;
    if (params->source_endpoint.type == MessagingEndpoint::Type::kNativeApp) {
      histogram_value = events::RUNTIME_ON_CONNECT_NATIVE;
    } else if (params->channel_name == "chrome.runtime.onRequest") {
      histogram_value = is_external ? events::RUNTIME_ON_REQUEST_EXTERNAL
                                    : events::RUNTIME_ON_REQUEST;
    } else if (params->channel_name == "chrome.runtime.onMessage") {
      histogram_value = is_external ? events::RUNTIME_ON_MESSAGE_EXTERNAL
                                    : events::RUNTIME_ON_MESSAGE;
    } else {
      histogram_value = is_external ? events::RUNTIME_ON_CONNECT_EXTERNAL
                                    : events::RUNTIME_ON_CONNECT;
    }
    EventRouter::Get(browser_context)
        ->ReportEvent(histogram_value, target_extension, did_enqueue);
  }

  // Keep both ends of the channel alive until the channel is closed.
  channel->opener->IncrementLazyKeepaliveCount(
      false /* is_for_native_message_connect */);
  // Note: Though the receiver can be SW for native hosts connecting to it, we
  // don't support long lived SW for this particular case yet and specify false
  // below.
  channel->receiver->IncrementLazyKeepaliveCount(
      false /* is_for_native_message_connect */);
}

void MessageService::AddChannel(std::unique_ptr<MessageChannel> channel,
                                const PortId& receiver_port_id) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  ChannelId channel_id = receiver_port_id.GetChannelId();
  CHECK(channels_.find(channel_id) == channels_.end());
  channels_[channel_id] = std::move(channel);
  pending_lazy_context_channels_.erase(channel_id);
}

void MessageService::OpenPort(const PortId& port_id,
                              int process_id,
                              const PortContext& port_context) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(!port_id.is_opener);

  ChannelId channel_id = port_id.GetChannelId();
  auto it = channels_.find(channel_id);
  if (it == channels_.end())
    return;

  it->second->receiver->OpenPort(process_id, port_context);
}

void MessageService::ClosePort(const PortId& port_id,
                               int process_id,
                               const PortContext& context,
                               bool force_close) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  int routing_id = context.frame ? context.frame->routing_id : MSG_ROUTING_NONE;
  int worker_thread_id =
      context.worker ? context.worker->thread_id : kMainThreadId;
  ClosePortImpl(port_id, process_id, routing_id, worker_thread_id, force_close,
                std::string());
}

void MessageService::CloseChannel(const PortId& port_id,
                                  const std::string& error_message) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  ClosePortImpl(port_id, content::ChildProcessHost::kInvalidUniqueID,
                MSG_ROUTING_NONE, kMainThreadId, true, error_message);
}

void MessageService::ClosePortImpl(const PortId& port_id,
                                   int process_id,
                                   int routing_id,
                                   int worker_thread_id,
                                   bool force_close,
                                   const std::string& error_message) {
  // Note: The channel might be not yet created (if the opener became invalid
  // before the channel initialization completed) or already gone (if the other
  // side closed first).
  ChannelId channel_id = port_id.GetChannelId();
  auto it = channels_.find(channel_id);
  if (it == channels_.end()) {
    auto pending = pending_lazy_context_channels_.find(channel_id);
    if (pending != pending_lazy_context_channels_.end()) {
      const LazyContextId& context_id = pending->second;
      context_id.GetTaskQueue()->AddPendingTask(
          context_id,
          base::BindOnce(&MessageService::PendingLazyContextClosePort,
                         weak_factory_.GetWeakPtr(), port_id, process_id,
                         routing_id, worker_thread_id, force_close,
                         error_message));
    }
    return;
  }

  // The difference between closing a channel and port is that closing a port
  // does not necessarily have to destroy the channel if there are multiple
  // receivers, whereas closing a channel always forces all ports to be closed.
  if (force_close) {
    CloseChannelImpl(it, port_id, error_message, true);
  } else if (port_id.is_opener) {
    it->second->opener->ClosePort(process_id, routing_id, worker_thread_id);
  } else {
    it->second->receiver->ClosePort(process_id, routing_id, worker_thread_id);
  }
}

void MessageService::CloseChannelImpl(MessageChannelMap::iterator channel_iter,
                                      const PortId& closing_port_id,
                                      const std::string& error_message,
                                      bool notify_other_port) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  std::unique_ptr<MessageChannel> channel = std::move(channel_iter->second);
  // Remove from map to make sure that it is impossible for CloseChannelImpl to
  // run twice for the same channel.
  channels_.erase(channel_iter);

  // Notify the other side.
  if (notify_other_port) {
    MessagePort* port = closing_port_id.is_opener ? channel->receiver.get()
                                                  : channel->opener.get();
    port->DispatchOnDisconnect(error_message);
  }

  // Balance the IncrementLazyKeepaliveCount() in OpenChannelImpl.
  channel->opener->DecrementLazyKeepaliveCount();
  channel->receiver->DecrementLazyKeepaliveCount();
}

void MessageService::PostMessage(const PortId& source_port_id,
                                 const Message& message) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  ChannelId channel_id = source_port_id.GetChannelId();
  auto iter = channels_.find(channel_id);
  if (iter == channels_.end()) {
    // If this channel is pending, queue up the PostMessage to run once
    // the channel opens.
    EnqueuePendingMessage(source_port_id, channel_id, message);
    return;
  }

  DispatchMessage(source_port_id, iter->second.get(), message);
}

void MessageService::EnqueuePendingMessage(const PortId& source_port_id,
                                           const ChannelId& channel_id,
                                           const Message& message) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  auto pending_for_incognito = pending_incognito_channels_.find(channel_id);
  if (pending_for_incognito != pending_incognito_channels_.end()) {
    pending_for_incognito->second.push_back(
        PendingMessage(source_port_id, message));
    // A channel should only be holding pending messages because it is in one
    // of these states.
    DCHECK(!base::Contains(pending_lazy_context_channels_, channel_id));
    return;
  }
  EnqueuePendingMessageForLazyBackgroundLoad(source_port_id, channel_id,
                                             message);
}

void MessageService::EnqueuePendingMessageForLazyBackgroundLoad(
    const PortId& source_port_id,
    const ChannelId& channel_id,
    const Message& message) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  auto pending = pending_lazy_context_channels_.find(channel_id);
  if (pending != pending_lazy_context_channels_.end()) {
    const LazyContextId& context_id = pending->second;
    context_id.GetTaskQueue()->AddPendingTask(
        context_id,
        base::BindOnce(&MessageService::PendingLazyContextPostMessage,
                       weak_factory_.GetWeakPtr(), source_port_id, message));
  }
}

void MessageService::DispatchMessage(const PortId& source_port_id,
                                     MessageChannel* channel,
                                     const Message& message) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  // Figure out which port the ID corresponds to.
  MessagePort* dest_port = source_port_id.is_opener ? channel->receiver.get()
                                                    : channel->opener.get();

  dest_port->DispatchOnMessage(message);
}

void MessageService::NotifyResponsePending(const PortId& port_id,
                                           int process_id,
                                           const PortContext& port_context) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(!port_id.is_opener);

  ChannelId channel_id = port_id.GetChannelId();
  auto it = channels_.find(channel_id);
  if (it == channels_.end())
    return;

  it->second->receiver->NotifyResponsePending();
}

bool MessageService::MaybeAddPendingLazyContextOpenChannelTask(
    BrowserContext* context,
    const Extension* extension,
    std::unique_ptr<OpenChannelParams>* params,
    const PendingMessagesQueue& pending_messages) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(ExtensionsBrowserClient::Get()->IsSameContext(context, context_));

  const bool is_event_page = BackgroundInfo::HasLazyBackgroundPage(extension);
  bool is_worker_based = false;
  if (!is_event_page)
    is_worker_based = BackgroundInfo::IsServiceWorkerBased(extension);
  if (!is_worker_based && !is_event_page)
    return false;

  // If the extension uses spanning incognito mode, make sure we're always
  // using the original profile since that is what the extension processes
  // will use.
  if (!IncognitoInfo::IsSplitMode(extension))
    context = ExtensionsBrowserClient::Get()->GetOriginalContext(context);

  const LazyContextId context_id = LazyContextIdFor(context, extension);
  LazyContextTaskQueue* task_queue = context_id.GetTaskQueue();
  if (!task_queue->ShouldEnqueueTask(context, extension))
    return false;

  ChannelId channel_id = (*params)->receiver_port_id.GetChannelId();
  pending_lazy_context_channels_.emplace(channel_id, context_id);
  task_queue->AddPendingTask(
      context_id,
      base::BindOnce(&MessageService::PendingLazyContextOpenChannel,
                     weak_factory_.GetWeakPtr(), std::move(*params)));

  for (const PendingMessage& message : pending_messages) {
    EnqueuePendingMessageForLazyBackgroundLoad(message.first, channel_id,
                                               message.second);
  }
  return true;
}

void MessageService::OnOpenChannelAllowed(
    std::unique_ptr<OpenChannelParams> params,
    bool allowed) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  ChannelId channel_id = params->receiver_port_id.GetChannelId();

  auto pending_for_incognito = pending_incognito_channels_.find(channel_id);
  if (pending_for_incognito == pending_incognito_channels_.end()) {
    NOTREACHED();
    return;
  }
  PendingMessagesQueue pending_messages;
  pending_messages.swap(pending_for_incognito->second);
  pending_incognito_channels_.erase(pending_for_incognito);

  // Check whether the source got closed while in flight.
  const ChannelEndpoint& source = params->source;
  // Re-lookup the source process since it may no longer be valid.
  if (!source.IsValid())
    return;

  if (!params->opener_port->IsValidPort())
    return;

  if (!allowed) {
    params->opener_port->DispatchOnDisconnect(kReceivingEndDoesntExistError);
    return;
  }

  BrowserContext* context = source.browser_context();
  DCHECK(ExtensionsBrowserClient::Get()->IsSameContext(context, context_));

  ExtensionRegistry* registry = ExtensionRegistry::Get(context);
  const Extension* target_extension =
      registry->enabled_extensions().GetByID(params->target_extension_id);
  if (!target_extension) {
    params->opener_port->DispatchOnDisconnect(kReceivingEndDoesntExistError);
    return;
  }

  BrowserContext* context_to_use_for_extension =
      IncognitoInfo::IsSplitMode(target_extension)
          ? context
          : ExtensionsBrowserClient::Get()->GetOriginalContext(context);
  params->receiver = ExtensionMessagePort::CreateForExtension(
      weak_factory_.GetWeakPtr(), params->receiver_port_id,
      params->target_extension_id, context_to_use_for_extension);

  // The target might be a lazy background page or a Service Worker. In that
  // case, we have to check if it is loaded and ready, and if not, queue up the
  // task and load the page.
  if (!MaybeAddPendingLazyContextOpenChannelTask(context, target_extension,
                                                 &params, pending_messages)) {
    OpenChannelImpl(context, std::move(params), target_extension,
                    false /* did_enqueue */);
    DispatchPendingMessages(pending_messages, channel_id);
  }
}

void MessageService::PendingLazyContextOpenChannel(
    std::unique_ptr<OpenChannelParams> params,
    std::unique_ptr<LazyContextTaskQueue::ContextInfo> context_info) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  if (context_info == nullptr)
    return;  // TODO(mpcomplete): notify source of disconnect?

  params->receiver = ExtensionMessagePort::CreateForExtension(
      weak_factory_.GetWeakPtr(), params->receiver_port_id,
      params->target_extension_id, context_info->browser_context);
  const Extension* const extension =
      extensions::ExtensionRegistry::Get(context_info->browser_context)
          ->enabled_extensions()
          .GetByID(context_info->extension_id);
  OpenChannelImpl(context_info->browser_context, std::move(params), extension,
                  true /* did_enqueue */);
}

void MessageService::DispatchPendingMessages(const PendingMessagesQueue& queue,
                                             const ChannelId& channel_id) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  auto channel_iter = channels_.find(channel_id);
  if (channel_iter != channels_.end()) {
    for (const PendingMessage& message : queue) {
      DispatchMessage(message.first, channel_iter->second.get(),
                      message.second);
    }
  }
}

}  // namespace extensions
