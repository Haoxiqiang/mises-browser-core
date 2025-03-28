/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_WALLET_BROWSER_ASSET_DISCOVERY_MANAGER_H_
#define BRAVE_COMPONENTS_BRAVE_WALLET_BROWSER_ASSET_DISCOVERY_MANAGER_H_

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/barrier_callback.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "mises/components/api_request_helper/api_request_helper.h"
#include "mises/components/brave_wallet/browser/blockchain_list_parser.h"
#include "mises/components/brave_wallet/common/brave_wallet.mojom.h"
#include "mises/components/brave_wallet/common/brave_wallet_types.h"
#include "mises/components/brave_wallet/common/solana_address.h"
#include "mojo/public/cpp/bindings/receiver.h"

class PrefService;

namespace brave_wallet {

class BraveWalletService;
class JsonRpcService;
class KeyringService;

class AssetDiscoveryManager : public mojom::KeyringServiceObserver {
 public:
  using APIRequestHelper = api_request_helper::APIRequestHelper;
  using APIRequestResult = api_request_helper::APIRequestResult;

  AssetDiscoveryManager(std::unique_ptr<APIRequestHelper> api_request_helper,
                        BraveWalletService* wallet_service,
                        JsonRpcService* json_rpc_service,
                        KeyringService* keyring_service,
                        PrefService* prefs);

  AssetDiscoveryManager(const AssetDiscoveryManager&) = delete;
  AssetDiscoveryManager& operator=(AssetDiscoveryManager&) = delete;
  ~AssetDiscoveryManager() override;

  // KeyringServiceObserver
  void KeyringCreated(const std::string& keyring_id) override {}
  void KeyringRestored(const std::string& keyring_id) override {}
  void KeyringReset() override {}
  void Locked() override {}
  void Unlocked() override {}
  void BackedUp() override {}
  void AccountsChanged() override {}
  void AccountsAdded(mojom::CoinType coin,
                     const std::vector<std::string>& addresses) override;
  void AutoLockMinutesChanged() override {}
  void SelectedAccountChanged(mojom::CoinType coin) override {}

  using DiscoverAssetsCompletedCallbackForTesting =
      base::RepeatingCallback<void(
          std::vector<mojom::BlockchainTokenPtr> discovered_assets_for_chain)>;

  // Called by frontend via BraveWalletService.
  // Subject to client side rate limiting based on
  // kBraveWalletLastDiscoveredAssetsAt pref value.
  void DiscoverAssetsOnAllSupportedChainsRefresh(
      const std::map<mojom::CoinType, std::vector<std::string>>&
          account_addresses);

  void SetSupportedChainsForTesting(
      const std::vector<std::string> supported_chains_for_testing) {
    supported_chains_for_testing_ = supported_chains_for_testing;
  }

  void SetDiscoverAssetsCompletedCallbackForTesting(
      DiscoverAssetsCompletedCallbackForTesting callback) {
    discover_assets_completed_callback_for_testing_ = std::move(callback);
  }

 private:
  friend class AssetDiscoveryManagerUnitTest;
  FRIEND_TEST_ALL_PREFIXES(AssetDiscoveryManagerUnitTest,
                           DiscoverAssetsOnAllSupportedChainsRefresh);
  FRIEND_TEST_ALL_PREFIXES(AssetDiscoveryManagerUnitTest,
                           GetAssetDiscoverySupportedEthChains);
  FRIEND_TEST_ALL_PREFIXES(AssetDiscoveryManagerUnitTest,
                           ParseNFTsFromSimpleHash);
  FRIEND_TEST_ALL_PREFIXES(AssetDiscoveryManagerUnitTest, DecodeMintAddress);
  FRIEND_TEST_ALL_PREFIXES(AssetDiscoveryManagerUnitTest,
                           GetSimpleHashNftsByWalletUrl);

  const std::vector<std::string>& GetAssetDiscoverySupportedEthChains();

  void DiscoverSolAssets(const std::vector<std::string>& account_addresses,
                         bool triggered_by_accounts_added);

  void OnGetSolanaTokenAccountsByOwner(
      base::OnceCallback<void(std::vector<SolanaAddress>)> barrier_callback,
      const std::vector<SolanaAccountInfo>& token_accounts,
      mojom::SolanaProviderError error,
      const std::string& error_message);

  void MergeDiscoveredSolanaAssets(
      bool triggered_by_accounts_added,
      const std::vector<std::vector<SolanaAddress>>&
          all_discovered_contract_addresses);

  void OnGetSolanaTokenRegistry(
      bool triggered_by_accounts_added,
      const base::flat_set<std::string>& discovered_contract_addresses,
      std::vector<mojom::BlockchainTokenPtr> sol_token_registry);

  void DiscoverEthAssets(const std::vector<std::string>& account_addresses,
                         bool triggered_by_accounts_added);

  void OnGetERC20TokenBalances(
      base::OnceCallback<void(std::map<std::string, std::vector<std::string>>)>
          barrier_callback,
      const std::string& chain_id,
      bool triggered_by_accounts_added,
      const std::vector<std::string>& contract_addresses,
      std::vector<mojom::ERC20BalanceResultPtr> balance_results,
      mojom::ProviderError error,
      const std::string& error_message);

  void MergeDiscoveredEthAssets(
      base::flat_map<std::string,
                     base::flat_map<std::string, mojom::BlockchainTokenPtr>>
          chain_id_to_contract_address_to_token,
      bool triggered_by_accounts_added,
      const std::vector<std::map<std::string, std::vector<std::string>>>&
          discovered_assets);

  using FetchNFTsFromSimpleHashCallback =
      base::OnceCallback<void(std::vector<mojom::BlockchainTokenPtr> nfts)>;
  void FetchNFTsFromSimpleHash(const std::string& account_address,
                               const std::vector<std::string>& chain_ids,
                               mojom::CoinType coin,
                               FetchNFTsFromSimpleHashCallback callback);

  void OnFetchNFTsFromSimpleHash(
      std::vector<mojom::BlockchainTokenPtr> nfts_so_far,
      mojom::CoinType coin,
      FetchNFTsFromSimpleHashCallback callback,
      APIRequestResult api_request_result);

  void DiscoverNFTsOnAllSupportedChains(
      const std::map<mojom::CoinType, std::vector<std::string>>&
          account_addresses,
      bool triggered_by_accounts_added);

  void MergeDiscoveredNFTs(
      bool triggered_by_accounts_added,
      const std::vector<std::vector<mojom::BlockchainTokenPtr>>& nfts);

  std::optional<std::pair<GURL, std::vector<mojom::BlockchainTokenPtr>>>
  ParseNFTsFromSimpleHash(const base::Value& json_value, mojom::CoinType coin);

  // CompleteDiscoverAssets signals that the discover assets request has
  // completed for a given chain_id
  void CompleteDiscoverAssets(
      const std::string& chain_id,
      std::vector<mojom::BlockchainTokenPtr> discovered_assets,
      std::optional<mojom::ProviderError> error,
      const std::string& error_message,
      bool triggered_by_accounts_added);

  // CompleteDiscoverAssets signals that the discover assets request has
  // completed for a given chain_id x account_address combination.
  void CompleteDiscoverAssets(
      std::vector<mojom::BlockchainTokenPtr> discovered_assets,
      bool triggered_by_accounts_added);

  // Triggered by when KeyringService emits AccountsAdded event.
  // Rate limits will be ignored.
  void DiscoverAssetsOnAllSupportedChainsAccountsAdded(
      mojom::CoinType coin,
      const std::vector<std::string>& account_addresses);

  static std::optional<SolanaAddress> DecodeMintAddress(
      const std::vector<uint8_t>& data);
  static GURL GetSimpleHashNftsByWalletUrl(
      const std::string& account_address,
      const std::vector<std::string>& chain_ids);

  // remaining_buckets_ is the number of 'buckets' of requets remaining for an
  // in-flight DiscoverAssetsOnAllSupportedChainsRefresh call to be completed.
  // When no call is in-flight, remaining_buckets_ is 0.  When a call is
  // in-flight, remaining_buckets_ is > 0 and the AssetDiscoverManager will
  // refuse to process additional  DiscoverAssetsOnAllSupportedChainsRefresh
  // calls.
  //
  // DiscoverAssetsOnAllSupportedChainsAccountsAdded does not read from or write
  // to remaining_buckets_ and thus those calls will always processed.
  int remaining_buckets_ = 0;
  std::vector<mojom::BlockchainTokenPtr> discovered_assets_;
  std::vector<std::string> supported_chains_for_testing_;
  DiscoverAssetsCompletedCallbackForTesting
      discover_assets_completed_callback_for_testing_;
  std::unique_ptr<APIRequestHelper> api_request_helper_;
  raw_ptr<BraveWalletService> wallet_service_;
  raw_ptr<JsonRpcService> json_rpc_service_;
  raw_ptr<KeyringService> keyring_service_;
  raw_ptr<PrefService> prefs_;
  mojo::Receiver<brave_wallet::mojom::KeyringServiceObserver>
      keyring_service_observer_receiver_{this};
  base::WeakPtrFactory<AssetDiscoveryManager> weak_ptr_factory_;
};

}  // namespace brave_wallet

#endif  // BRAVE_COMPONENTS_BRAVE_WALLET_BROWSER_ASSET_DISCOVERY_MANAGER_H_
