// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

export const supportedENSExtensions = ['.eth']
export const supportedSNSExtensions = ['.sol']
// Should match `kUDPattern` array from json_rpc_service.cc.
export const supportedUDExtensions = [
  '.crypto',
  '.x',
  '.nft',
  '.dao',
  '.wallet',
  '.blockchain',
  '.bitcoin',
  '.zil',
  '.altimist',
  '.anime',
  '.klever',
  '.manga',
  '.polygon',
  '.unstoppable',
  '.pudgy',
  '.tball',
  '.stepn',
  '.secret',
  '.raiin',
  '.pog',
  '.clay',
  '.metropolis',
  '.witg',
  '.ubu',
  '.kryptic',
  '.farms',
  '.dfz',
]

export const supportedOneIDExtensions = [
  '.c98',
  '.atm',
  '.buidl',
  '.hodl',
  '.wagmi',
  '.btc',
  '.bitcoin',
  '.crypto',
  '.blockchain',
  '.wallet',
  '.nft',
  '.dao',
  '.ape',
  '.web3',
  '.multichain',
  '.102',
  '.888',
  '.aff',
  '.boss',
  '.kol',
  '.mmo',
  '.hc',
  '.bta',
  '.up',
  '.alix',
  '.gft',
  '.babydoge',
  '.gmvn'
]

export const allSupportedExtensions = [
  ...supportedENSExtensions,
  ...supportedSNSExtensions,
  ...supportedUDExtensions,
  ...supportedOneIDExtensions
]
