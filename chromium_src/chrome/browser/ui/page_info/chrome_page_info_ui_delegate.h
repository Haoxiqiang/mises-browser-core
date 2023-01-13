/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_CHROME_BROWSER_UI_PAGE_INFO_CHROME_PAGE_INFO_UI_DELEGATE_H_
#define BRAVE_CHROMIUM_SRC_CHROME_BROWSER_UI_PAGE_INFO_CHROME_PAGE_INFO_UI_DELEGATE_H_

#include "components/page_info/page_info_ui_delegate.h"

#if !BUILDFLAG(IS_ANDROID)
#define ShouldShowAsk                     \
  AddIPFSTabForURL(const GURL& ipfs_url); \
  bool ShouldShowAsk

#else

#define ShouldShowAsk                     \
  AddIPFSTabForURL(const GURL& ipfs_url); \
  bool ShouldShowSiteSettings(int* link_text_id, int* tooltip_text_id); \
  std::u16string GetPermissionDetail(ContentSettingsType type); \
  void ShowPrivacySandboxAdPersonalization(); \
  bool IsBlockAutoPlayEnabled() override; \
  bool IsMultipleTabsOpen() override; \
  bool ShouldShowAsk

#endif

#include "src/chrome/browser/ui/page_info/chrome_page_info_ui_delegate.h"

#undef ShouldShowAsk

#endif  // BRAVE_CHROMIUM_SRC_CHROME_BROWSER_UI_PAGE_INFO_CHROME_PAGE_INFO_UI_DELEGATE_H_

