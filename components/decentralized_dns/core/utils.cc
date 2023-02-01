/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mises/components/decentralized_dns/core/utils.h"

#include "base/strings/string_util.h"
#include "mises/components/decentralized_dns/core/constants.h"
#include "mises/components/decentralized_dns/core/pref_names.h"
#include "mises/net/decentralized_dns/constants.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "url/gurl.h"

namespace decentralized_dns {

void RegisterLocalStatePrefs(PrefRegistrySimple* registry) {
  registry->RegisterIntegerPref(kUnstoppableDomainsResolveMethod,
                                static_cast<int>(ResolveMethodTypes::ETHEREUM));
  registry->RegisterIntegerPref(kENSResolveMethod,
                                static_cast<int>(ResolveMethodTypes::ETHEREUM));
  registry->RegisterIntegerPref(
      kEnsOffchainResolveMethod,
      static_cast<int>(EnsOffchainResolveMethod::kAsk));
}

void MigrateObsoleteLocalStatePrefs(PrefService* local_state) {
}

bool IsUnstoppableDomainsTLD(const GURL& url) {
  for (auto* domain : kUnstoppableDomains) {
    if (base::EndsWith(url.host_piece(), domain))
      return true;
  }
  return false;
}

bool IsUnstoppableDomainsResolveMethodAsk(PrefService* local_state) {
  if (!local_state) {
    return false;  // Treat it as disabled.
  }

  return local_state->GetInteger(kUnstoppableDomainsResolveMethod) ==
         static_cast<int>(ResolveMethodTypes::ASK);
}

bool IsUnstoppableDomainsResolveMethodEthereum(PrefService* local_state) {
  if (!local_state) {
    return false;  // Treat it as disabled.
  }

  return local_state->GetInteger(kUnstoppableDomainsResolveMethod) ==
         static_cast<int>(ResolveMethodTypes::ETHEREUM);
}

bool IsBitTLD(const GURL& url) {
  return base::EndsWith(url.host_piece(), kBitDomain);
}

bool IsENSTLD(const GURL& url) {
  return base::EndsWith(url.host_piece(), kEthDomain);
}

bool IsENSResolveMethodAsk(PrefService* local_state) {
  if (!local_state) {
    return false;  // Treat it as disabled.
  }

  return local_state->GetInteger(kENSResolveMethod) ==
         static_cast<int>(ResolveMethodTypes::ASK);
}

bool IsENSResolveMethodEthereum(PrefService* local_state) {
  if (!local_state) {
    return false;  // Treat it as disabled.
  }

  return local_state->GetInteger(kENSResolveMethod) ==
         static_cast<int>(ResolveMethodTypes::ETHEREUM);
}

void SetEnsOffchainResolveMethod(PrefService* local_state,
                                 EnsOffchainResolveMethod method) {
  local_state->SetInteger(kEnsOffchainResolveMethod, static_cast<int>(method));
}

EnsOffchainResolveMethod GetEnsOffchainResolveMethod(PrefService* local_state) {
  return static_cast<EnsOffchainResolveMethod>(
      local_state->GetInteger(kEnsOffchainResolveMethod));
}

}  // namespace decentralized_dns
