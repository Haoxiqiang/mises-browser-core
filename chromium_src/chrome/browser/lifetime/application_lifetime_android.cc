#include "base/no_destructor.h"
#include "src/chrome/browser/lifetime/application_lifetime_android.cc"
namespace chrome {

namespace {

base::RepeatingCallbackList<void(bool)>& GetClosingAllBrowsersCallbackList() {
  static base::NoDestructor<base::RepeatingCallbackList<void(bool)>>
      callback_list;
  return *callback_list;
}

}

void OnAppExiting() {
}
void CloseAllBrowsers() {
}
void OnClosingAllBrowsers(bool) {
  
}

base::CallbackListSubscription AddClosingAllBrowsersCallback(
    base::RepeatingCallback<void(bool)> closing_all_browsers_callback) {
  return GetClosingAllBrowsersCallbackList().Add(
      std::move(closing_all_browsers_callback));
}

}