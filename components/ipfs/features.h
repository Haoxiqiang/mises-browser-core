/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_IPFS_FEATURES_H_
#define BRAVE_COMPONENTS_IPFS_FEATURES_H_

#include "base/feature_list.h"

namespace ipfs {
namespace features {

#define BASE_DECLARE_FEATURE(kFeature) \
  extern CONSTINIT const base::Feature kFeature
  
BASE_DECLARE_FEATURE(kIpfsFeature);

#undef BASE_DECLARE_FEATURE
}  // namespace features
}  // namespace ipfs

#endif  // BRAVE_COMPONENTS_IPFS_FEATURES_H_
