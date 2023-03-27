
#include "build/build_config.h"

#if BUILDFLAG(IS_ANDROID)

#undef BUILDFLAG_INTERNAL_IS_ANDROID
#define BUILDFLAG_INTERNAL_IS_ANDROID() (0)

#undef ANDROID

#include "src/chrome/browser/ui/webui/side_panel/read_anything/read_anything_prefs.cc"
#undef BUILDFLAG_INTERNAL_IS_ANDROID
#define BUILDFLAG_INTERNAL_IS_ANDROID() (1)

#define ANDROID 1

#else

#include "src/chrome/browser/ui/webui/side_panel/read_anything/read_anything_prefs.cc"


#endif


