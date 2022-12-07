// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/media_router/media_router_ui.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "base/atomic_sequence_num.h"
#include "base/bind.h"
#include "base/containers/contains.h"
#include "base/containers/cxx20_erase.h"
#include "base/metrics/histogram_functions.h"
#include "base/observer_list.h"
#include "base/strings/strcat.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "build/build_config.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/media/router/providers/wired_display/wired_display_media_route_provider.h"
#include "chrome/browser/media/webrtc/desktop_media_picker_controller.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_tabstrip.h"
#include "chrome/browser/ui/media_router/media_sink_with_cast_modes.h"
#include "chrome/browser/ui/media_router/ui_media_sink.h"
#include "chrome/common/url_constants.h"
#include "chrome/common/webui_url_constants.h"
#include "chrome/grit/generated_resources.h"
#include "components/media_router/browser/issue_manager.h"
#include "components/media_router/browser/issues_observer.h"
#include "components/media_router/browser/media_router.h"
#include "components/media_router/browser/media_router_factory.h"
#include "components/media_router/browser/media_routes_observer.h"
#include "components/media_router/browser/presentation/presentation_service_delegate_impl.h"
#include "components/media_router/common/media_route.h"
#include "components/media_router/common/media_sink.h"
#include "components/media_router/common/media_source.h"
#include "components/media_router/common/route_request_result.h"
#include "components/sessions/content/session_tab_helper.h"
#include "components/url_formatter/elide_url.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/navigation_handle.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/common/constants.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"
#include "third_party/blink/public/mojom/media/fullscreen_video_element.mojom.h"
#include "third_party/icu/source/i18n/unicode/coll.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/display/display.h"
#include "url/origin.h"

#if BUILDFLAG(IS_MAC)
#include "base/mac/mac_util.h"
#include "ui/base/cocoa/permissions_utils.h"
#endif

namespace media_router {

namespace {

constexpr char kLoggerComponent[] = "MediaRouterUI";

// Returns true if |issue| is associated with |ui_sink|.
bool IssueMatches(const Issue& issue, const UIMediaSink& ui_sink) {
  return issue.info().sink_id == ui_sink.id ||
         (!issue.info().route_id.empty() && ui_sink.route &&
          issue.info().route_id == ui_sink.route->media_route_id());
}

std::u16string GetSinkFriendlyName(const MediaSink& sink) {
  // Use U+2010 (HYPHEN) instead of ASCII hyphen to avoid problems with RTL
  // languages.
  const char* separator = " \u2010 ";
  return base::UTF8ToUTF16(sink.description() ? sink.name() + separator +
                                                    sink.description().value()
                                              : sink.name());
}

void MaybeReportCastingSource(MediaCastMode cast_mode,
                              const RouteRequestResult& result) {
  if (result.result_code() == mojom::RouteRequestResultCode::OK)
    base::UmaHistogramSparse("MediaRouter.Source.CastingSource", cast_mode);
}

}  // namespace

MediaRouterUI::MediaRouterUI(
    std::unique_ptr<MediaRouteStarter> media_route_starter)
    : media_route_starter_(std::move(media_route_starter)),
      router_(media_route_starter_->GetMediaRouter()),
      logger_(router_->GetLogger()) {
  DCHECK(media_route_starter_) << "Must have a media_route_starter!";
  media_route_starter_->SetLoggerComponent(kLoggerComponent);
  Init();
}

MediaRouterUI::~MediaRouterUI() {
  if (media_route_starter_)
    DetachFromMediaRouteStarter();
}

// static
std::unique_ptr<MediaRouterUI> MediaRouterUI::CreateMediaRouterUI(
    const CastModeSet& initial_modes,
    content::WebContents* initiator,
    std::unique_ptr<StartPresentationContext> start_presentation_context) {
  DCHECK(initiator) << "Must have an initiator!";
  auto media_route_starter = std::make_unique<MediaRouteStarter>(
      initial_modes, initiator, std::move(start_presentation_context));
  return std::make_unique<MediaRouterUI>(std::move(media_route_starter));
}

std::unique_ptr<MediaRouterUI> MediaRouterUI::CreateWithDefaultMediaSource(
    content::WebContents* initiator) {
  return CreateMediaRouterUI({MediaCastMode::PRESENTATION}, initiator,
                             /* start_presentation_context */ nullptr);
}

// static
std::unique_ptr<MediaRouterUI>
MediaRouterUI::CreateWithDefaultMediaSourceAndMirroring(
    content::WebContents* initiator) {
  return CreateMediaRouterUI(
      {MediaCastMode::PRESENTATION, MediaCastMode::TAB_MIRROR,
       MediaCastMode::DESKTOP_MIRROR},
      initiator, /* start_presentation_context */ nullptr);
}

// static
std::unique_ptr<MediaRouterUI>
MediaRouterUI::CreateWithStartPresentationContext(
    content::WebContents* initiator,
    std::unique_ptr<StartPresentationContext> context) {
  DCHECK(context) << "context must not be null!";
  return CreateMediaRouterUI({MediaCastMode::PRESENTATION}, initiator,
                             std::move(context));
}

// static
std::unique_ptr<MediaRouterUI>
MediaRouterUI::CreateWithStartPresentationContextAndMirroring(
    content::WebContents* initiator,
    std::unique_ptr<StartPresentationContext> context) {
  DCHECK(context) << "context must not be null!";
  return CreateMediaRouterUI(
      {MediaCastMode::PRESENTATION, MediaCastMode::TAB_MIRROR,
       MediaCastMode::DESKTOP_MIRROR},
      initiator, std::move(context));
}

void MediaRouterUI::DetachFromMediaRouteStarter() {
  for (CastDialogController::Observer& observer : observers_)
    observer.OnControllerInvalidated();

  media_route_starter()->RemovePresentationRequestSourceObserver(this);
  media_route_starter()->RemoveMediaSinkWithCastModesObserver(this);
}

void MediaRouterUI::AddObserver(CastDialogController::Observer* observer) {
  observers_.AddObserver(observer);
  // TODO(takumif): Update the header when this object is initialized instead.
  UpdateModelHeader(media_route_starter()->GetPresentationRequestSourceName());
}

void MediaRouterUI::RemoveObserver(CastDialogController::Observer* observer) {
  observers_.RemoveObserver(observer);
}

void MediaRouterUI::StartCasting(const std::string& sink_id,
                                 MediaCastMode cast_mode) {
  CreateRoute(sink_id, cast_mode);
}

void MediaRouterUI::StopCasting(const std::string& route_id) {
  terminating_route_id_ = route_id;
  // |route_id| may become invalid after UpdateSinks(), so we cannot refer to
  // |route_id| below this line.
  UpdateSinks();
  TerminateRoute(terminating_route_id_.value());
}

void MediaRouterUI::ClearIssue(const Issue::Id& issue_id) {
  RemoveIssue(issue_id);
}

content::WebContents* MediaRouterUI::GetInitiator() {
  return initiator();
}

std::unique_ptr<MediaRouteStarter> MediaRouterUI::TakeMediaRouteStarter() {
  DCHECK(media_route_starter_) << "MediaRouteStarter already taken!";
  DetachFromMediaRouteStarter();
  return std::move(media_route_starter_);
}

bool MediaRouterUI::CreateRoute(const MediaSink::Id& sink_id,
                                MediaCastMode cast_mode) {
  logger_->LogInfo(mojom::LogCategory::kUi, kLoggerComponent,
                   "CreateRoute requested by MediaRouterViewsUI.", sink_id, "",
                   "");

  auto params =
      media_route_starter()->CreateRouteParameters(sink_id, cast_mode);
  if (!params) {
    SendIssueForUnableToCast(cast_mode, sink_id);
    return false;
  }

  if (!MediaRouteStarter::GetScreenCapturePermission(cast_mode)) {
    SendIssueForScreenPermission(sink_id);
    return false;
  }

  GetIssueManager()->ClearNonBlockingIssues();

  current_route_request_ = absl::make_optional(*params->request);

  params->route_result_callbacks.push_back(
      base::BindOnce(&MaybeReportCastingSource, cast_mode));

  params->route_result_callbacks.push_back(base::BindOnce(
      &MediaRouterUI::OnRouteResponseReceived, weak_factory_.GetWeakPtr(),
      current_route_request_->id, sink_id, cast_mode,
      media_route_starter()->GetPresentationRequestSourceName()));

  media_route_starter()->StartRoute(std::move(params));

  // TODO(crbug.com/1015203): This call to UpdateSinks() was originally in
  // StartCasting(), but it causes Chrome to crash when the desktop picker
  // dialog is shown, so for now we just don't call it in that case.  Move it
  // back once the problem is resolved.
  if (cast_mode != MediaCastMode::DESKTOP_MIRROR)
    UpdateSinks();

  return true;
}

void MediaRouterUI::TerminateRoute(const MediaRoute::Id& route_id) {
  logger_->LogInfo(mojom::LogCategory::kUi, kLoggerComponent,
                   "TerminateRoute requested by MediaRouterUI.",
                   MediaRoute::GetSinkIdFromMediaRouteId(route_id),
                   MediaRoute::GetMediaSourceIdFromMediaRouteId(route_id),
                   MediaRoute::GetPresentationIdFromMediaRouteId(route_id));
  GetMediaRouter()->TerminateRoute(route_id);
}

std::vector<MediaSinkWithCastModes> MediaRouterUI::GetEnabledSinks() const {
  if (!display_observer_)
    return sinks_;

  // Filter out the wired display sink for the display that the dialog is on.
  // This is not the best place to do this because MRUI should not perform a
  // provider-specific behavior, but we currently do not have a way to
  // communicate dialog-specific information to/from the
  // WiredDisplayMediaRouteProvider.
  std::vector<MediaSinkWithCastModes> enabled_sinks(sinks_);
#if !BUILDFLAG(IS_ANDROID)
  const std::string display_sink_id =
      WiredDisplayMediaRouteProvider::GetSinkIdForDisplay(
          display_observer_->GetCurrentDisplay());
  base::EraseIf(enabled_sinks,
                [&display_sink_id](const MediaSinkWithCastModes& sink) {
                  return sink.sink.id() == display_sink_id;
                });
#endif

  return enabled_sinks;
}

void MediaRouterUI::AddIssue(const IssueInfo& issue) {
  GetIssueManager()->AddIssue(issue);
  switch (issue.severity) {
    case IssueInfo::Severity::NOTIFICATION:
      logger_->LogInfo(
          mojom::LogCategory::kUi, kLoggerComponent,
          base::StrCat({"Sink button shows an issue in NOTIFICATION level: ",
                        issue.title}),
          issue.sink_id,
          MediaRoute::GetMediaSourceIdFromMediaRouteId(issue.route_id),
          MediaRoute::GetPresentationIdFromMediaRouteId(issue.route_id));
      break;
    default:
      logger_->LogError(
          mojom::LogCategory::kUi, kLoggerComponent,
          base::StrCat(
              {"Sink button shows an issue in WARNING or FATAL level: ",
               issue.title}),
          issue.sink_id,
          MediaRoute::GetMediaSourceIdFromMediaRouteId(issue.route_id),
          MediaRoute::GetPresentationIdFromMediaRouteId(issue.route_id));
      break;
  }
}

void MediaRouterUI::RemoveIssue(const Issue::Id& issue_id) {
  GetIssueManager()->ClearIssue(issue_id);
}

void MediaRouterUI::LogMediaSinkStatus() {
  std::vector<std::string> sink_ids;
  for (const auto& sink : GetEnabledSinks()) {
    if (sink.sink.id().length() <= 4) {
      sink_ids.push_back(sink.sink.id());
    } else {
      sink_ids.push_back(sink.sink.id().substr(sink.sink.id().length() - 4));
    }
  }
  logger_->LogInfo(
      mojom::LogCategory::kUi, kLoggerComponent,
      base::StrCat(
          {base::StringPrintf("%zu sinks shown on CastDialogView closed: ",
                              sink_ids.size()),
           base::JoinString(sink_ids, ",")}),
      "", "", "");
}

MediaRouterUI::UiIssuesObserver::UiIssuesObserver(IssueManager* issue_manager,
                                                  MediaRouterUI* ui)
    : IssuesObserver(issue_manager), ui_(ui) {
  DCHECK(ui);
}

MediaRouterUI::UiIssuesObserver::~UiIssuesObserver() = default;

void MediaRouterUI::UiIssuesObserver::OnIssue(const Issue& issue) {
  ui_->OnIssue(issue);
}

void MediaRouterUI::UiIssuesObserver::OnIssuesCleared() {
  ui_->OnIssueCleared();
}

MediaRouterUI::UIMediaRoutesObserver::UIMediaRoutesObserver(
    MediaRouter* router,
    const RoutesUpdatedCallback& callback)
    : MediaRoutesObserver(router), callback_(callback) {
  DCHECK(!callback_.is_null());
}

MediaRouterUI::UIMediaRoutesObserver::~UIMediaRoutesObserver() = default;

void MediaRouterUI::UIMediaRoutesObserver::OnRoutesUpdated(
    const std::vector<MediaRoute>& routes) {
  callback_.Run(routes);
}

void MediaRouterUI::Init() {
  DCHECK(!collator_) << "Init should only be called once!";

  media_route_starter()->AddMediaSinkWithCastModesObserver(this);
  media_route_starter()->AddPresentationRequestSourceObserver(this);

  GetMediaRouter()->OnUserGesture();

  UErrorCode error = U_ZERO_ERROR;
  const std::string& locale = g_browser_process->GetApplicationLocale();
  collator_.reset(
      icu::Collator::createInstance(icu::Locale(locale.c_str()), error));
  if (U_FAILURE(error)) {
    DLOG(ERROR) << "Failed to create collator for locale " << locale;
    collator_.reset();
  }

  // Get the current list of media routes, so that the WebUI will have routes
  // information at initialization.
  OnRoutesUpdated(GetMediaRouter()->GetCurrentRoutes());
  display_observer_ = WebContentsDisplayObserver::Create(
      initiator(),
      base::BindRepeating(&MediaRouterUI::UpdateSinks, base::Unretained(this)));

  routes_observer_ = std::make_unique<UIMediaRoutesObserver>(
      GetMediaRouter(), base::BindRepeating(&MediaRouterUI::OnRoutesUpdated,
                                            base::Unretained(this)));

  StartObservingIssues();
}

void MediaRouterUI::OnSourceUpdated(std::u16string& source_name) {
  UpdateModelHeader(source_name);
}

void MediaRouterUI::UpdateSinks() {
  std::vector<UIMediaSink> media_sinks;
  for (const MediaSinkWithCastModes& sink : GetEnabledSinks()) {
    auto pred = [&sink](const MediaRoute& route) {
      return route.media_sink_id() == sink.sink.id();
    };
    auto route_it = std::find_if(routes().begin(), routes().end(), pred);
    const MediaRoute* route = route_it == routes().end() ? nullptr : &*route_it;
    media_sinks.push_back(ConvertToUISink(sink, route, issue_));
  }
  model_.set_media_sinks(std::move(media_sinks));
  for (CastDialogController::Observer& observer : observers_)
    observer.OnModelUpdated(model_);
}

void MediaRouterUI::SendIssueForRouteTimeout(
    MediaCastMode cast_mode,
    const MediaSink::Id& sink_id,
    const std::u16string& presentation_request_source_name) {
  std::string issue_title;
  switch (cast_mode) {
    case PRESENTATION:
      DLOG_IF(ERROR, presentation_request_source_name.empty())
          << "Empty presentation request source name.";
      issue_title =
          l10n_util::GetStringFUTF8(IDS_MEDIA_ROUTER_ISSUE_CREATE_ROUTE_TIMEOUT,
                                    presentation_request_source_name);
      break;
    case TAB_MIRROR:
      issue_title = l10n_util::GetStringUTF8(
          IDS_MEDIA_ROUTER_ISSUE_CREATE_ROUTE_TIMEOUT_FOR_TAB);
      break;
    case DESKTOP_MIRROR:
      issue_title = l10n_util::GetStringUTF8(
          IDS_MEDIA_ROUTER_ISSUE_CREATE_ROUTE_TIMEOUT_FOR_DESKTOP);
      break;
  }

  IssueInfo issue_info(issue_title, IssueInfo::Action::DISMISS,
                       IssueInfo::Severity::NOTIFICATION);
  issue_info.sink_id = sink_id;
  AddIssue(issue_info);
}

void MediaRouterUI::SendIssueForScreenPermission(const MediaSink::Id& sink_id) {
#if BUILDFLAG(IS_MAC)
  std::string issue_title = l10n_util::GetStringUTF8(
      IDS_MEDIA_ROUTER_ISSUE_MAC_SCREEN_CAPTURE_PERMISSION_ERROR);
  IssueInfo issue_info(issue_title, IssueInfo::Action::DISMISS,
                       IssueInfo::Severity::WARNING);
  issue_info.sink_id = sink_id;
  AddIssue(issue_info);
#else
  NOTREACHED() << "Only valid for MAC OS!";
#endif
}

void MediaRouterUI::SendIssueForUnableToCast(MediaCastMode cast_mode,
                                             const MediaSink::Id& sink_id) {
  // For a generic error, claim a tab error unless it was specifically desktop
  // mirroring.
  std::string issue_title =
      (cast_mode == MediaCastMode::DESKTOP_MIRROR)
          ? l10n_util::GetStringUTF8(
                IDS_MEDIA_ROUTER_ISSUE_UNABLE_TO_CAST_DESKTOP)
          : l10n_util::GetStringUTF8(
                IDS_MEDIA_ROUTER_ISSUE_CREATE_ROUTE_TIMEOUT_FOR_TAB);
  IssueInfo issue_info(issue_title, IssueInfo::Action::DISMISS,
                       IssueInfo::Severity::WARNING);
  issue_info.sink_id = sink_id;
  AddIssue(issue_info);
}

void MediaRouterUI::SendIssueForTabAudioNotSupported(
    const MediaSink::Id& sink_id) {
  IssueInfo issue_info(
      l10n_util::GetStringUTF8(IDS_MEDIA_ROUTER_ISSUE_TAB_AUDIO_NOT_SUPPORTED),
      IssueInfo::Action::DISMISS, IssueInfo::Severity::NOTIFICATION);
  issue_info.sink_id = sink_id;
  AddIssue(issue_info);
}

IssueManager* MediaRouterUI::GetIssueManager() {
  return GetMediaRouter()->GetIssueManager();
}

void MediaRouterUI::StartObservingIssues() {
  issues_observer_ =
      std::make_unique<UiIssuesObserver>(GetIssueManager(), this);
  issues_observer_->Init();
}

void MediaRouterUI::OnIssue(const Issue& issue) {
  issue_ = issue;
  UpdateSinks();
}

void MediaRouterUI::OnIssueCleared() {
  issue_ = absl::nullopt;
  UpdateSinks();
}

void MediaRouterUI::OnRoutesUpdated(const std::vector<MediaRoute>& routes) {
  routes_.clear();

  for (const MediaRoute& route : routes) {
#ifndef NDEBUG
    for (const MediaRoute& existing_route : routes_) {
      if (existing_route.media_sink_id() == route.media_sink_id()) {
        DVLOG(2) << "Received another route for display with the same sink"
                 << " id as an existing route. " << route.media_route_id()
                 << " has the same sink id as "
                 << existing_route.media_sink_id() << ".";
      }
    }
#endif
    routes_.push_back(route);
  }

  if (terminating_route_id_ &&
      std::find_if(
          routes.begin(), routes.end(), [this](const MediaRoute& route) {
            return route.media_route_id() == terminating_route_id_.value();
          }) == routes.end()) {
    terminating_route_id_.reset();
  }
  UpdateSinks();
}

void MediaRouterUI::OnSinksUpdated(
    const std::vector<MediaSinkWithCastModes>& sinks) {
  sinks_ = sinks;

  const icu::Collator* collator_ptr = collator_.get();
  std::sort(sinks_.begin(), sinks_.end(),
            [collator_ptr](const MediaSinkWithCastModes& sink1,
                           const MediaSinkWithCastModes& sink2) {
              return sink1.sink.CompareUsingCollator(sink2.sink, collator_ptr);
            });
  UpdateSinks();
}

void MediaRouterUI::OnRouteResponseReceived(
    int route_request_id,
    const MediaSink::Id& sink_id,
    MediaCastMode cast_mode,
    const std::u16string& presentation_request_source_name,
    const RouteRequestResult& result) {
  // If we receive a new route that we aren't expecting, do nothing.
  if (!current_route_request_ || route_request_id != current_route_request_->id)
    return;

  const MediaRoute* route = result.route();
  if (!route) {
    // The provider will handle sending an issue for a failed route request.
    logger_->LogError(mojom::LogCategory::kUi, kLoggerComponent,
                      "MediaRouteResponse returned error: " + result.error(),
                      sink_id, "", "");
  }

  current_route_request_.reset();
  if (result.result_code() == mojom::RouteRequestResultCode::OK &&
      cast_mode == TAB_MIRROR && !base::TimeTicks::IsHighResolution()) {
    // When tab mirroring on a device without a high resolution clock, the audio
    // is not mirrored.
    SendIssueForTabAudioNotSupported(sink_id);
  } else if (result.result_code() == mojom::RouteRequestResultCode::TIMED_OUT) {
    SendIssueForRouteTimeout(cast_mode, sink_id,
                             presentation_request_source_name);
  }
}

void MediaRouterUI::UpdateModelHeader(const std::u16string& source_name) {
  const std::u16string header_text =
      source_name.empty()
          ? l10n_util::GetStringUTF16(IDS_MEDIA_ROUTER_TAB_MIRROR_CAST_MODE)
          : l10n_util::GetStringFUTF16(IDS_MEDIA_ROUTER_PRESENTATION_CAST_MODE,
                                       source_name);
  model_.set_dialog_header(header_text);
  for (CastDialogController::Observer& observer : observers_)
    observer.OnModelUpdated(model_);
}

UIMediaSink MediaRouterUI::ConvertToUISink(const MediaSinkWithCastModes& sink,
                                           const MediaRoute* route,
                                           const absl::optional<Issue>& issue) {
  UIMediaSink ui_sink{sink.sink.provider_id()};
  ui_sink.id = sink.sink.id();
  ui_sink.friendly_name = GetSinkFriendlyName(sink.sink);
  ui_sink.icon_type = sink.sink.icon_type();
  ui_sink.cast_modes = sink.cast_modes;

  if (route) {
    ui_sink.status_text = base::UTF8ToUTF16(route->description());
    ui_sink.route = *route;
    if (terminating_route_id_ &&
        route->media_route_id() == terminating_route_id_.value()) {
      ui_sink.state = UIMediaSinkState::DISCONNECTING;
    } else if (route->is_connecting()) {
      ui_sink.state = UIMediaSinkState::CONNECTING;
    } else {
      ui_sink.state = UIMediaSinkState::CONNECTED;
    }
  } else {
    ui_sink.state = current_route_request() &&
                            sink.sink.id() == current_route_request()->sink_id
                        ? UIMediaSinkState::CONNECTING
                        : UIMediaSinkState::AVAILABLE;
  }
  if (issue && IssueMatches(*issue, ui_sink))
    ui_sink.issue = issue;
  return ui_sink;
}

MediaRouter* MediaRouterUI::GetMediaRouter() const {
  return router_;
}

Browser* MediaRouterUI::GetBrowser() {
  return chrome::FindBrowserWithWebContents(initiator());
}

void MediaRouterUI::SimulateDocumentAvailableForTest() {
  DCHECK(web_contents_observer_for_test_);
  web_contents_observer_for_test_->DidFinishNavigation(nullptr);
}

}  // namespace media_router
