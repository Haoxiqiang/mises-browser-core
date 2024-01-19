/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mises/browser/mises_local_state_prefs.h"

#include "base/values.h"
#include "mises/components/constants/pref_names.h"
#include "mises/components/decentralized_dns/core/utils.h"
#include "mises/components/brave_wallet/browser/brave_wallet_prefs.h"
#include "build/build_config.h"
#include "chrome/common/pref_names.h"
#include "components/metrics/metrics_pref_names.h"
#include "components/prefs/pref_registry_simple.h"
#include "third_party/widevine/cdm/buildflags.h"

#if BUILDFLAG(ENABLE_WIDEVINE)
#include "mises/browser/widevine/widevine_utils.h"
#endif

namespace mises {

void RegisterLocalStatePrefsForMigration(PrefRegistrySimple* registry) {
#if BUILDFLAG(ENABLE_WIDEVINE)
  RegisterWidevineLocalstatePrefsForMigration(registry);
#endif
   brave_wallet::RegisterLocalStatePrefsForMigration(registry);
}
void RegisterLocalStatePrefs(PrefRegistrySimple* registry) {
  

  decentralized_dns::RegisterLocalStatePrefs(registry);

  RegisterLocalStatePrefsForMigration(registry);
  
  brave_wallet::RegisterLocalStatePrefs(registry);

#if BUILDFLAG(ENABLE_WIDEVINE)
  RegisterWidevineLocalstatePrefs(registry);
#endif

}

}  // namespace brave
