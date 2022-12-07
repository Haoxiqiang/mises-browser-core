// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/bookmarks/android/bookmark_bridge.h"

#include <stddef.h>
#include <stdint.h>

#include <algorithm>
#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/bind.h"
#include "base/containers/adapters.h"
#include "base/containers/stack.h"
#include "base/feature_list.h"
#include "base/guid.h"
#include "base/i18n/string_compare.h"
#include "base/memory/raw_ptr.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/android/chrome_jni_headers/BookmarkBridge_jni.h"
#include "chrome/browser/android/bookmarks/partner_bookmarks_reader.h"
#include "chrome/browser/android/reading_list/reading_list_manager_factory.h"
#include "chrome/browser/bookmarks/bookmark_model_factory.h"
#include "chrome/browser/bookmarks/managed_bookmark_service_factory.h"
#include "chrome/browser/commerce/shopping_list/shopping_data_provider.h"
#include "chrome/browser/optimization_guide/optimization_guide_keyed_service.h"
#include "chrome/browser/optimization_guide/optimization_guide_keyed_service_factory.h"
#include "chrome/browser/profiles/incognito_helpers.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_android.h"
#include "chrome/browser/signin/identity_manager_factory.h"
#include "chrome/browser/undo/bookmark_undo_service_factory.h"
#include "components/bookmarks/browser/bookmark_model.h"
#include "components/bookmarks/browser/bookmark_utils.h"
#include "components/bookmarks/browser/scoped_group_bookmark_actions.h"
#include "components/bookmarks/browser/titled_url_match.h"
#include "components/bookmarks/common/android/bookmark_type.h"
#include "components/bookmarks/common/bookmark_pref_names.h"
#include "components/bookmarks/managed/managed_bookmark_service.h"
#include "components/commerce/core/commerce_feature_list.h"
#include "components/commerce/core/proto/price_tracking.pb.h"
#include "components/dom_distiller/core/url_utils.h"
#include "components/optimization_guide/proto/hints.pb.h"
#include "components/power_bookmarks/core/power_bookmark_utils.h"
#include "components/power_bookmarks/core/proto/power_bookmark_meta.pb.h"
#include "components/prefs/pref_service.h"
#include "components/query_parser/query_parser.h"
#include "components/signin/public/identity_manager/identity_manager.h"
#include "components/undo/bookmark_undo_service.h"
#include "components/undo/undo_manager.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"

#include "base/files/file_util.h"
#include "chrome/common/chrome_paths_internal.h"
#include "chrome/browser/ui/chrome_select_file_policy.h"
#include "base/android/content_uri_utils.h"
#include "base/android/path_utils.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/utility/importer/bookmark_html_reader.h"
#include "chrome/browser/bookmarks/bookmark_html_writer.h"
#include "chrome/browser/importer/profile_writer.h"
#include "chrome/browser/platform_util.h"
#include "chrome/browser/ui/chrome_select_file_policy.h"
#include "chrome/common/importer/imported_bookmark_entry.h"
#include "chrome/common/importer/importer_data_types.h"
#include "chrome/common/url_constants.h"
#include "components/url_formatter/url_fixer.h"
#include "ui/android/window_android.h"

using base::android::AttachCurrentThread;
using base::android::ConvertUTF16ToJavaString;
using base::android::ConvertUTF8ToJavaString;
using base::android::JavaParamRef;
using base::android::JavaRef;
using base::android::ScopedJavaGlobalRef;
using base::android::ScopedJavaLocalRef;
using base::android::ToJavaIntArray;
using bookmarks::BookmarkModel;
using bookmarks::BookmarkNode;
using bookmarks::BookmarkType;
using bookmarks::android::JavaBookmarkIdCreateBookmarkId;
using bookmarks::android::JavaBookmarkIdGetId;
using bookmarks::android::JavaBookmarkIdGetType;
using content::BrowserThread;
using power_bookmarks::PowerBookmarkMeta;

namespace {

class BookmarkTitleComparer {
 public:
  explicit BookmarkTitleComparer(BookmarkBridge* bookmark_bridge,
                                 const icu::Collator* collator)
      : bookmark_bridge_(bookmark_bridge), collator_(collator) {}

  bool operator()(const BookmarkNode* lhs, const BookmarkNode* rhs) {
    if (collator_) {
      return base::i18n::CompareString16WithCollator(
                 *collator_, bookmark_bridge_->GetTitle(lhs),
                 bookmark_bridge_->GetTitle(rhs)) == UCOL_LESS;
    } else {
      return lhs->GetTitle() < rhs->GetTitle();
    }
  }

 private:
  raw_ptr<BookmarkBridge> bookmark_bridge_;  // weak
  raw_ptr<const icu::Collator> collator_;
};

std::unique_ptr<icu::Collator> GetICUCollator() {
  UErrorCode error = U_ZERO_ERROR;
  std::unique_ptr<icu::Collator> collator_;
  collator_.reset(icu::Collator::createInstance(error));
  if (U_FAILURE(error))
    collator_.reset(nullptr);

  return collator_;
}

}  // namespace

BookmarkBridge::BookmarkBridge(JNIEnv* env,
                               const JavaRef<jobject>& obj,
                               const JavaRef<jobject>& j_profile)
    : weak_java_ref_(env, obj),
      bookmark_model_(nullptr),
      managed_bookmark_service_(nullptr),
      partner_bookmarks_shim_(nullptr),
      weak_ptr_factory_(this) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  profile_ = ProfileAndroid::FromProfileAndroid(j_profile);
  profile_observation_.Observe(profile_.get());
  bookmark_model_ = BookmarkModelFactory::GetForBrowserContext(profile_);
  managed_bookmark_service_ =
      ManagedBookmarkServiceFactory::GetForProfile(profile_);
  opt_guide_ = OptimizationGuideKeyedServiceFactory::GetForProfile(profile_);

  // Registers the notifications we are interested.
  bookmark_model_->AddObserver(this);

  // Create the partner Bookmarks shim as early as possible (but don't attach).
  partner_bookmarks_shim_ = PartnerBookmarksShim::BuildForBrowserContext(
      chrome::GetBrowserContextRedirectedInIncognito(profile_));
  partner_bookmarks_shim_->AddObserver(this);

  reading_list_manager_ =
      ReadingListManagerFactory::GetForBrowserContext(profile_);
  reading_list_manager_->AddObserver(this);

  pref_change_registrar_.Init(profile_->GetPrefs());
  pref_change_registrar_.Add(
      bookmarks::prefs::kEditBookmarksEnabled,
      base::BindRepeating(&BookmarkBridge::EditBookmarksEnabledChanged,
                          base::Unretained(this)));

  NotifyIfDoneLoading();

  // Since a sync or import could have started before this class is
  // initialized, we need to make sure that our initial state is
  // up to date.
  if (bookmark_model_->IsDoingExtensiveChanges())
    ExtensiveBookmarkChangesBeginning(bookmark_model_);
}

BookmarkBridge::~BookmarkBridge() {
  if (profile_) {
    DCHECK(profile_observation_.IsObservingSource(profile_.get()));
    profile_observation_.Reset();
  }
  bookmark_model_->RemoveObserver(this);
  if (partner_bookmarks_shim_)
    partner_bookmarks_shim_->RemoveObserver(this);
  reading_list_manager_->RemoveObserver(this);
}


void BookmarkBridge::FileSelected(const base::FilePath& file_path, int index,
                            void* params) {
  LOG(ERROR) << "Bookmarks - Bookmarks file to be imported is present in " << file_path;

  base::FilePath file_path_tmp;
  if (!base::android::GetCacheDirectory(&file_path_tmp)) {
    LOG(ERROR) << "Bookmarks - Getting Cache Directory for Import";
    return;
  }
  file_path_tmp = file_path_tmp.Append(FILE_PATH_LITERAL("bookmarks.html.tmp"));

  LOG(ERROR) << "Bookmarks - Copying from " << file_path << " to " << file_path_tmp;

  base::CopyFile(file_path, file_path_tmp);

  LOG(ERROR) << "Bookmarks - Reading " << file_path_tmp;

  std::vector<ImportedBookmarkEntry> bookmarks;
  std::vector<importer::SearchEngineInfo> search_engines;

  bookmark_html_reader::ImportBookmarksFile(
      base::RepeatingCallback<bool(void)>(),
      base::RepeatingCallback<bool(const GURL&)>(),
      file_path_tmp,
      &bookmarks,
      &search_engines,
      nullptr);
  base::DeleteFile(file_path_tmp);

  auto *writer = new ProfileWriter(profile_);

  if (!bookmarks.empty()) {
    writer->AddBookmarks(bookmarks, u"Imported");
  }

  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = weak_java_ref_.get(env);
  if (obj.is_null())
    return;

  std::string message = "";
  if (bookmarks.size())
    message = "Imported " + std::to_string(bookmarks.size()) + " bookmarks";
  else
    message = "No bookmarks have been imported";
  Java_BookmarkBridge_bookmarksImported(env, obj, ConvertUTF8ToJavaString(env, message));
}

void BookmarkBridge::FileSelectionCanceled(void* params) {
  LOG(ERROR) << "Bookmarks - FileSelectionCanceled";
}

void BookmarkBridge::ImportBookmarks(JNIEnv* env, const base::android::JavaParamRef<jobject>& java_window) {
  DCHECK(IsLoaded());


  ui::WindowAndroid* window = ui::WindowAndroid::FromJavaWindowAndroid(java_window);
  CHECK(window);

  select_file_dialog_ = ui::SelectFileDialog::Create(
    this, std::make_unique<ChromeSelectFilePolicy>(nullptr));

  ui::SelectFileDialog::FileTypeInfo file_type_info;
  file_type_info.extensions = {{FILE_PATH_LITERAL("html"), FILE_PATH_LITERAL("htm")}};
  file_type_info.allowed_paths = ui::SelectFileDialog::FileTypeInfo::NATIVE_PATH;

  select_file_dialog_->SelectFile(
        ui::SelectFileDialog::SELECT_OPEN_FILE,
        std::u16string(),
        base::FilePath(),
        &file_type_info,
        0,
        base::FilePath::StringType(),
        window,
        NULL);
}


void BookmarkBridge::ExportBookmarks(JNIEnv* env) {
  DCHECK(IsLoaded());

  base::FilePath file_path;
  if (!base::android::GetDownloadsDirectory(&file_path)) {
    LOG(ERROR) << "Bookmarks - Getting Download Directory";
    return;
  }

  file_path = file_path.Append(FILE_PATH_LITERAL("bookmarks.html"));

  LOG(ERROR) << "Bookmarks - Output path is " << file_path;


  bookmark_html_writer::WriteBookmarks(profile_, file_path, NULL);

  ScopedJavaLocalRef<jobject> obj = weak_java_ref_.get(env);
  if (obj.is_null())
    return;

  Java_BookmarkBridge_bookmarksExported(env, obj, ConvertUTF8ToJavaString(env, file_path.MaybeAsASCII()));
}


void BookmarkBridge::Destroy(JNIEnv*, const JavaParamRef<jobject>&) {
  delete this;
}

static jlong JNI_BookmarkBridge_Init(JNIEnv* env,
                                     const JavaParamRef<jobject>& obj,
                                     const JavaParamRef<jobject>& j_profile) {
  BookmarkBridge* bridge = new BookmarkBridge(env, obj, j_profile);
  return reinterpret_cast<intptr_t>(bridge);
}

base::android::ScopedJavaLocalRef<jobject>
BookmarkBridge::GetBookmarkIdForWebContents(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    const JavaParamRef<jobject>& jweb_contents,
    jboolean only_editable) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  auto* web_contents = content::WebContents::FromJavaWebContents(jweb_contents);
  if (!web_contents)
    return nullptr;

  // TODO(https://crbug.com/1023759): We currently don't have a separate tab
  // model for incognito CCTs and incognito Tabs. So for incognito CCTs, the
  // incognito Tabs profile is passed to initialize BookmarkBridge, but here the
  // incognito CCT profile is used.
  // This DCHECK should be updated to compare profile objects instead of their
  // OTR state.
  DCHECK_EQ(profile_->IsOffTheRecord(),
            Profile::FromBrowserContext(web_contents->GetBrowserContext())
                ->IsOffTheRecord());
  GURL url = dom_distiller::url_utils::GetOriginalUrlFromDistillerUrl(
      web_contents->GetLastCommittedURL());

  // TODO(crbug.com/1150559): This is a hack to avoid a historical issue that
  // this function doesn't wait for any backend loaded.
  if (reading_list_manager_->IsLoaded()) {
    const auto* node = reading_list_manager_->Get(url);
    if (node)
      return JavaBookmarkIdCreateBookmarkId(env, node->id(),
                                            GetBookmarkType(node));
  }

  // Get all the nodes for |url| and sort them by date added.
  std::vector<const bookmarks::BookmarkNode*> nodes;
  bookmarks::ManagedBookmarkService* managed =
      ManagedBookmarkServiceFactory::GetForProfile(profile_);
  bookmarks::BookmarkModel* model =
      BookmarkModelFactory::GetForBrowserContext(profile_);

  model->GetNodesByURL(url, &nodes);
  std::sort(nodes.begin(), nodes.end(), &bookmarks::MoreRecentlyAdded);

  // Return the first node matching the search criteria.
  for (const auto* node : nodes) {
    if (only_editable && !managed->CanBeEditedByUser(node))
      continue;
    return JavaBookmarkIdCreateBookmarkId(env, node->id(),
                                          GetBookmarkType(node));
  }

  return nullptr;
}

jboolean BookmarkBridge::IsEditBookmarksEnabled(JNIEnv* env) {
  return IsEditBookmarksEnabled();
}

void BookmarkBridge::LoadEmptyPartnerBookmarkShimForTesting(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj) {
  if (partner_bookmarks_shim_->IsLoaded())
    return;
  partner_bookmarks_shim_->SetPartnerBookmarksRoot(
      PartnerBookmarksReader::CreatePartnerBookmarksRootForTesting());
  PartnerBookmarksShim::DisablePartnerBookmarksEditing();
  DCHECK(partner_bookmarks_shim_->IsLoaded());
}

// Loads a fake partner bookmarks shim for testing.
// This is used in BookmarkBridgeTest.java.
void BookmarkBridge::LoadFakePartnerBookmarkShimForTesting(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj) {
  if (partner_bookmarks_shim_->IsLoaded())
    return;
  std::unique_ptr<BookmarkNode> root_partner_node =
      PartnerBookmarksReader::CreatePartnerBookmarksRootForTesting();
  BookmarkNode* partner_bookmark_a =
      root_partner_node->Add(std::make_unique<BookmarkNode>(
          1, base::GUID::GenerateRandomV4(), GURL("http://www.a.com")));
  partner_bookmark_a->SetTitle(u"Partner Bookmark A");
  BookmarkNode* partner_bookmark_b =
      root_partner_node->Add(std::make_unique<BookmarkNode>(
          2, base::GUID::GenerateRandomV4(), GURL("http://www.b.com")));
  partner_bookmark_b->SetTitle(u"Partner Bookmark B");
  partner_bookmarks_shim_->SetPartnerBookmarksRoot(
      std::move(root_partner_node));
  PartnerBookmarksShim::DisablePartnerBookmarksEditing();
  DCHECK(partner_bookmarks_shim_->IsLoaded());
}

ScopedJavaLocalRef<jobject> BookmarkBridge::GetBookmarkByID(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    jlong id,
    jint type) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());
  const BookmarkNode* node = GetNodeByID(id, type);
  return node ? CreateJavaBookmark(node) : ScopedJavaLocalRef<jobject>();
}

bool BookmarkBridge::IsDoingExtensiveChanges(JNIEnv* env,
                                             const JavaParamRef<jobject>& obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  return bookmark_model_->IsDoingExtensiveChanges();
}

void BookmarkBridge::GetTopLevelFolderParentIDs(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    const JavaParamRef<jobject>& j_result_obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  Java_BookmarkBridge_addToBookmarkIdList(
      env, j_result_obj, bookmark_model_->root_node()->id(),
      GetBookmarkType(bookmark_model_->root_node()));
}

void BookmarkBridge::GetTopLevelFolderIDs(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    jboolean get_special,
    jboolean get_normal,
    const JavaParamRef<jobject>& j_result_obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());

  std::vector<const BookmarkNode*> top_level_folders;

  if (get_special) {
    if (managed_bookmark_service_->managed_node() &&
        !managed_bookmark_service_->managed_node()->children().empty()) {
      top_level_folders.push_back(managed_bookmark_service_->managed_node());
    }
    if (partner_bookmarks_shim_->HasPartnerBookmarks() &&
        IsReachable(partner_bookmarks_shim_->GetPartnerBookmarksRoot())) {
      top_level_folders.push_back(
          partner_bookmarks_shim_->GetPartnerBookmarksRoot());
    }
    if (reading_list_manager_->GetRoot()) {
      top_level_folders.push_back(reading_list_manager_->GetRoot());
    }
  }
  std::size_t special_count = top_level_folders.size();

  if (get_normal) {
    DCHECK_EQ(4u, bookmark_model_->root_node()->children().size());
    for (const auto& node : bookmark_model_->mobile_node()->children()) {
      if (node->is_folder())
        top_level_folders.push_back(node.get());
    }

    for (const auto& node : bookmark_model_->bookmark_bar_node()->children()) {
      if (node->is_folder())
        top_level_folders.push_back(node.get());
    }

    for (const auto& node : bookmark_model_->other_node()->children()) {
      if (node->is_folder())
        top_level_folders.push_back(node.get());
    }

    std::unique_ptr<icu::Collator> collator = GetICUCollator();
    std::stable_sort(top_level_folders.begin() + special_count,
                     top_level_folders.end(),
                     BookmarkTitleComparer(this, collator.get()));
  }

  for (std::vector<const BookmarkNode*>::const_iterator it =
           top_level_folders.begin();
       it != top_level_folders.end(); ++it) {
    Java_BookmarkBridge_addToBookmarkIdList(env, j_result_obj, (*it)->id(),
                                            GetBookmarkType(*it));
  }
}

base::android::ScopedJavaLocalRef<jobject> BookmarkBridge::GetReadingListFolder(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  const BookmarkNode* root_node = reading_list_manager_->GetRoot();
  ScopedJavaLocalRef<jobject> folder_id_obj = JavaBookmarkIdCreateBookmarkId(
      env, root_node->id(), GetBookmarkType(root_node));
  return folder_id_obj;
}

void BookmarkBridge::GetAllFoldersWithDepths(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    const JavaParamRef<jobject>& j_folders_obj,
    const JavaParamRef<jobject>& j_depths_obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());

  std::unique_ptr<icu::Collator> collator = GetICUCollator();

  // Vector to temporarily contain all child bookmarks at same level for sorting
  std::vector<const BookmarkNode*> bookmarks = {
      bookmark_model_->mobile_node(),
      bookmark_model_->bookmark_bar_node(),
      bookmark_model_->other_node(),
  };

  // Push all sorted top folders in stack and give them depth of 0.
  // Note the order to push folders to stack should be opposite to the order in
  // output.
  base::stack<std::pair<const BookmarkNode*, int>> stk;
  for (const auto* bookmark : base::Reversed(bookmarks))
    stk.emplace(bookmark, 0);

  while (!stk.empty()) {
    const BookmarkNode* node = stk.top().first;
    int depth = stk.top().second;
    stk.pop();
    Java_BookmarkBridge_addToBookmarkIdListWithDepth(
        env, j_folders_obj, node->id(), GetBookmarkType(node), j_depths_obj,
        depth);
    bookmarks.clear();
    for (const auto& child : node->children()) {
      if (child->is_folder() &&
          managed_bookmark_service_->CanBeEditedByUser(child.get())) {
        bookmarks.push_back(child.get());
      }
    }
    std::stable_sort(bookmarks.begin(), bookmarks.end(),
                     BookmarkTitleComparer(this, collator.get()));
    for (const auto* bookmark : base::Reversed(bookmarks))
      stk.emplace(bookmark, depth + 1);
  }
}

ScopedJavaLocalRef<jobject> BookmarkBridge::GetRootFolderId(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  const BookmarkNode* root_node = bookmark_model_->root_node();
  ScopedJavaLocalRef<jobject> folder_id_obj = JavaBookmarkIdCreateBookmarkId(
      env, root_node->id(), GetBookmarkType(root_node));
  return folder_id_obj;
}

ScopedJavaLocalRef<jobject> BookmarkBridge::GetMobileFolderId(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  const BookmarkNode* mobile_node = bookmark_model_->mobile_node();
  ScopedJavaLocalRef<jobject> folder_id_obj = JavaBookmarkIdCreateBookmarkId(
      env, mobile_node->id(), GetBookmarkType(mobile_node));
  return folder_id_obj;
}

ScopedJavaLocalRef<jobject> BookmarkBridge::GetOtherFolderId(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  const BookmarkNode* other_node = bookmark_model_->other_node();
  ScopedJavaLocalRef<jobject> folder_id_obj = JavaBookmarkIdCreateBookmarkId(
      env, other_node->id(), GetBookmarkType(other_node));
  return folder_id_obj;
}

ScopedJavaLocalRef<jobject> BookmarkBridge::GetDesktopFolderId(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  const BookmarkNode* desktop_node = bookmark_model_->bookmark_bar_node();
  ScopedJavaLocalRef<jobject> folder_id_obj = JavaBookmarkIdCreateBookmarkId(
      env, desktop_node->id(), GetBookmarkType(desktop_node));
  return folder_id_obj;
}

ScopedJavaLocalRef<jobject> BookmarkBridge::GetPartnerFolderId(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(partner_bookmarks_shim_->IsLoaded());

  const BookmarkNode* partner_node =
      partner_bookmarks_shim_->GetPartnerBookmarksRoot();
  ScopedJavaLocalRef<jobject> folder_id_obj = JavaBookmarkIdCreateBookmarkId(
      env, partner_node->id(), GetBookmarkType(partner_node));
  return folder_id_obj;
}

base::android::ScopedJavaLocalRef<jstring>
BookmarkBridge::GetBookmarkGuidByIdForTesting(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj,
    jlong id,
    jint type) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  const BookmarkNode* node = GetNodeByID(id, type);
  DCHECK(node) << "Bookmark with id " << id << " doesn't exist.";
  return base::android::ConvertUTF8ToJavaString(
      env, node->guid().AsLowercaseString());
}

jint BookmarkBridge::GetChildCount(JNIEnv* env,
                                   const JavaParamRef<jobject>& obj,
                                   jlong id,
                                   jint type) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());
  const BookmarkNode* node = GetNodeByID(id, type);
  return static_cast<jint>(node->children().size());
}

void BookmarkBridge::GetChildIDs(JNIEnv* env,
                                 const JavaParamRef<jobject>& obj,
                                 jlong id,
                                 jint type,
                                 const JavaParamRef<jobject>& j_result_obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());

  const BookmarkNode* parent = GetNodeByID(id, type);
  if (!parent->is_folder() || !IsReachable(parent))
    return;

  // Get the folder contents
  for (const auto& child : parent->children()) {
    if (IsFolderAvailable(child.get()) && IsReachable(child.get())) {
      Java_BookmarkBridge_addToBookmarkIdList(env, j_result_obj, child->id(),
                                              GetBookmarkType(child.get()));
    }
  }

  // Partner bookmark root node is under mobile node.
  if (parent == bookmark_model_->mobile_node() &&
      partner_bookmarks_shim_->HasPartnerBookmarks() &&
      IsReachable(partner_bookmarks_shim_->GetPartnerBookmarksRoot())) {
    Java_BookmarkBridge_addToBookmarkIdList(
        env, j_result_obj,
        partner_bookmarks_shim_->GetPartnerBookmarksRoot()->id(),
        BookmarkType::BOOKMARK_TYPE_PARTNER);
  }
}

ScopedJavaLocalRef<jobject> BookmarkBridge::GetChildAt(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    jlong id,
    jint type,
    jint index) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());

  const BookmarkNode* parent = GetNodeByID(id, type);
  DCHECK(parent);
  const BookmarkNode* child =
      parent->children()[static_cast<size_t>(index)].get();
  return JavaBookmarkIdCreateBookmarkId(env, child->id(),
                                        GetBookmarkType(child));
}

jint BookmarkBridge::GetTotalBookmarkCount(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj,
    jlong id,
    jint type) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());

  std::queue<const BookmarkNode*> nodes;
  const BookmarkNode* parent = GetNodeByID(id, type);
  DCHECK(parent->is_folder());

  int count = 0;
  nodes.push(parent);
  while (!nodes.empty()) {
    const BookmarkNode* node = nodes.front();
    nodes.pop();

    for (const auto& child : node->children()) {
      // Do not count deleted partner bookmarks or folders, which will have
      // empty titles. See PartnerBookmarkShim::RemoveBookmark().
      if (partner_bookmarks_shim_->IsPartnerBookmark(child.get()) &&
          partner_bookmarks_shim_->GetTitle(child.get()).empty())
        continue;
      if (child->is_folder())
        nodes.push(child.get());
      else
        ++count;
    }
    // If we are looking at the mobile bookmarks folder,
    // and we have partner bookmarks
    if (node == bookmark_model_->mobile_node() &&
        partner_bookmarks_shim_->HasPartnerBookmarks() &&
        IsReachable(partner_bookmarks_shim_->GetPartnerBookmarksRoot())) {
      nodes.push(partner_bookmarks_shim_->GetPartnerBookmarksRoot());
    }
  }

  return count;
}

void BookmarkBridge::SetBookmarkTitle(JNIEnv* env,
                                      const JavaParamRef<jobject>& obj,
                                      jlong id,
                                      jint type,
                                      const JavaParamRef<jstring>& j_title) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());

  const BookmarkNode* bookmark = GetNodeByID(id, type);
  const std::u16string title =
      base::android::ConvertJavaStringToUTF16(env, j_title);

  if (partner_bookmarks_shim_->IsPartnerBookmark(bookmark)) {
    partner_bookmarks_shim_->RenameBookmark(bookmark, title);
  } else {
    bookmark_model_->SetTitle(bookmark, title);
  }
}

void BookmarkBridge::SetBookmarkUrl(JNIEnv* env,
                                    const JavaParamRef<jobject>& obj,
                                    jlong id,
                                    jint type,
                                    const JavaParamRef<jobject>& url) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());
  bookmark_model_->SetURL(GetNodeByID(id, type),
                          *url::GURLAndroid::ToNativeGURL(env, url));
}

void BookmarkBridge::SetPowerBookmarkMeta(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    jlong id,
    jint type,
    const JavaParamRef<jbyteArray>& bytes) {
  const BookmarkNode* node = GetNodeByID(id, type);
  if (!node || bytes.is_null())
    return;

  std::unique_ptr<power_bookmarks::PowerBookmarkMeta> meta =
      std::make_unique<power_bookmarks::PowerBookmarkMeta>();
  std::vector<uint8_t> byte_vec;
  base::android::JavaByteArrayToByteVector(env, bytes, &byte_vec);
  if (meta->ParseFromArray(byte_vec.data(), byte_vec.size())) {
    power_bookmarks::SetNodePowerBookmarkMeta(bookmark_model_, node,
                                              std::move(meta));
  } else {
    DCHECK(false) << "Failed to parse bytes from java into PowerBookmarkMeta!";
  }
}

ScopedJavaLocalRef<jbyteArray> BookmarkBridge::GetPowerBookmarkMeta(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    jlong id,
    jint type) {
  const BookmarkNode* node = GetNodeByID(id, type);
  std::unique_ptr<power_bookmarks::PowerBookmarkMeta> meta =
      power_bookmarks::GetNodePowerBookmarkMeta(bookmark_model_, node);

  if (!meta)
    return ScopedJavaLocalRef<jbyteArray>(nullptr);

  int size = meta->ByteSize();
  std::string proto_bytes;
  meta->SerializeToString(&proto_bytes);
  std::vector<uint8_t> data;
  data.resize(size);
  meta->SerializeToArray(data.data(), size);

  return base::android::ToJavaByteArray(env, data.data(), size);
}

void BookmarkBridge::DeletePowerBookmarkMeta(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj,
    jlong id,
    jint type) {
  const BookmarkNode* node = GetNodeByID(id, type);

  if (!node)
    return;

  power_bookmarks::DeleteNodePowerBookmarkMeta(bookmark_model_, node);
}

bool BookmarkBridge::DoesBookmarkExist(JNIEnv* env,
                                       const JavaParamRef<jobject>& obj,
                                       jlong id,
                                       jint type) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());

  const BookmarkNode* node = GetNodeByID(id, type);

  if (!node)
    return false;

  if (type == BookmarkType::BOOKMARK_TYPE_NORMAL ||
      type == BookmarkType::BOOKMARK_TYPE_READING_LIST) {
    return true;
  } else {
    DCHECK(type == BookmarkType::BOOKMARK_TYPE_PARTNER);
    return partner_bookmarks_shim_->IsReachable(node);
  }
}

void BookmarkBridge::GetBookmarksForFolder(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    const JavaParamRef<jobject>& j_folder_id_obj,
    const JavaParamRef<jobject>& j_callback_obj,
    const JavaParamRef<jobject>& j_result_obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());

  long folder_id = JavaBookmarkIdGetId(env, j_folder_id_obj);
  int type = JavaBookmarkIdGetType(env, j_folder_id_obj);
  const BookmarkNode* folder = GetFolderWithFallback(folder_id, type);

  if (!folder->is_folder() || !IsReachable(folder))
    return;

  // Recreate the java bookmarkId object due to fallback.
  ScopedJavaLocalRef<jobject> folder_id_obj = JavaBookmarkIdCreateBookmarkId(
      env, folder->id(), GetBookmarkType(folder));

  // Get the folder contents.
  for (const auto& node : folder->children()) {
    if (IsFolderAvailable(node.get()))
      ExtractBookmarkNodeInformation(node.get(), j_result_obj);
  }

  if (folder == bookmark_model_->mobile_node() &&
      partner_bookmarks_shim_->HasPartnerBookmarks()) {
    ExtractBookmarkNodeInformation(
        partner_bookmarks_shim_->GetPartnerBookmarksRoot(), j_result_obj);
  }

  if (j_callback_obj) {
    Java_BookmarksCallback_onBookmarksAvailable(env, j_callback_obj,
                                                folder_id_obj, j_result_obj);
  }
}

jboolean BookmarkBridge::IsFolderVisible(JNIEnv* env,
                                         const JavaParamRef<jobject>& obj,
                                         jlong id,
                                         jint type) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  if (type == BookmarkType::BOOKMARK_TYPE_NORMAL) {
    const BookmarkNode* node = bookmarks::GetBookmarkNodeByID(
        bookmark_model_, static_cast<int64_t>(id));
    return node->IsVisible();
  }
  DCHECK_EQ(BookmarkType::BOOKMARK_TYPE_PARTNER, type);
  const BookmarkNode* node =
      partner_bookmarks_shim_->GetNodeByID(static_cast<long>(id));
  return partner_bookmarks_shim_->IsReachable(node);
}

void BookmarkBridge::GetCurrentFolderHierarchy(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    const JavaParamRef<jobject>& j_folder_id_obj,
    const JavaParamRef<jobject>& j_callback_obj,
    const JavaParamRef<jobject>& j_result_obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());

  long folder_id = JavaBookmarkIdGetId(env, j_folder_id_obj);
  int type = JavaBookmarkIdGetType(env, j_folder_id_obj);
  const BookmarkNode* folder = GetFolderWithFallback(folder_id, type);

  if (!folder->is_folder() || !IsReachable(folder))
    return;

  // Recreate the java bookmarkId object due to fallback.
  ScopedJavaLocalRef<jobject> folder_id_obj = JavaBookmarkIdCreateBookmarkId(
      env, folder->id(), GetBookmarkType(folder));

  // Get the folder hierarchy.
  const BookmarkNode* node = folder;
  while (node) {
    ExtractBookmarkNodeInformation(node, j_result_obj);
    node = GetParentNode(node);
  }

  Java_BookmarksCallback_onBookmarksFolderHierarchyAvailable(
      env, j_callback_obj, folder_id_obj, j_result_obj);
}

void BookmarkBridge::SearchBookmarks(JNIEnv* env,
                                     const JavaParamRef<jobject>& obj,
                                     const JavaParamRef<jobject>& j_list,
                                     const JavaParamRef<jstring>& j_query,
                                     const JavaParamRef<jobjectArray>& j_tags,
                                     jint type,
                                     jint max_results) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(bookmark_model_->loaded());

  std::vector<const BookmarkNode*> results;

  power_bookmarks::PowerBookmarkQueryFields query;
  query.word_phrase_query = std::make_unique<std::u16string>(
      base::android::ConvertJavaStringToUTF16(env, j_query));
  if (query.word_phrase_query->empty())
    query.word_phrase_query.reset();

  if (base::FeatureList::IsEnabled(commerce::kShoppingList)) {
    if (!j_tags.is_null()) {
      base::android::AppendJavaStringArrayToStringVector(env, j_tags,
                                                         &query.tags);
    }

    if (type >= 0)
      query.type = static_cast<power_bookmarks::PowerBookmarkType>(type);

    power_bookmarks::GetBookmarksMatchingProperties(bookmark_model_, query,
                                                    max_results, &results);
  } else {
    GetBookmarksMatchingProperties(bookmark_model_, query, max_results,
                                   &results);
  }

  reading_list_manager_->GetMatchingNodes(query, max_results, &results);
  if (partner_bookmarks_shim_->HasPartnerBookmarks() &&
      IsReachable(partner_bookmarks_shim_->GetPartnerBookmarksRoot())) {
    partner_bookmarks_shim_->GetPartnerBookmarksMatchingProperties(
        query, max_results, &results);
  }
  DCHECK((int)results.size() <= max_results || max_results == -1);
  for (const bookmarks::BookmarkNode* match : results) {
    if (!IsReachable(match))
      continue;
    Java_BookmarkBridge_addToBookmarkIdList(env, j_list, match->id(),
                                            GetBookmarkType(match));
  }
}

void BookmarkBridge::GetBookmarksOfType(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    const base::android::JavaParamRef<jobject>& j_list,
    jint type) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  std::vector<const BookmarkNode*> results;
  power_bookmarks::PowerBookmarkQueryFields query;
  query.type = static_cast<power_bookmarks::PowerBookmarkType>(type);
  power_bookmarks::GetBookmarksMatchingProperties(bookmark_model_, query, -1,
                                                  &results);

  for (const bookmarks::BookmarkNode* match : results) {
    Java_BookmarkBridge_addToBookmarkIdList(env, j_list, match->id(),
                                            GetBookmarkType(match));
  }
}

ScopedJavaLocalRef<jobject> BookmarkBridge::AddFolder(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    const JavaParamRef<jobject>& j_parent_id_obj,
    jint index,
    const JavaParamRef<jstring>& j_title) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());

  long bookmark_id = JavaBookmarkIdGetId(env, j_parent_id_obj);
  int type = JavaBookmarkIdGetType(env, j_parent_id_obj);
  const BookmarkNode* parent = GetNodeByID(bookmark_id, type);

  const BookmarkNode* new_node = bookmark_model_->AddFolder(
      parent, static_cast<size_t>(index),
      base::android::ConvertJavaStringToUTF16(env, j_title));
  DCHECK(new_node);
  ScopedJavaLocalRef<jobject> new_java_obj = JavaBookmarkIdCreateBookmarkId(
      env, new_node->id(), GetBookmarkType(new_node));
  return new_java_obj;
}

void BookmarkBridge::DeleteBookmark(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    const JavaParamRef<jobject>& j_bookmark_id_obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());

  long bookmark_id = JavaBookmarkIdGetId(env, j_bookmark_id_obj);
  int type = JavaBookmarkIdGetType(env, j_bookmark_id_obj);
  const BookmarkNode* node = GetNodeByID(bookmark_id, type);

  // TODO(twellington): Switch back to a DCHECK after debugging
  // why this is called with an uneditable node.
  // See https://crbug.com/981172.
  if (!IsEditable(node)) {
    LOG(ERROR) << "Deleting non editable bookmark, type:" << type;
    NOTREACHED();
    return;
  }

  if (partner_bookmarks_shim_->IsPartnerBookmark(node)) {
    partner_bookmarks_shim_->RemoveBookmark(node);
  } else if (type == BookmarkType::BOOKMARK_TYPE_READING_LIST) {
    // Inside the Delete method, node will be destroyed and node->url will be
    // also destroyed. This causes heap-use-after-free at
    // ReadingListModelImpl::RemoveEntryByURLImpl. To avoid the
    // heap-use-after-free, make a copy of node->url() and use it.
    GURL url(node->url());
    reading_list_manager_->Delete(url);
  } else {
    bookmark_model_->Remove(node);
  }
}

void BookmarkBridge::RemoveAllUserBookmarks(JNIEnv* env,
                                            const JavaParamRef<jobject>& obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());
  bookmark_model_->RemoveAllUserBookmarks();
}

void BookmarkBridge::MoveBookmark(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    const JavaParamRef<jobject>& j_bookmark_id_obj,
    const JavaParamRef<jobject>& j_parent_id_obj,
    jint index) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());

  long bookmark_id = JavaBookmarkIdGetId(env, j_bookmark_id_obj);
  int type = JavaBookmarkIdGetType(env, j_bookmark_id_obj);
  const BookmarkNode* node = GetNodeByID(bookmark_id, type);
  DCHECK(IsEditable(node));
  bookmark_id = JavaBookmarkIdGetId(env, j_parent_id_obj);
  type = JavaBookmarkIdGetType(env, j_parent_id_obj);
  const BookmarkNode* new_parent_node = GetNodeByID(bookmark_id, type);
  // Bookmark should not be moved to its own parent folder
  if (node->parent() != new_parent_node) {
    bookmark_model_->Move(node, new_parent_node, static_cast<size_t>(index));
  }
}

ScopedJavaLocalRef<jobject> BookmarkBridge::AddBookmark(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    const base::android::JavaParamRef<jobject>& j_web_contents,
    const JavaParamRef<jobject>& j_parent_id_obj,
    jint index,
    const JavaParamRef<jstring>& j_title,
    const JavaParamRef<jobject>& j_url) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());
  long bookmark_id = JavaBookmarkIdGetId(env, j_parent_id_obj);
  int type = JavaBookmarkIdGetType(env, j_parent_id_obj);
  const BookmarkNode* parent = GetNodeByID(bookmark_id, type);

  const BookmarkNode* new_node = bookmark_model_->AddNewURL(
      parent, static_cast<size_t>(index),
      base::android::ConvertJavaStringToUTF16(env, j_title),
      *url::GURLAndroid::ToNativeGURL(env, j_url));

  auto* web_contents =
      content::WebContents::FromJavaWebContents(j_web_contents);

  // TODO(crbug.com/1247352): Move to platform-independent location.
  if (web_contents && base::FeatureList::IsEnabled(commerce::kShoppingList)) {
    shopping_list::ShoppingDataProvider* data_provider =
        shopping_list::ShoppingDataProvider::FromWebContents(web_contents);
    std::unique_ptr<PowerBookmarkMeta> meta =
        data_provider ? data_provider->GetCurrentMetadata() : nullptr;
    if (meta) {
      power_bookmarks::SetNodePowerBookmarkMeta(bookmark_model_, new_node,
                                                std::move(meta));
    }
  }
  DCHECK(new_node);
  ScopedJavaLocalRef<jobject> new_java_obj = JavaBookmarkIdCreateBookmarkId(
      env, new_node->id(), GetBookmarkType(new_node));
  return new_java_obj;
}

ScopedJavaLocalRef<jobject> BookmarkBridge::AddToReadingList(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    const JavaParamRef<jstring>& j_title,
    const JavaParamRef<jobject>& j_url) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());

  const BookmarkNode* node = reading_list_manager_->Add(
      *url::GURLAndroid::ToNativeGURL(env, j_url),
      base::android::ConvertJavaStringToUTF8(env, j_title));
  return node ? JavaBookmarkIdCreateBookmarkId(env, node->id(),
                                               GetBookmarkType(node))
              : ScopedJavaLocalRef<jobject>();
}

ScopedJavaLocalRef<jobject> BookmarkBridge::GetReadingListItem(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    const JavaParamRef<jobject>& j_url) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());

  const BookmarkNode* node =
      reading_list_manager_->Get(*url::GURLAndroid::ToNativeGURL(env, j_url));
  return node ? CreateJavaBookmark(node) : ScopedJavaLocalRef<jobject>();
}

void BookmarkBridge::SetReadStatus(JNIEnv* env,
                                   const JavaParamRef<jobject>& obj,
                                   const JavaParamRef<jobject>& j_url,
                                   jboolean j_read) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());

  reading_list_manager_->SetReadStatus(
      *url::GURLAndroid::ToNativeGURL(env, j_url), j_read);
}

void BookmarkBridge::Undo(JNIEnv* env, const JavaParamRef<jobject>& obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());
  BookmarkUndoService* undo_service =
      BookmarkUndoServiceFactory::GetForProfile(profile_);
  UndoManager* undo_manager = undo_service->undo_manager();
  undo_manager->Undo();
}

void BookmarkBridge::StartGroupingUndos(JNIEnv* env,
                                        const JavaParamRef<jobject>& obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());
  DCHECK(!grouped_bookmark_actions_.get());  // shouldn't have started already
  grouped_bookmark_actions_ =
      std::make_unique<bookmarks::ScopedGroupBookmarkActions>(bookmark_model_);
}

void BookmarkBridge::EndGroupingUndos(JNIEnv* env,
                                      const JavaParamRef<jobject>& obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(IsLoaded());
  DCHECK(grouped_bookmark_actions_.get());  // should only call after start
  grouped_bookmark_actions_.reset();
}

bool BookmarkBridge::IsBookmarked(JNIEnv* env,
                                  const JavaParamRef<jobject>& gurl) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  return bookmark_model_->IsBookmarked(
      *url::GURLAndroid::ToNativeGURL(env, gurl));
}

std::u16string BookmarkBridge::GetTitle(const BookmarkNode* node) const {
  if (partner_bookmarks_shim_->IsPartnerBookmark(node))
    return partner_bookmarks_shim_->GetTitle(node);

  return node->GetTitle();
}

ScopedJavaLocalRef<jobject> BookmarkBridge::CreateJavaBookmark(
    const BookmarkNode* node) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  JNIEnv* env = AttachCurrentThread();

  const BookmarkNode* parent = GetParentNode(node);
  int64_t parent_id = parent ? parent->id() : -1;

  GURL url;
  if (node->is_url())
    url = node->url();

  int type = GetBookmarkType(node);
  bool read = false;
  if (reading_list_manager_->IsReadingListBookmark(node)) {
    read = reading_list_manager_->GetReadStatus(node);
  }

  return Java_BookmarkBridge_createBookmarkItem(
      env, node->id(), type, ConvertUTF16ToJavaString(env, GetTitle(node)),
      url::GURLAndroid::FromNativeGURL(env, url), node->is_folder(), parent_id,
      GetBookmarkType(parent), IsEditable(node), IsManaged(node),
      node->date_added().ToJavaTime(), read);
}

void BookmarkBridge::ExtractBookmarkNodeInformation(
    const BookmarkNode* node,
    const JavaRef<jobject>& j_result_obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  JNIEnv* env = AttachCurrentThread();
  if (!IsReachable(node))
    return;
  Java_BookmarkBridge_addToList(env, j_result_obj, CreateJavaBookmark(node));
}

const BookmarkNode* BookmarkBridge::GetNodeByID(long node_id, int type) {
  const BookmarkNode* node;
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  if (type == BookmarkType::BOOKMARK_TYPE_PARTNER) {
    node = partner_bookmarks_shim_->GetNodeByID(static_cast<int64_t>(node_id));
  } else if (type == BookmarkType::BOOKMARK_TYPE_READING_LIST) {
    node = reading_list_manager_->GetNodeByID(static_cast<int64_t>(node_id));
  } else {
    node = bookmarks::GetBookmarkNodeByID(bookmark_model_,
                                          static_cast<int64_t>(node_id));
  }
  return node;
}

const BookmarkNode* BookmarkBridge::GetFolderWithFallback(long folder_id,
                                                          int type) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  const BookmarkNode* folder = GetNodeByID(folder_id, type);
  if (!folder || folder->type() == BookmarkNode::URL ||
      !IsFolderAvailable(folder)) {
    if (!managed_bookmark_service_->managed_node()->children().empty())
      folder = managed_bookmark_service_->managed_node();
    else
      folder = bookmark_model_->mobile_node();
  }
  return folder;
}

bool BookmarkBridge::IsEditBookmarksEnabled() const {
  return profile_->GetPrefs()->GetBoolean(
      bookmarks::prefs::kEditBookmarksEnabled);
}

void BookmarkBridge::EditBookmarksEnabledChanged() {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = weak_java_ref_.get(env);
  if (obj.is_null())
    return;
  Java_BookmarkBridge_editBookmarksEnabledChanged(env, obj);
}

bool BookmarkBridge::IsEditable(const BookmarkNode* node) const {
  if (!node || (node->type() != BookmarkNode::FOLDER &&
                node->type() != BookmarkNode::URL)) {
    return false;
  }
  if (!IsEditBookmarksEnabled() || bookmark_model_->is_permanent_node(node))
    return false;
  if (partner_bookmarks_shim_->IsPartnerBookmark(node))
    return partner_bookmarks_shim_->IsEditable(node);
  if (reading_list_manager_->IsReadingListBookmark(node))
    return reading_list_manager_->GetRoot() != node;

  return managed_bookmark_service_->CanBeEditedByUser(node);
}

bool BookmarkBridge::IsManaged(const BookmarkNode* node) const {
  return bookmarks::IsDescendantOf(node,
                                   managed_bookmark_service_->managed_node());
}

const BookmarkNode* BookmarkBridge::GetParentNode(const BookmarkNode* node) {
  DCHECK(IsLoaded());
  if (node == partner_bookmarks_shim_->GetPartnerBookmarksRoot())
    return bookmark_model_->mobile_node();

  if (node == reading_list_manager_->GetRoot())
    return bookmark_model_->root_node();

  return node->parent();
}

int BookmarkBridge::GetBookmarkType(const BookmarkNode* node) {
  // TODO(crbug.com/1150559) return the wrong type when the backend is not
  // loaded?
  if (partner_bookmarks_shim_->IsLoaded() &&
      partner_bookmarks_shim_->IsPartnerBookmark(node))
    return BookmarkType::BOOKMARK_TYPE_PARTNER;

  if (reading_list_manager_->IsLoaded() &&
      reading_list_manager_->IsReadingListBookmark(node))
    return BookmarkType::BOOKMARK_TYPE_READING_LIST;

  return BookmarkType::BOOKMARK_TYPE_NORMAL;
}

bool BookmarkBridge::IsReachable(const BookmarkNode* node) const {
  if (!partner_bookmarks_shim_->IsPartnerBookmark(node))
    return true;
  return partner_bookmarks_shim_->IsReachable(node);
}

bool BookmarkBridge::IsLoaded() const {
  return (bookmark_model_->loaded() && partner_bookmarks_shim_->IsLoaded() &&
          reading_list_manager_->IsLoaded());
}

bool BookmarkBridge::IsFolderAvailable(const BookmarkNode* folder) const {
  // The managed bookmarks folder is not shown if there are no bookmarks
  // configured via policy.
  if (folder == managed_bookmark_service_->managed_node() &&
      folder->children().empty())
    return false;

  auto* identity_manager =
      IdentityManagerFactory::GetForProfile(profile_->GetOriginalProfile());
  return (folder->type() != BookmarkNode::BOOKMARK_BAR &&
          folder->type() != BookmarkNode::OTHER_NODE) ||
         (identity_manager &&
          identity_manager->HasPrimaryAccount(signin::ConsentLevel::kSync));
}

void BookmarkBridge::NotifyIfDoneLoading() {
  if (!IsLoaded())
    return;
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = weak_java_ref_.get(env);
  if (obj.is_null())
    return;
  Java_BookmarkBridge_bookmarkModelLoaded(env, obj);
}

// ------------- Observer-related methods ------------- //

void BookmarkBridge::BookmarkModelChanged() {
  if (!IsLoaded())
    return;

  // Called when there are changes to the bookmark model. It is most
  // likely changes to the partner bookmarks.
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = weak_java_ref_.get(env);
  if (obj.is_null())
    return;
  Java_BookmarkBridge_bookmarkModelChanged(env, obj);
}

void BookmarkBridge::BookmarkModelLoaded(BookmarkModel* model,
                                         bool ids_reassigned) {
  NotifyIfDoneLoading();
}

void BookmarkBridge::BookmarkModelBeingDeleted(BookmarkModel* model) {
  if (!IsLoaded())
    return;

  DestroyJavaObject();
}

void BookmarkBridge::BookmarkNodeMoved(BookmarkModel* model,
                                       const BookmarkNode* old_parent,
                                       size_t old_index,
                                       const BookmarkNode* new_parent,
                                       size_t new_index) {
  if (!IsLoaded())
    return;

  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = weak_java_ref_.get(env);
  if (obj.is_null())
    return;
  Java_BookmarkBridge_bookmarkNodeMoved(
      env, obj, CreateJavaBookmark(old_parent), static_cast<int>(old_index),
      CreateJavaBookmark(new_parent), static_cast<int>(new_index));
}

void BookmarkBridge::BookmarkNodeAdded(BookmarkModel* model,
                                       const BookmarkNode* parent,
                                       size_t index) {
  if (!IsLoaded())
    return;

  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = weak_java_ref_.get(env);
  if (obj.is_null())
    return;
  Java_BookmarkBridge_bookmarkNodeAdded(env, obj, CreateJavaBookmark(parent),
                                        static_cast<int>(index));
}

void BookmarkBridge::BookmarkNodeRemoved(BookmarkModel* model,
                                         const BookmarkNode* parent,
                                         size_t old_index,
                                         const BookmarkNode* node,
                                         const std::set<GURL>& removed_urls) {
  if (!IsLoaded())
    return;

  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = weak_java_ref_.get(env);
  if (obj.is_null())
    return;
  Java_BookmarkBridge_bookmarkNodeRemoved(env, obj, CreateJavaBookmark(parent),
                                          static_cast<int>(old_index),
                                          CreateJavaBookmark(node));
}

void BookmarkBridge::BookmarkAllUserNodesRemoved(
    BookmarkModel* model,
    const std::set<GURL>& removed_urls) {
  if (!IsLoaded())
    return;

  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = weak_java_ref_.get(env);
  if (obj.is_null())
    return;
  Java_BookmarkBridge_bookmarkAllUserNodesRemoved(env, obj);
}

void BookmarkBridge::BookmarkNodeChanged(BookmarkModel* model,
                                         const BookmarkNode* node) {
  if (!IsLoaded())
    return;

  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = weak_java_ref_.get(env);
  if (obj.is_null())
    return;
  Java_BookmarkBridge_bookmarkNodeChanged(env, obj, CreateJavaBookmark(node));
}

void BookmarkBridge::BookmarkNodeChildrenReordered(BookmarkModel* model,
                                                   const BookmarkNode* node) {
  if (!IsLoaded())
    return;

  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = weak_java_ref_.get(env);
  if (obj.is_null())
    return;
  Java_BookmarkBridge_bookmarkNodeChildrenReordered(env, obj,
                                                    CreateJavaBookmark(node));
}

void BookmarkBridge::ExtensiveBookmarkChangesBeginning(BookmarkModel* model) {
  if (!IsLoaded())
    return;

  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = weak_java_ref_.get(env);
  if (obj.is_null())
    return;
  Java_BookmarkBridge_extensiveBookmarkChangesBeginning(env, obj);
}

void BookmarkBridge::ExtensiveBookmarkChangesEnded(BookmarkModel* model) {
  if (!IsLoaded())
    return;

  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = weak_java_ref_.get(env);
  if (obj.is_null())
    return;
  Java_BookmarkBridge_extensiveBookmarkChangesEnded(env, obj);
}

void BookmarkBridge::PartnerShimChanged(PartnerBookmarksShim* shim) {
  BookmarkModelChanged();
}

void BookmarkBridge::PartnerShimLoaded(PartnerBookmarksShim* shim) {
  NotifyIfDoneLoading();
}

void BookmarkBridge::ShimBeingDeleted(PartnerBookmarksShim* shim) {
  partner_bookmarks_shim_ = nullptr;
}

void BookmarkBridge::ReadingListLoaded() {
  NotifyIfDoneLoading();
}

void BookmarkBridge::ReadingListChanged() {
  BookmarkModelChanged();
}

void BookmarkBridge::ReorderChildren(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    const JavaParamRef<jobject>& j_bookmark_id_obj,
    jlongArray arr) {
  DCHECK(IsLoaded());
  // get the BookmarkNode* for the "parent" bookmark parameter
  const long bookmark_id = JavaBookmarkIdGetId(env, j_bookmark_id_obj);
  const int bookmark_type = JavaBookmarkIdGetType(env, j_bookmark_id_obj);

  const BookmarkNode* bookmark_node = GetNodeByID(bookmark_id, bookmark_type);

  // populate a vector
  std::vector<const BookmarkNode*> ordered_nodes;
  jsize arraySize = env->GetArrayLength(arr);
  jlong* elements = env->GetLongArrayElements(arr, 0);

  // iterate through array, adding the BookmarkNode*s of the objects
  for (int i = 0; i < arraySize; ++i) {
    ordered_nodes.push_back(GetNodeByID(elements[i], 0));
  }

  bookmark_model_->ReorderChildren(bookmark_node, ordered_nodes);
}

// Should destroy the bookmark bridge, if OTR profile is destroyed not to delete
// related resources twice.
void BookmarkBridge::OnProfileWillBeDestroyed(Profile* profile) {
  weak_ptr_factory_.InvalidateWeakPtrs();
  DestroyJavaObject();
}

void BookmarkBridge::GetUpdatedProductPrices(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj,
    const JavaParamRef<jobjectArray>& gurls,
    const JavaParamRef<jobject>& callback) {
  std::vector<GURL> urls;
  for (int i = 0; i < env->GetArrayLength(gurls.obj()); i++) {
    urls.push_back(*url::GURLAndroid::ToNativeGURL(
        env, ScopedJavaLocalRef<jobject>(
                 env, env->GetObjectArrayElement(gurls.obj(), i))));
  }

  CHECK(opt_guide_);

  opt_guide_->CanApplyOptimizationOnDemand(
      urls, {optimization_guide::proto::OptimizationType::PRICE_TRACKING},
      optimization_guide::proto::RequestContext::CONTEXT_BOOKMARKS,
      base::BindRepeating(&BookmarkBridge::OnProductPriceUpdated,
                          weak_ptr_factory_.GetWeakPtr(),
                          ScopedJavaGlobalRef<jobject>(callback)));
}

void BookmarkBridge::OnProductPriceUpdated(
    ScopedJavaGlobalRef<jobject> callback,
    const GURL& url,
    const base::flat_map<
        optimization_guide::proto::OptimizationType,
        optimization_guide::OptimizationGuideDecisionWithMetadata>& decisions) {
  JNIEnv* env = AttachCurrentThread();

  if (!decisions.contains(
          optimization_guide::proto::OptimizationType::PRICE_TRACKING)) {
    return;
  }

  auto iter = decisions.find(
      optimization_guide::proto::OptimizationType::PRICE_TRACKING);

  if (iter == decisions.cend())
    return;

  optimization_guide::OptimizationGuideDecisionWithMetadata decision =
      iter->second;

  // Only fire the callback for price tracking info if successful.
  if (decision.decision !=
      optimization_guide::OptimizationGuideDecision::kTrue) {
    return;
  }

  if (decision.metadata.any_metadata().has_value()) {
    absl::optional<commerce::PriceTrackingData> parsed_any =
        optimization_guide::ParsedAnyMetadata<commerce::PriceTrackingData>(
            decision.metadata.any_metadata().value());

    if (!parsed_any.has_value())
      return;

    commerce::PriceTrackingData price_tracking_data = parsed_any.value();

    bool has_price = price_tracking_data.IsInitialized() &&
                     price_tracking_data.has_buyable_product() &&
                     price_tracking_data.buyable_product().has_current_price();

    if (has_price) {
      commerce::ProductPrice price =
          price_tracking_data.buyable_product().current_price();

      int size = price.ByteSize();
      std::vector<uint8_t> data;
      data.resize(size);
      price.SerializeToArray(data.data(), size);

      Java_BookmarkBridge_onProductPriceUpdated(
          env, weak_java_ref_.get(env),
          url::GURLAndroid::FromNativeGURL(env, url),
          base::android::ToJavaByteArray(env, data.data(), size), callback);
    }
  }
}

void BookmarkBridge::DestroyJavaObject() {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = weak_java_ref_.get(env);
  if (obj.is_null())
    return;
  Java_BookmarkBridge_destroyFromNative(env, obj);
}
