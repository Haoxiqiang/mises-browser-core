#include "build/build_config.h"

#if BUILDFLAG(IS_ANDROID)

#undef BUILDFLAG_INTERNAL_IS_ANDROID
#define BUILDFLAG_INTERNAL_IS_ANDROID() (0)

#include "src/chrome/browser/share/share_features.cc"
#undef BUILDFLAG_INTERNAL_IS_ANDROID
#define BUILDFLAG_INTERNAL_IS_ANDROID() (1)


namespace share {

BASE_FEATURE(kCrowLaunchTab,
             "ShareCrowLaunchTab",
             base::FEATURE_DISABLED_BY_DEFAULT);

void dummy_run() {
  LOG(INFO) << kCrowLaunchTab.name;
}

}
#else

#include "src/chrome/browser/share/share_features.cc"


#endif




