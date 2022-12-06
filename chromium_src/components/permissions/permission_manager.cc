// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/permissions/permission_manager.h"

#include <memory>
#include <utility>

#include "base/bind.h"
#include "base/callback.h"
#include "base/feature_list.h"
#include "base/memory/raw_ptr.h"
#include "build/build_config.h"
#include "build/chromeos_buildflags.h"
#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "components/permissions/features.h"
#include "components/permissions/permission_context_base.h"
#include "components/permissions/permission_request_id.h"
#include "components/permissions/permission_result.h"
#include "components/permissions/permission_uma_util.h"
#include "components/permissions/permission_util.h"
#include "components/permissions/permissions_client.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/permission_controller.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "third_party/blink/public/common/permissions/permission_utils.h"

using blink::PermissionType;
using blink::mojom::PermissionStatus;

namespace permissions {
namespace {

// Represents the possible methods of delegating permissions from main frames
// to child frames.
enum class PermissionDelegationMode {
  // Permissions from the main frame are delegated to child frames.
  // This is the default delegation mode for permissions. If a main frame was
  // granted a permission that is delegated, its child frames will inherit that
  // permission if allowed by the permissions policy.
  kDelegated,
  // Permissions from the main frame are not delegated to child frames.
  // An undelegated permission will only be granted to a child frame if the
  // child frame's origin was previously granted access to the permission when
  // in a main frame.
  kUndelegated,
  // Permission access is a function of both the requesting and embedding
  // origins.
  kDoubleKeyed,
};

// Helper methods to convert ContentSetting to PermissionStatus and vice versa.
PermissionStatus ContentSettingToPermissionStatus(ContentSetting setting) {
  switch (setting) {
    case CONTENT_SETTING_ALLOW:
      return PermissionStatus::GRANTED;
    case CONTENT_SETTING_BLOCK:
      return PermissionStatus::DENIED;
    case CONTENT_SETTING_ASK:
      return PermissionStatus::ASK;
    case CONTENT_SETTING_SESSION_ONLY:
    case CONTENT_SETTING_DETECT_IMPORTANT_CONTENT:
    case CONTENT_SETTING_DEFAULT:
    case CONTENT_SETTING_NUM_SETTINGS:
      break;
  }

  NOTREACHED();
  return PermissionStatus::DENIED;
}

PermissionDelegationMode GetPermissionDelegationMode(
    ContentSettingsType permission) {
  // TODO(crbug.com/987654): Generalize this to other "background permissions",
  // that is, permissions that can be used by a service worker. This includes
  // durable storage, background sync, etc.
  if (permission == ContentSettingsType::NOTIFICATIONS)
    return PermissionDelegationMode::kUndelegated;
  if (permission == ContentSettingsType::STORAGE_ACCESS)
    return PermissionDelegationMode::kDoubleKeyed;
  return PermissionDelegationMode::kDelegated;
}

void SubscriptionCallbackWrapper(
    base::OnceCallback<void(PermissionStatus)> callback,
    ContentSetting content_setting) {
  std::move(callback).Run(ContentSettingToPermissionStatus(content_setting));
}

void PermissionStatusVectorCallbackWrapper(
    base::OnceCallback<void(const std::vector<PermissionStatus>&)> callback,
    const std::vector<ContentSetting>& content_settings) {
  std::vector<PermissionStatus> permission_statuses;
  std::transform(content_settings.begin(), content_settings.end(),
                 back_inserter(permission_statuses),
                 ContentSettingToPermissionStatus);
  std::move(callback).Run(permission_statuses);
}

GURL GetEmbeddingOrigin(content::RenderFrameHost* const render_frame_host,
                        const GURL& requesting_origin) {
  content::WebContents* const web_contents =
      content::WebContents::FromRenderFrameHost(render_frame_host);
  DCHECK(web_contents);

  if (PermissionsClient::Get()->DoOriginsMatchNewTabPage(
          requesting_origin,
          web_contents->GetLastCommittedURL().DeprecatedGetOriginAsURL())) {
    return web_contents->GetLastCommittedURL().DeprecatedGetOriginAsURL();
  } else {
    return PermissionUtil::GetLastCommittedOriginAsURL(
        render_frame_host->GetMainFrame());
  }
}

// If an iframed document/worker inherits a different StoragePartition from its
// embedder than it would use if it were a main frame, we should block
// undelegated permissions. Because permissions are scoped to BrowserContext
// instead of StoragePartition, without this check the aforementioned iframe
// would be given undelegated permissions if the user had granted its origin
// access when it was loaded as a main frame.
bool IsPermissionBlockedInPartition(
    ContentSettingsType permission,
    const GURL& requesting_origin,
    content::RenderProcessHost* render_process_host) {
  DCHECK(render_process_host);
  switch (GetPermissionDelegationMode(permission)) {
    case PermissionDelegationMode::kDelegated:
      return false;
    case PermissionDelegationMode::kDoubleKeyed:
      return false;
    case PermissionDelegationMode::kUndelegated:
      // TODO(crbug.com/1312218): This will create |requesting_origin|'s home
      // StoragePartition if it doesn't already exist. Given how
      // StoragePartitions are used today, this shouldn't actually be a
      // problem, but ideally we'd compare StoragePartitionConfigs.
      content::StoragePartition* requesting_home_partition =
          render_process_host->GetBrowserContext()->GetStoragePartitionForUrl(
              requesting_origin);
      return requesting_home_partition !=
             render_process_host->GetStoragePartition();
  }
}

}  // anonymous namespace

class PermissionManager::PendingRequest {
 public:
  PendingRequest(
      content::RenderFrameHost* render_frame_host,
      const std::vector<ContentSettingsType>& permissions,
      base::OnceCallback<void(const std::vector<ContentSetting>&)> callback)
      : render_process_id_(render_frame_host->GetProcess()->GetID()),
        render_frame_id_(render_frame_host->GetRoutingID()),
        callback_(std::move(callback)),
        permissions_(permissions),
        results_(permissions.size(), CONTENT_SETTING_BLOCK),
        remaining_results_(permissions.size()) {}

  void SetContentSetting(int permission_id, ContentSetting content_setting) {
    DCHECK(!IsComplete());

    results_[permission_id] = content_setting;
    --remaining_results_;
  }

  bool IsComplete() const { return remaining_results_ == 0; }

  int render_process_id() const { return render_process_id_; }
  int render_frame_id() const { return render_frame_id_; }

  base::OnceCallback<void(const std::vector<ContentSetting>&)> TakeCallback() {
    return std::move(callback_);
  }

  std::vector<ContentSettingsType> permissions() const { return permissions_; }

  std::vector<ContentSetting> results() const { return results_; }

 private:
  int render_process_id_;
  int render_frame_id_;
  base::OnceCallback<void(const std::vector<ContentSetting>&)> callback_;
  std::vector<ContentSettingsType> permissions_;
  std::vector<ContentSetting> results_;
  size_t remaining_results_;
};

// Object to track the callback passed to
// PermissionContextBase::RequestPermission. The callback passed in will never
// be run when a permission prompt has been ignored, but it's important that we
// know when a prompt is ignored to clean up |pending_requests_| correctly.
// If the callback is destroyed without being run, the destructor here will
// cancel the request to clean up. |permission_manager| must outlive this
// object.
class PermissionManager::PermissionResponseCallback {
 public:
  PermissionResponseCallback(PermissionManager* permission_manager,
                             PendingRequestLocalId request_local_id,
                             int permission_id)
      : permission_manager_(permission_manager),
        request_local_id_(request_local_id),
        permission_id_(permission_id),
        request_answered_(false) {}

  PermissionResponseCallback(const PermissionResponseCallback&) = delete;
  PermissionResponseCallback& operator=(const PermissionResponseCallback&) =
      delete;

  ~PermissionResponseCallback() {
    if (!request_answered_ &&
        permission_manager_->pending_requests_.Lookup(request_local_id_)) {
      permission_manager_->pending_requests_.Remove(request_local_id_);
    }
  }

  void OnPermissionsRequestResponseStatus(ContentSetting content_setting) {
    request_answered_ = true;
    permission_manager_->OnPermissionsRequestResponseStatus(
        request_local_id_, permission_id_, content_setting);
  }

 private:
  raw_ptr<PermissionManager> permission_manager_;
  PendingRequestLocalId request_local_id_;
  int permission_id_;
  bool request_answered_;
};

struct PermissionManager::Subscription {
  ContentSettingsType permission;
  GURL requesting_origin;
  int render_frame_id = -1;
  int render_process_id = -1;
  base::RepeatingCallback<void(ContentSetting)> callback;
  ContentSetting current_value;
};

PermissionManager::PermissionManager(content::BrowserContext* browser_context,
                                     PermissionContextMap permission_contexts)
    : browser_context_(browser_context),
      permission_contexts_(std::move(permission_contexts)) {}

PermissionManager::~PermissionManager() {
  DCHECK(pending_requests_.IsEmpty());
  DCHECK(subscriptions_.IsEmpty());
}

GURL PermissionManager::GetCanonicalOrigin(ContentSettingsType permission,
                                           const GURL& requesting_origin,
                                           const GURL& embedding_origin) const {
  absl::optional<GURL> override_origin =
      PermissionsClient::Get()->OverrideCanonicalOrigin(requesting_origin,
                                                        embedding_origin);
  if (override_origin)
    return override_origin.value();

  switch (GetPermissionDelegationMode(permission)) {
    case PermissionDelegationMode::kDelegated:
      return embedding_origin;
    case PermissionDelegationMode::kDoubleKeyed:
    case PermissionDelegationMode::kUndelegated:
      return requesting_origin;
  }
}

PermissionResult PermissionManager::GetPermissionStatusDeprecated(
    ContentSettingsType permission,
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
  DCHECK_EQ(requesting_origin, embedding_origin);

  return GetPermissionStatusHelper(permission,
                                   /*render_process_host=*/nullptr,
                                   /*render_frame_host=*/nullptr,
                                   requesting_origin, embedding_origin);
}

PermissionResult PermissionManager::GetPermissionStatusForDisplayOnSettingsUI(
    ContentSettingsType permission,
    const GURL& origin) {
  return GetPermissionStatusHelper(permission,
                                   /*render_process_host=*/nullptr,
                                   /*render_frame_host=*/nullptr, origin,
                                   origin);
}

PermissionResult PermissionManager::GetPermissionStatusForCurrentDocument(
    ContentSettingsType permission,
    content::RenderFrameHost* render_frame_host) {
  const GURL requesting_origin =
      PermissionUtil::GetLastCommittedOriginAsURL(render_frame_host);
  const GURL embedding_origin =
      GetEmbeddingOrigin(render_frame_host, requesting_origin);

  return GetPermissionStatusHelper(permission,
                                   /*render_process_host=*/nullptr,
                                   render_frame_host, requesting_origin,
                                   embedding_origin);
}

void PermissionManager::Shutdown() {
  is_shutting_down_ = true;

  if (!subscriptions_.IsEmpty()) {
    subscriptions_.Clear();
    for (const auto& type_to_count : subscription_type_counts_) {
      if (type_to_count.second > 0) {
        PermissionContextBase* context =
            GetPermissionContext(type_to_count.first);
        context->RemoveObserver(this);
      }
    }
    subscription_type_counts_.clear();
  }
}

PermissionContextBase* PermissionManager::GetPermissionContextForTesting(
    ContentSettingsType type) {
  return GetPermissionContext(type);
}

PermissionContextBase* PermissionManager::GetPermissionContext(
    ContentSettingsType type) {
  const auto& it = permission_contexts_.find(type);
  return it == permission_contexts_.end() ? nullptr : it->second.get();
}

// TODO(crbug.com/1271543): Remove
// PermissionControllerDelegate::RequestPermission.
void PermissionManager::RequestPermission(
    PermissionType permission,
    content::RenderFrameHost* render_frame_host,
    const GURL& requesting_origin,
    bool user_gesture,
    base::OnceCallback<void(PermissionStatus)> callback) {
  NOTIMPLEMENTED();
}

// TODO(crbug.com/1271543): Remove
// PermissionControllerDelegate::RequestPermissions.
void PermissionManager::RequestPermissions(
    const std::vector<PermissionType>& permissions,
    content::RenderFrameHost* render_frame_host,
    const GURL& requesting_origin,
    bool user_gesture,
    base::OnceCallback<void(const std::vector<PermissionStatus>&)> callback) {
  NOTIMPLEMENTED();
}

void PermissionManager::ResetPermission(PermissionType permission,
                                        const GURL& requesting_origin,
                                        const GURL& embedding_origin) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  ContentSettingsType type =
      PermissionUtil::PermissionTypeToContentSetting(permission);
  PermissionContextBase* context = GetPermissionContext(type);
  if (!context)
    return;
  context->ResetPermission(
      GetCanonicalOrigin(type, requesting_origin, embedding_origin),
      embedding_origin.DeprecatedGetOriginAsURL());
}

void PermissionManager::RequestPermissionsFromCurrentDocument(
    const std::vector<PermissionType>& permissions_types,
    content::RenderFrameHost* render_frame_host,
    bool user_gesture,
    base::OnceCallback<void(const std::vector<blink::mojom::PermissionStatus>&)>
        permission_status_callback) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  std::vector<ContentSettingsType> permissions;
  std::transform(permissions_types.begin(), permissions_types.end(),
                 back_inserter(permissions),
                 PermissionUtil::PermissionTypeToContentSetting);

  base::OnceCallback<void(const std::vector<ContentSetting>&)> callback =
      base::BindOnce(&PermissionStatusVectorCallbackWrapper,
                     std::move(permission_status_callback));

  if (permissions.empty()) {
    std::move(callback).Run(std::vector<ContentSetting>());
    return;
  }

  auto request_local_id = request_local_id_generator_.GenerateNextId();
  pending_requests_.AddWithID(
      std::make_unique<PendingRequest>(render_frame_host, permissions,
                                       std::move(callback)),
      request_local_id);

  const PermissionRequestID request_id(render_frame_host, request_local_id);
  const GURL requesting_origin =
      PermissionUtil::GetLastCommittedOriginAsURL(render_frame_host);
  const GURL embedding_origin =
      GetEmbeddingOrigin(render_frame_host, requesting_origin);

  for (size_t i = 0; i < permissions.size(); ++i) {
    const ContentSettingsType permission = permissions[i];
    const GURL canonical_requesting_origin =
        GetCanonicalOrigin(permission, requesting_origin, embedding_origin);

    auto response_callback =
        std::make_unique<PermissionResponseCallback>(this, request_local_id, i);
    if (IsPermissionBlockedInPartition(permission, requesting_origin,
                                       render_frame_host->GetProcess())) {
      response_callback->OnPermissionsRequestResponseStatus(
          CONTENT_SETTING_BLOCK);
      continue;
    }

    auto status = GetPermissionOverrideForDevTools(
        url::Origin::Create(canonical_requesting_origin), permission);
    if (status != CONTENT_SETTING_DEFAULT) {
      response_callback->OnPermissionsRequestResponseStatus(
          CONTENT_SETTING_ALLOW);
      continue;
    }

    PermissionContextBase* context = GetPermissionContext(permission);
    DCHECK(context);

    context->RequestPermission(
        request_id, canonical_requesting_origin, user_gesture,
        base::BindOnce(
            &PermissionResponseCallback::OnPermissionsRequestResponseStatus,
            std::move(response_callback)));
  }
}

PermissionStatus PermissionManager::GetPermissionStatus(
    PermissionType permission,
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
  // TODO(benwells): split this into two functions, GetPermissionStatus and
  // GetPermissionStatusForPermissionsAPI.
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  PermissionResult result = GetPermissionStatusHelper(
      PermissionUtil::PermissionTypeToContentSetting(permission),
      /*render_process_host=*/nullptr,
      /*render_frame_host=*/nullptr, requesting_origin, embedding_origin);
  return ContentSettingToPermissionStatus(result.content_setting);
}

PermissionStatus PermissionManager::GetPermissionStatusForCurrentDocument(
    PermissionType permission,
    content::RenderFrameHost* render_frame_host) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  ContentSettingsType type =
      PermissionUtil::PermissionTypeToContentSetting(permission);

  const GURL requesting_origin =
      PermissionUtil::GetLastCommittedOriginAsURL(render_frame_host);
  const GURL embedding_origin =
      GetEmbeddingOrigin(render_frame_host, requesting_origin);

  PermissionResult result = GetPermissionStatusHelper(
      type,
      /*render_process_host=*/nullptr, render_frame_host, requesting_origin,
      embedding_origin);

  return ContentSettingToPermissionStatus(result.content_setting);
}

PermissionStatus PermissionManager::GetPermissionStatusForWorker(
    PermissionType permission,
    content::RenderProcessHost* render_process_host,
    const GURL& worker_origin) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  ContentSettingsType type =
      PermissionUtil::PermissionTypeToContentSetting(permission);
  PermissionResult result = GetPermissionStatusHelper(
      type, render_process_host,
      /*render_frame_host=*/nullptr, worker_origin, worker_origin);

  return ContentSettingToPermissionStatus(result.content_setting);
}

bool PermissionManager::IsPermissionOverridableByDevTools(
    PermissionType permission,
    const absl::optional<url::Origin>& origin) {
  ContentSettingsType type =
      PermissionUtil::PermissionTypeToContentSettingSafe(permission);
  PermissionContextBase* context = GetPermissionContext(type);

  if (!context || context->IsPermissionKillSwitchOn())
    return false;

  return !origin || context->IsPermissionAvailableToOrigins(origin->GetURL(),
                                                            origin->GetURL());
}

PermissionManager::SubscriptionId
PermissionManager::SubscribePermissionStatusChange(
    PermissionType permission,
    content::RenderProcessHost* render_process_host,
    content::RenderFrameHost* render_frame_host,
    const GURL& requesting_origin,
    base::RepeatingCallback<void(PermissionStatus)> callback) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  DCHECK(!render_frame_host || !render_process_host);
  if (is_shutting_down_)
    return SubscriptionId();

  ContentSettingsType content_type =
      PermissionUtil::PermissionTypeToContentSetting(permission);
  auto& type_count = subscription_type_counts_[content_type];
  if (type_count == 0) {
    PermissionContextBase* context = GetPermissionContext(content_type);
    context->AddObserver(this);
  }
  ++type_count;

  auto subscription = std::make_unique<Subscription>();

  // The RFH may be null if the request is for a worker.
  GURL embedding_origin;
  if (render_frame_host) {
    // Permissions API must be deferred during the prerendering.
    DCHECK_NE(render_frame_host->GetLifecycleState(),
              content::RenderFrameHost::LifecycleState::kPrerendering);
    embedding_origin = GetEmbeddingOrigin(render_frame_host, requesting_origin);
    subscription->render_frame_id = render_frame_host->GetRoutingID();
    subscription->render_process_id = render_frame_host->GetProcess()->GetID();
    subscription->current_value =
        GetPermissionStatusHelper(content_type,
                                  /*render_process_host=*/nullptr,
                                  render_frame_host, requesting_origin,
                                  embedding_origin)
            .content_setting;

  } else {
    embedding_origin = requesting_origin;
    subscription->render_frame_id = -1;
    subscription->render_process_id =
        render_process_host ? render_process_host->GetID() : -1;
    subscription->current_value =
        GetPermissionStatusHelper(content_type, render_process_host,
                                  /*render_frame_host=*/nullptr,
                                  requesting_origin, embedding_origin)
            .content_setting;
  }

  subscription->permission = content_type;
  subscription->requesting_origin =
      GetCanonicalOrigin(content_type, requesting_origin, embedding_origin);
  subscription->callback =
      base::BindRepeating(&SubscriptionCallbackWrapper, std::move(callback));

  auto id = subscription_id_generator_.GenerateNextId();
  subscriptions_.AddWithID(std::move(subscription), id);
  return id;
}

void PermissionManager::UnsubscribePermissionStatusChange(
    SubscriptionId subscription_id) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  if (is_shutting_down_)
    return;

  Subscription* subscription = subscriptions_.Lookup(subscription_id);
  if (!subscription)
    return;

  ContentSettingsType type = subscription->permission;
  subscriptions_.Remove(subscription_id);
  auto type_count = subscription_type_counts_.find(type);
  CHECK(type_count != subscription_type_counts_.end());
  CHECK_GT(type_count->second, size_t(0));
  type_count->second--;
  if (type_count->second == 0) {
    PermissionContextBase* context = GetPermissionContext(type);
    context->RemoveObserver(this);
  }
}

void PermissionManager::OnPermissionsRequestResponseStatus(
    PendingRequestLocalId request_local_id,
    int permission_id,
    ContentSetting content_setting) {
  PendingRequest* pending_request = pending_requests_.Lookup(request_local_id);
  if (!pending_request)
    return;

  pending_request->SetContentSetting(permission_id, content_setting);

  if (!pending_request->IsComplete())
    return;

  pending_request->TakeCallback().Run(pending_request->results());
  pending_requests_.Remove(request_local_id);
}

void PermissionManager::OnPermissionChanged(
    const ContentSettingsPattern& primary_pattern,
    const ContentSettingsPattern& secondary_pattern,
    ContentSettingsTypeSet content_type_set) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  DCHECK(primary_pattern.IsValid());
  DCHECK(secondary_pattern.IsValid());

  std::vector<base::OnceClosure> callbacks;
  callbacks.reserve(subscriptions_.size());
  for (SubscriptionsMap::iterator iter(&subscriptions_); !iter.IsAtEnd();
       iter.Advance()) {
    Subscription* subscription = iter.GetCurrentValue();
    if (!content_type_set.Contains(subscription->permission))
      continue;

    // The RFH may be null if the request is for a worker.
    content::RenderFrameHost* rfh = content::RenderFrameHost::FromID(
        subscription->render_process_id, subscription->render_frame_id);
    GURL embedding_origin;
    GURL requesting_origin = subscription->requesting_origin;
    if (rfh) {
      embedding_origin =
          GetEmbeddingOrigin(rfh, subscription->requesting_origin);
    } else {
      embedding_origin = subscription->requesting_origin;
    }

    if (!primary_pattern.Matches(requesting_origin) ||
        !secondary_pattern.Matches(embedding_origin)) {
      continue;
    }

    content::RenderProcessHost* rph =
        rfh ? nullptr
            : content::RenderProcessHost::FromID(
                  subscription->render_process_id);

    ContentSetting new_value =
        GetPermissionStatusHelper(subscription->permission, rph, rfh,
                                  subscription->requesting_origin,
                                  embedding_origin)
            .content_setting;

    if (subscription->current_value == new_value)
      continue;

    subscription->current_value = new_value;

    // Add the callback to |callbacks| which will be run after the loop to
    // prevent re-entrance issues.
    callbacks.push_back(base::BindOnce(subscription->callback, new_value));
  }

  for (auto& callback : callbacks)
    std::move(callback).Run();
}

PermissionResult PermissionManager::GetPermissionStatusHelper(
    ContentSettingsType permission,
    content::RenderProcessHost* render_process_host,
    content::RenderFrameHost* render_frame_host,
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
  DCHECK(!render_process_host || !render_frame_host);

  // TODO(crbug.com/1307044): Move this to PermissionContextBase.
  content::RenderProcessHost* rph =
      render_frame_host ? render_frame_host->GetProcess() : render_process_host;
  if (rph &&
      IsPermissionBlockedInPartition(permission, requesting_origin, rph)) {
    return PermissionResult(CONTENT_SETTING_BLOCK,
                            PermissionStatusSource::UNSPECIFIED);
  }

  GURL canonical_requesting_origin =
      GetCanonicalOrigin(permission, requesting_origin, embedding_origin);
  auto status = GetPermissionOverrideForDevTools(
      url::Origin::Create(canonical_requesting_origin), permission);
  if (status != CONTENT_SETTING_DEFAULT)
    return PermissionResult(status, PermissionStatusSource::UNSPECIFIED);
  PermissionContextBase* context = GetPermissionContext(permission);
  PermissionResult result = context->GetPermissionStatus(
      render_frame_host, canonical_requesting_origin.DeprecatedGetOriginAsURL(),
      embedding_origin.DeprecatedGetOriginAsURL());
  result = context->UpdatePermissionStatusWithDeviceStatus(
      result, requesting_origin, embedding_origin);
  DCHECK(result.content_setting == CONTENT_SETTING_ALLOW ||
         result.content_setting == CONTENT_SETTING_ASK ||
         result.content_setting == CONTENT_SETTING_BLOCK);
  return result;
}

ContentSetting PermissionManager::GetPermissionOverrideForDevTools(
    const url::Origin& origin,
    ContentSettingsType permission) {
  auto it = devtools_permission_overrides_.find(origin);
  if (it == devtools_permission_overrides_.end())
    it = devtools_permission_overrides_.find(devtools_global_overrides_origin_);
  if (it == devtools_permission_overrides_.end())
    return CONTENT_SETTING_DEFAULT;

  auto setting_it = it->second.find(permission);
  if (setting_it == it->second.end())
    return CONTENT_SETTING_DEFAULT;

  return setting_it->second;
}

void PermissionManager::SetPermissionOverridesForDevTools(
    const absl::optional<url::Origin>& optional_origin,
    const PermissionOverrides& overrides) {
  ContentSettingsTypeOverrides result;
  for (const auto& item : overrides) {
    ContentSettingsType content_setting =
        PermissionUtil::PermissionTypeToContentSettingSafe(item.first);
    if (content_setting != ContentSettingsType::DEFAULT)
      result[content_setting] =
          PermissionUtil::PermissionStatusToContentSetting(item.second);
  }
  const url::Origin& origin =
      optional_origin.value_or(devtools_global_overrides_origin_);
  devtools_permission_overrides_[origin] = std::move(result);
}

void PermissionManager::ResetPermissionOverridesForDevTools() {
  devtools_permission_overrides_.clear();
}

}  // namespace permissions
