/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#define CreateJavaDelegate                                      \
  Java_PermissionDialogController_createDialog_MisesImpl(       \
      JNIEnv* env, const base::android::JavaRef<jobject>& obj); \
  virtual void CreateJavaDelegate
#include "components/permissions/android/permission_prompt/permission_dialog_delegate.h"
#undef CreateJavaDelegate

#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "mises/components/l10n/common/localization_util.h"
#include "mises/components/permissions/android/jni_headers/MisesPermissionDialogDelegate_jni.h"
#include "mises/components/permissions/permission_lifetime_utils.h"
#include "mises/components/permissions/permission_widevine_utils.h"
#include "components/grit/mises_components_strings.h"
#include "components/permissions/android/jni_headers/PermissionDialogController_jni.h"
#include "components/permissions/android/permission_prompt/permission_prompt_android.h"
#include "components/permissions/features.h"
#include "components/strings/grit/components_strings.h"
#include "third_party/widevine/cdm/buildflags.h"

namespace permissions {
namespace {

void SetLifetimeOptions(const base::android::JavaRef<jobject>& j_delegate) {
  if (!base::FeatureList::IsEnabled(features::kPermissionLifetime)) {
    return;
  }

  JNIEnv* env = base::android::AttachCurrentThread();
  Java_MisesPermissionDialogDelegate_setLifetimeOptionsText(
      env, j_delegate,
      base::android::ConvertUTF16ToJavaString(
          env, brave_l10n::GetLocalizedResourceUTF16String(
                   IDS_PERMISSIONS_BUBBLE_LIFETIME_COMBOBOX_LABEL)));

  std::vector<PermissionLifetimeOption> lifetime_options =
      CreatePermissionLifetimeOptions();
  std::vector<std::u16string> lifetime_labels;
  for (const auto& lifetime_option : lifetime_options) {
    lifetime_labels.push_back(lifetime_option.label);
  }

  Java_MisesPermissionDialogDelegate_setLifetimeOptions(
      env, j_delegate,
      base::android::ToJavaArrayOfStrings(env, lifetime_labels));
}

void ApplyLifetimeToPermissionRequests(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    PermissionPromptAndroid* permission_prompt) {
  if (!base::FeatureList::IsEnabled(features::kPermissionLifetime)) {
    return;
  }
  const int selected_lifetime_option =
      Java_MisesPermissionDialogDelegate_getSelectedLifetimeOption(env, obj);
  DCHECK(!ShouldShowLifetimeOptions(permission_prompt->delegate()) ||
         selected_lifetime_option != -1);
  if (selected_lifetime_option != -1) {
    std::vector<PermissionLifetimeOption> lifetime_options =
        CreatePermissionLifetimeOptions();
    SetRequestsLifetime(lifetime_options, selected_lifetime_option,
                        permission_prompt->delegate());
  }
}



void ApplyDontAskAgainOption(JNIEnv* env,
                             const JavaParamRef<jobject>& obj,
                             PermissionPromptAndroid* permission_prompt) {
  if (permission_prompt->delegate()->Requests().size() < 1) {
    return;
  }

  const bool dont_ask_again =
      Java_MisesPermissionDialogDelegate_getDontAskAgain(env, obj);
  PermissionRequest* request = permission_prompt->delegate()->Requests()[0];
  request->set_dont_ask_again(dont_ask_again);
}

}  // namespace

void PermissionDialogJavaDelegate::
    Java_PermissionDialogController_createDialog_MisesImpl(
    JNIEnv* env,
    const base::android::JavaRef<jobject>& j_delegate) {
#if BUILDFLAG(ENABLE_WIDEVINE)
  if (HasWidevinePermissionRequest(
          permission_prompt_->delegate()->Requests())) {
    Java_MisesPermissionDialogDelegate_setIsWidevinePermissionRequest(
        env, j_delegate, true);
  }
#else
    Java_MisesPermissionDialogDelegate_setIsWidevinePermissionRequest(
        env, j_delegate, false);
#endif
  if (ShouldShowLifetimeOptions(permission_prompt_->delegate())) {
    SetLifetimeOptions(j_delegate);
  }
  Java_PermissionDialogController_createDialog(env, j_delegate);
}

}  // namespace permissions

#define MISES_PERMISSION_DIALOG_DELEGATE_ACCEPT                    \
  ApplyLifetimeToPermissionRequests(env, obj, permission_prompt_); \
  ApplyDontAskAgainOption(env, obj, permission_prompt_);
#define MISES_PERMISSION_DIALOG_DELEGATE_CANCEL                    \
  ApplyLifetimeToPermissionRequests(env, obj, permission_prompt_); \
  ApplyDontAskAgainOption(env, obj, permission_prompt_);
#define Java_PermissionDialogController_createDialog \
  Java_PermissionDialogController_createDialog_MisesImpl

#include "src/components/permissions/android/permission_prompt/permission_dialog_delegate.cc"

#undef Java_PermissionDialogController_createDialog
#undef MISES_PERMISSION_DIALOG_DELEGATE_CANCEL
#undef MISES_PERMISSION_DIALOG_DELEGATE_ACCEPT
