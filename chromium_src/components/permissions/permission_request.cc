/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <optional>
#include <vector>

#include "base/containers/contains.h"
#include "build/build_config.h"
#include "components/grit/mises_components_strings.h"
#include "components/permissions/permission_request.h"
#include "components/strings/grit/components_strings.h"
#include "third_party/widevine/cdm/buildflags.h"

#define PermissionRequest PermissionRequest_ChromiumImpl
#define IsDuplicateOf IsDuplicateOf_ChromiumImpl

// `kWidevine` handled by an override in `WidevinePermissionRequest` and the
// Brave Ethereum/Solana permission has its own permission request prompt.
#if BUILDFLAG(IS_ANDROID)
#define BRAVE_ENUM_ITEMS_FOR_SWITCH                              \
  case RequestType::kMisesEthereum:                              \
  case RequestType::kMisesSolana:                                \
    NOTREACHED_IN_MIGRATION();                                   \
    return permissions::PermissionRequest::AnnotatedMessageText( \
        std::u16string(), {});                                   \
  case RequestType::kWidevine:                                   \
    NOTREACHED_IN_MIGRATION();                                   \
    return permissions::PermissionRequest::AnnotatedMessageText( \
        std::u16string(), {});
#else
#define BRAVE_ENUM_ITEMS_FOR_SWITCH \
  case RequestType::kMisesEthereum: \
  case RequestType::kMisesSolana:   \
    NOTREACHED_IN_MIGRATION();      \
    return std::u16string();        \
  case RequestType::kWidevine:      \
    NOTREACHED_IN_MIGRATION();      \
    return std::u16string();
#endif

// For permission strings that we also need on Android, we need to use
// a string that has a placeholder ($1) in it.
#define BRAVE_ENUM_ITEMS_FOR_SWITCH_DESKTOP                \
  BRAVE_ENUM_ITEMS_FOR_SWITCH                              

#define BRAVE_ENUM_ITEMS_FOR_SWITCH_ANDROID       \
  BRAVE_ENUM_ITEMS_FOR_SWITCH   \
  case RequestType::kCameraPanTiltZoom:\
  case RequestType::kLocalFonts: \
  case RequestType::kRegisterProtocolHandler: \
  case RequestType::kWindowManagement: \
  case RequestType::kFileSystemAccess: \
  case RequestType::kCapturedSurfaceControl:\
  case RequestType::kKeyboardLock: \
  case RequestType::kPointerLock: \
  case RequestType::kWebAppInstallation: \
      return permissions::PermissionRequest::AnnotatedMessageText( \
        std::u16string(), {});                                   

#define BRAVE_ENUM_ITEMS_FOR_SWITCH_ANDROID_MESSAGE       \
  case RequestType::kMisesEthereum:                              \
  case RequestType::kMisesSolana:                                \
  case RequestType::kWidevine:                                   \
  case RequestType::kNfcDevice: \
  case RequestType::kProtectedMediaIdentifier: \
  return std::u16string();   


namespace {
#if BUILDFLAG(IS_ANDROID)
const unsigned int IDS_VR_INFOBAR_TEXT_OVERRIDE = IDS_VR_INFOBAR_TEXT;
#endif
const unsigned int IDS_VR_PERMISSION_FRAGMENT_OVERRIDE =
    IDS_VR_PERMISSION_FRAGMENT;

}  // namespace

#if BUILDFLAG(IS_ANDROID)
// For PermissionRequest::GetDialogMessageText
#undef IDS_VR_INFOBAR_TEXT
#define IDS_VR_INFOBAR_TEXT     \
  IDS_VR_INFOBAR_TEXT_OVERRIDE; \
  break;                        \
  BRAVE_ENUM_ITEMS_FOR_SWITCH_ANDROID

// For PermissionRequest::GetMessageTextFragment
#undef IDS_VR_PERMISSION_FRAGMENT
#define IDS_VR_PERMISSION_FRAGMENT     \
  IDS_VR_PERMISSION_FRAGMENT_OVERRIDE; \
  break;                               \
  BRAVE_ENUM_ITEMS_FOR_SWITCH_ANDROID_MESSAGE

#else
// For PermissionRequest::GetMessageTextFragment
#undef IDS_VR_PERMISSION_FRAGMENT
#define IDS_VR_PERMISSION_FRAGMENT     \
  IDS_VR_PERMISSION_FRAGMENT_OVERRIDE; \
  break;                               \
  BRAVE_ENUM_ITEMS_FOR_SWITCH_DESKTOP
#endif

#include "src/components/permissions/permission_request.cc"
#undef IDS_VR_INFOBAR_TEXT
#undef IDS_VR_PERMISSION_FRAGMENT
#undef BRAVE_ENUM_ITEMS_FOR_SWITCH_ANDROID
#undef BRAVE_ENUM_ITEMS_FOR_SWITCH_DESKTOP
#undef BRAVE_ENUM_ITEMS_FOR_SWITCH
#undef IsDuplicateOf
#undef PermissionRequest

namespace permissions {

PermissionRequest::PermissionRequest(
    const GURL& requesting_origin,
    RequestType request_type,
    bool has_gesture,
    PermissionDecidedCallback permission_decided_callback,
    base::OnceClosure delete_callback)
    : PermissionRequest_ChromiumImpl(requesting_origin,
                                     request_type,
                                     has_gesture,
                                     std::move(permission_decided_callback),
                                     std::move(delete_callback)) {}

PermissionRequest::PermissionRequest(
    PermissionRequestData request_data,
    PermissionDecidedCallback permission_decided_callback,
    base::OnceClosure delete_callback,
    bool uses_automatic_embargo)
    : PermissionRequest_ChromiumImpl(std::move(request_data),
                                     std::move(permission_decided_callback),
                                     std::move(delete_callback),
                                     uses_automatic_embargo) {}

PermissionRequest::~PermissionRequest() = default;

#if BUILDFLAG(IS_ANDROID)
PermissionRequest::AnnotatedMessageText
PermissionRequest::GetDialogAnnotatedMessageText(
    const GURL& embedding_origin) const {
  if (request_type() != RequestType::kStorageAccess) {
    return PermissionRequest_ChromiumImpl::GetDialogAnnotatedMessageText(
        embedding_origin);
  }
  std::u16string requesting_origin_string_formatted =
      url_formatter::FormatUrlForSecurityDisplay(
          requesting_origin(),
          url_formatter::SchemeDisplay::OMIT_CRYPTOGRAPHIC);
  std::u16string embedding_origin_string_formatted =
      url_formatter::FormatUrlForSecurityDisplay(
          embedding_origin, url_formatter::SchemeDisplay::OMIT_CRYPTOGRAPHIC);
  return AnnotatedMessageText(
      l10n_util::GetStringFUTF16(IDS_STORAGE_ACCESS_INFOBAR_TEXT,
                                 requesting_origin_string_formatted,
                                 embedding_origin_string_formatted),
      /*bolded_ranges=*/{});
}

// static
PermissionRequest::AnnotatedMessageText
PermissionRequest::GetDialogAnnotatedMessageText(
    std::u16string requesting_origin_formatted_for_display,
    int message_id,
    bool format_origin_bold) {
  return PermissionRequest_ChromiumImpl::GetDialogAnnotatedMessageText(
      requesting_origin_formatted_for_display, message_id, format_origin_bold);
}
#endif

bool PermissionRequest::SupportsLifetime() const {
  const RequestType kExcludedTypes[] = {
    RequestType::kDiskQuota,
    RequestType::kMultipleDownloads,
#if BUILDFLAG(IS_ANDROID)
    RequestType::kProtectedMediaIdentifier,
#else
    RequestType::kRegisterProtocolHandler,
#endif  // BUILDFLAG(IS_ANDROID)
#if BUILDFLAG(ENABLE_WIDEVINE)
    RequestType::kWidevine
#endif  // BUILDFLAG(ENABLE_WIDEVINE)
  };
  return !base::Contains(kExcludedTypes, request_type());
}

void PermissionRequest::SetLifetime(std::optional<base::TimeDelta> lifetime) {
  DCHECK(SupportsLifetime());
  lifetime_ = std::move(lifetime);
}

const std::optional<base::TimeDelta>& PermissionRequest::GetLifetime() const {
  DCHECK(SupportsLifetime());
  return lifetime_;
}

bool PermissionRequest::IsDuplicateOf(PermissionRequest* other_request) const {
  return PermissionRequest_ChromiumImpl::IsDuplicateOf_ChromiumImpl(
      other_request);
}

base::WeakPtr<PermissionRequest> PermissionRequest::GetWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

}  // namespace permissions
