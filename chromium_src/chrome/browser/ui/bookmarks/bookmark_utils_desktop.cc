// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/bookmarks/bookmark_utils_desktop.h"

#include <iterator>
#include <numeric>

#include "base/bind.h"
#include "base/callback.h"
#include "base/containers/contains.h"
#include "base/containers/flat_set.h"
#include "base/strings/string_number_conversions.h"
#include "build/build_config.h"
#include "chrome/browser/bookmarks/bookmark_model_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/bookmarks/bookmark_editor.h"
#include "chrome/browser/ui/bookmarks/bookmark_stats.h"
#include "chrome/browser/ui/bookmarks/bookmark_utils.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_navigator.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/simple_message_box.h"
#include "chrome/browser/ui/tabs/saved_tab_groups/saved_tab_group.h"
#include "chrome/browser/ui/tabs/saved_tab_groups/saved_tab_group_keyed_service.h"
#include "chrome/browser/ui/tabs/saved_tab_groups/saved_tab_group_service_factory.h"
#include "chrome/browser/ui/tabs/tab_group.h"
#include "chrome/browser/ui/tabs/tab_group_model.h"
#include "chrome/grit/chromium_strings.h"
#include "chrome/grit/generated_resources.h"
#include "components/bookmarks/browser/bookmark_model.h"
#include "components/bookmarks/browser/bookmark_node.h"
#include "components/bookmarks/browser/bookmark_utils.h"
#include "components/tab_groups/tab_group_id.h"
#include "content/public/browser/page_navigator.h"
#include "content/public/browser/web_contents.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/window_open_disposition.h"
#include "url/gurl.h"

using bookmarks::BookmarkModel;
using bookmarks::BookmarkNode;

namespace chrome {

size_t kNumBookmarkUrlsBeforePrompting = 15;

namespace {

// Returns a vector of all URLs in |nodes| and their immediate children.  Only
// recurses one level deep, not infinitely.  TODO(pkasting): It's not clear why
// this shouldn't just recurse infinitely.
std::vector<GURL> GetURLsToOpen(
    const std::vector<const BookmarkNode*>& nodes,
    content::BrowserContext* browser_context = nullptr,
    bool incognito_urls_only = false) {
  std::vector<GURL> urls;

  const auto AddUrlIfLegal = [&](const GURL url) {
    if (!incognito_urls_only || IsURLAllowedInIncognito(url, browser_context))
      urls.push_back(url);
  };

  for (const BookmarkNode* node : nodes) {
    if (node->is_url()) {
      AddUrlIfLegal(node->url());
    } else {
      // If the node is not a URL, it is a folder. We want to add those of its
      // children which are URLs.
      for (const auto& child : node->children()) {
        if (child->is_url())
          AddUrlIfLegal(child->url());
      }
    }
  }
  return urls;
}

// Returns the total number of descendants nodes.
int ChildURLCountTotal(const BookmarkNode* node) {
  const auto count_children = [](int total, const auto& child) {
    if (child->is_folder())
      total += ChildURLCountTotal(child.get());
    return total + 1;
  };
  return std::accumulate(node->children().cbegin(), node->children().cend(), 0,
                         count_children);
}

#if true ||!BUILDFLAG(IS_ANDROID)
// Returns in |urls|, the url and title pairs for each open tab in browser.
void GetURLsAndFoldersForOpenTabs(
    Browser* browser,
    std::vector<BookmarkEditor::EditDetails::BookmarkData>* folder_data) {
  std::vector<std::pair<GURL, std::u16string>> tab_entries;
  base::flat_map<int, TabGroupData> groups_by_index;
  for (int i = 0; i < browser->tab_strip_model()->count(); ++i) {
    std::pair<GURL, std::u16string> entry;
    auto* contents = browser->tab_strip_model()->GetWebContentsAt(i);
    GetURLAndTitleToBookmark(contents, &(entry.first), &(entry.second));
    tab_entries.push_back(entry);
    auto tab_group_id = browser->tab_strip_model()->GetTabGroupForTab(i);
    std::u16string title;
    if (tab_group_id.has_value()) {
      title = browser->tab_strip_model()
                  ->group_model()
                  ->GetTabGroup(tab_group_id.value())
                  ->visual_data()
                  ->title();
    }
    groups_by_index.emplace(i, std::make_pair(tab_group_id, title));
  }
  GetURLsAndFoldersForTabEntries(folder_data, tab_entries, groups_by_index);
}
#endif

// Represents a reference set of web contents opened by OpenAllHelper() so that
// the actual web contents and what browsers they are located in can be
// determined (if necessary).
using OpenedWebContentsSet = base::flat_set<const content::WebContents*>;

// Opens all of the URLs in `bookmark_urls` using `navigator` and
// `initial_disposition` as a starting point. Returns a reference set of the
// WebContents created; see OpenedWebContentsSet.
OpenedWebContentsSet OpenAllHelper(content::PageNavigator* navigator,
                                   std::vector<GURL> bookmark_urls,
                                   WindowOpenDisposition initial_disposition) {
  OpenedWebContentsSet::container_type opened_tabs;
  WindowOpenDisposition disposition = initial_disposition;
  for (std::vector<GURL>::const_iterator url_it = bookmark_urls.begin();
       url_it != bookmark_urls.end(); ++url_it) {
    content::WebContents* opened_tab = navigator->OpenURL(
        content::OpenURLParams(*url_it, content::Referrer(), disposition,
                               ui::PAGE_TRANSITION_AUTO_BOOKMARK, false));
    if (url_it == bookmark_urls.begin()) {
      // We opened the first URL which may have opened a new window or clobbered
      // the current page, reset the navigator just to be sure. |opened_tab| may
      // be null in tests.
      if (opened_tab)
        navigator = opened_tab;
      disposition = WindowOpenDisposition::NEW_BACKGROUND_TAB;
    }

    if (opened_tab)
      opened_tabs.push_back(opened_tab);
  }

  // Constructing the return value in this way is significantly more efficient.
  return OpenedWebContentsSet(std::move(opened_tabs));
}

}  // namespace

#if true || !BUILDFLAG(IS_ANDROID)
void OpenAllIfAllowed(
    Browser* browser,
    base::OnceCallback<content::PageNavigator*()> get_navigator,
    const std::vector<const bookmarks::BookmarkNode*>& nodes,
    WindowOpenDisposition initial_disposition,
    bool add_to_group) {
  std::vector<GURL> urls = GetURLsToOpen(
      nodes, browser->profile(),
      initial_disposition == WindowOpenDisposition::OFF_THE_RECORD);

  auto do_open = [](Browser* browser,
                    base::OnceCallback<content::PageNavigator*()> get_navigator,
                    std::vector<GURL> urls,
                    WindowOpenDisposition initial_disposition,
                    absl::optional<std::u16string> folder_title,
                    chrome::MessageBoxResult result) {
    if (result != chrome::MESSAGE_BOX_RESULT_YES)
      return;
    if (!get_navigator)
      return;
    content::PageNavigator* navigator = std::move(get_navigator).Run();
    if (!navigator)
      return;
    const auto opened_web_contents =
        OpenAllHelper(navigator, std::move(urls), initial_disposition);
    if (folder_title.has_value()) {
      TabStripModel* model = browser->tab_strip_model();

      // Figure out which tabs we actually opened in this browser that aren't
      // already in groups.
      std::vector<int> tab_indices;
      for (int i = 0; i < model->count(); ++i) {
        if (base::Contains(opened_web_contents, model->GetWebContentsAt(i)) &&
            !model->GetTabGroupForTab(i).has_value()) {
          tab_indices.push_back(i);
        }
      }

      if (tab_indices.empty())
        return;

      absl::optional<tab_groups::TabGroupId> new_group_id =
          model->AddToNewGroup(tab_indices);
      if (!new_group_id.has_value())
        return;

      // Use the bookmark folder's title as the group's title.
      TabGroup* group = model->group_model()->GetTabGroup(new_group_id.value());
      const tab_groups::TabGroupVisualData* current_visual_data =
          group->visual_data();
      tab_groups::TabGroupVisualData new_visual_data(
          folder_title.value(), current_visual_data->color(),
          current_visual_data->is_collapsed());
      group->SetVisualData(new_visual_data);

      model->OpenTabGroupEditor(new_group_id.value());
    }
  };

  // Skip the prompt if there are few bookmarks.
  size_t child_count = urls.size();
  if (child_count < kNumBookmarkUrlsBeforePrompting) {
    do_open(
        browser, std::move(get_navigator), std::move(urls), initial_disposition,
        add_to_group
            ? absl::optional<std::u16string>(nodes[0]->GetTitledUrlNodeTitle())
            : absl::nullopt,
        chrome::MESSAGE_BOX_RESULT_YES);
    return;
  }

  // The callback passed contains the pointer |browser|. This is safe
  // since if |browser| is closed, the message box will be destroyed
  // before the user can answer "Yes".

  ShowQuestionMessageBox(
      browser->window()->GetNativeWindow(),
      l10n_util::GetStringUTF16(IDS_PRODUCT_NAME),
      l10n_util::GetStringFUTF16(IDS_BOOKMARK_BAR_SHOULD_OPEN_ALL,
                                 base::NumberToString16(child_count)),
      base::BindOnce(do_open, browser, std::move(get_navigator),
                     std::move(urls), initial_disposition,
                     add_to_group ? absl::optional<std::u16string>(
                                        nodes[0]->GetTitledUrlNodeTitle())
                                  : absl::nullopt));
}

void OpenAllNow(content::PageNavigator* navigator,
                const std::vector<const BookmarkNode*>& nodes,
                WindowOpenDisposition initial_disposition,
                content::BrowserContext* browser_context) {
  // Opens all |nodes| of type URL and any children of |nodes| that are of type
  // URL. |navigator| is the PageNavigator used to open URLs. After the first
  // url is opened |navigator| is set to the PageNavigator of the last active
  // tab. This is done to handle a window disposition of new window, in which
  // case we want subsequent tabs to open in that window.

  std::vector<GURL> urls = GetURLsToOpen(
      nodes, browser_context,
      initial_disposition == WindowOpenDisposition::OFF_THE_RECORD);

  OpenAllHelper(navigator, std::move(urls), initial_disposition);
}

void OpenSavedTabGroupHelper(
    Browser* browser,
    base::OnceCallback<content::PageNavigator*()> get_navigator,
    const base::GUID& saved_group_id,
    chrome::MessageBoxResult result) {
  if (result != chrome::MESSAGE_BOX_RESULT_YES)
    return;
  if (!get_navigator)
    return;
  content::PageNavigator* navigator = std::move(get_navigator).Run();
  if (!navigator)
    return;

  SavedTabGroupKeyedService* service =
      SavedTabGroupServiceFactory::GetForProfile(browser->profile());

  const SavedTabGroup* saved_group = service->model()->Get(saved_group_id);

  // In the case where this function is called after confirmation of an
  // interstitial, the saved_group could be null, so protect against this by
  // early returning.
  if (!saved_group)
    return;

  if (saved_group->tab_group_id().has_value()) {
    TabStripModel* model_for_activation =
        service->listener()->GetTabStripModelWithTabGroupId(
            saved_group->tab_group_id().value());

    // Only activate the tab group's first tab if it exists in any browser's
    // tabstrip model.
    if (model_for_activation) {
      absl::optional<int> first_tab =
          model_for_activation->group_model()
              ->GetTabGroup(saved_group->tab_group_id().value())
              ->GetFirstTab();
      DCHECK(first_tab.has_value());
      model_for_activation->ActivateTabAt(first_tab.value());
      return;
    }
  }

  // If our tab group was not found in any tabstrip model, open the group in
  // this browser's tabstrip model.
  TabStripModel* model_for_creation = browser->tab_strip_model();

  std::vector<GURL> urls;
  auto get_urls = [&](SavedTabGroupTab saved_tab) { return saved_tab.url; };
  base::ranges::transform(saved_group->saved_tabs(), std::back_inserter(urls),
                          get_urls);
  if (urls.empty())
    return;

  const auto opened_web_contents = OpenAllHelper(
      navigator, std::move(urls), WindowOpenDisposition::NEW_BACKGROUND_TAB);

  // Figure out which tabs we actually opened in this browser that aren't
  // already in groups.
  std::vector<int> tab_indices;
  for (int i = 0; i < model_for_creation->count(); ++i) {
    if (base::Contains(opened_web_contents,
                       model_for_creation->GetWebContentsAt(i)) &&
        !model_for_creation->GetTabGroupForTab(i).has_value()) {
      tab_indices.push_back(i);
    }
  }

  const std::u16string& title = saved_group->title();
  const tab_groups::TabGroupColorId& color = saved_group->color();
  tab_groups::TabGroupId tab_group_id = tab_groups::TabGroupId::GenerateNew();

  // If the group does not exist, create a tab group with the same group_id.
  model_for_creation->AddToGroupForRestore(tab_indices, tab_group_id);
  TabGroup* group =
      model_for_creation->group_model()->GetTabGroup(tab_group_id);
  tab_groups::TabGroupVisualData visual_data(title, color,
                                             /*is_collapsed=*/false);
  group->SetVisualData(visual_data, /*is_customized=*/true);

  service->model()->OnGroupOpenedInTabStrip(saved_group->saved_guid(),
                                            tab_group_id);

  absl::optional<int> first_tab =
      model_for_creation->group_model()
          ->GetTabGroup(saved_group->tab_group_id().value())
          ->GetFirstTab();
  DCHECK(first_tab.has_value());
  model_for_creation->ActivateTabAt(first_tab.value());
}

void OpenSavedTabGroup(
    Browser* browser,
    base::OnceCallback<content::PageNavigator*()> get_navigator,
    const base::GUID& saved_group_id,
    const size_t num_tabs) {
  // Skip the prompt if there are few bookmarks.
  if (num_tabs < kNumBookmarkUrlsBeforePrompting) {
    OpenSavedTabGroupHelper(browser, std::move(get_navigator), saved_group_id,
                            chrome::MESSAGE_BOX_RESULT_YES);
    return;
  }

  // The callback passed contains the pointer |browser|. This is safe
  // since if |browser| is closed, the message box will be destroyed
  // before the user can answer "Yes".
  ShowQuestionMessageBox(
      browser->window()->GetNativeWindow(),
      l10n_util::GetStringUTF16(IDS_PRODUCT_NAME),
      l10n_util::GetStringFUTF16(IDS_BOOKMARK_BAR_SHOULD_OPEN_ALL,
                                 base::NumberToString16(num_tabs)),
      base::BindOnce(&OpenSavedTabGroupHelper, browser,
                     std::move(get_navigator), saved_group_id));
}

int OpenCount(gfx::NativeWindow parent,
              const std::vector<const bookmarks::BookmarkNode*>& nodes,
              content::BrowserContext* incognito_context) {
  return GetURLsToOpen(nodes, incognito_context, incognito_context != nullptr)
      .size();
}

int OpenCount(gfx::NativeWindow parent,
              const BookmarkNode* node,
              content::BrowserContext* incognito_context) {
  std::vector<const BookmarkNode*> nodes;
  nodes.push_back(node);
  return OpenCount(parent, std::vector<const bookmarks::BookmarkNode*>{node},
                   incognito_context);
}

bool ConfirmDeleteBookmarkNode(gfx::NativeWindow window,
                               const BookmarkNode* node) {
  DCHECK(node && node->is_folder() && !node->children().empty());
  return ShowQuestionMessageBoxSync(
             window, l10n_util::GetStringUTF16(IDS_PRODUCT_NAME),
             l10n_util::GetPluralStringFUTF16(
                 IDS_BOOKMARK_EDITOR_CONFIRM_DELETE,
                 ChildURLCountTotal(node))) == MESSAGE_BOX_RESULT_YES;
}

void ShowBookmarkAllTabsDialog(Browser* browser) {
  Profile* profile = browser->profile();
  BookmarkModel* model = BookmarkModelFactory::GetForBrowserContext(profile);
  DCHECK(model && model->loaded());

  const BookmarkNode* parent = GetParentForNewNodes(model);
  BookmarkEditor::EditDetails details =
      BookmarkEditor::EditDetails::AddFolder(parent, parent->children().size());

  GetURLsAndFoldersForOpenTabs(browser, &(details.bookmark_data.children));
  DCHECK(!details.bookmark_data.children.empty());
  BookmarkEditor::Show(browser->window()->GetNativeWindow(), profile, details,
                       BookmarkEditor::SHOW_TREE,
                       base::BindOnce(
                           [](const Profile* profile) {
                             // We record the profile that invoked this option.
                             RecordBookmarksAdded(profile);
                           },
                           base::Unretained(profile)));
}

bool HasBookmarkURLs(const std::vector<const BookmarkNode*>& selection) {
  return !GetURLsToOpen(selection).empty();
}

bool HasBookmarkURLsAllowedInIncognitoMode(
    const std::vector<const BookmarkNode*>& selection,
    content::BrowserContext* browser_context) {
  return !GetURLsToOpen(selection, browser_context, true).empty();
}

void GetURLsAndFoldersForTabEntries(
    std::vector<BookmarkEditor::EditDetails::BookmarkData>* folder_data,
    std::vector<std::pair<GURL, std::u16string>> tab_entries,
    base::flat_map<int, TabGroupData> groups_by_index) {
  absl::optional<tab_groups::TabGroupId> current_group_id;
  for (size_t i = 0; i < tab_entries.size(); ++i) {
    std::pair<GURL, std::u16string> entry = tab_entries.at(i);
    if (entry.first.is_empty()) {
      continue;
    }
    BookmarkEditor::EditDetails::BookmarkData child;
    child.url = entry.first;
    child.title = entry.second;
    if (groups_by_index.at(i).first.has_value()) {
      if (current_group_id != groups_by_index.at(i).first.value()) {
        BookmarkEditor::EditDetails::BookmarkData tab_group;
        tab_group.title = groups_by_index.at(i).second;
        folder_data->push_back(tab_group);
        current_group_id = groups_by_index.at(i).first;
      }
      folder_data->back().children.push_back(child);
    } else {
      folder_data->push_back(child);
    }
  }
}
#endif  // !BUILDFLAG(IS_ANDROID)

}  // namespace chrome
