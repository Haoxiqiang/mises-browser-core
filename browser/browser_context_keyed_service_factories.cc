#include "mises/browser/browser_context_keyed_service_factories.h"

#include "base/feature_list.h"
#include "mises/components/ipfs/buildflags/buildflags.h"
#include "mises/browser/brave_wallet/asset_ratio_service_factory.h"
#include "mises/browser/brave_wallet/brave_wallet_service_factory.h"
#include "mises/browser/brave_wallet/json_rpc_service_factory.h"
#include "mises/browser/brave_wallet/keyring_service_factory.h"
#include "mises/browser/brave_wallet/swap_service_factory.h"
#include "mises/browser/brave_wallet/tx_service_factory.h"
#include "mises/browser/permissions/permission_lifetime_manager_factory.h"



#include "chrome/browser/profiles/chrome_browser_main_extra_parts_profiles.h"

#if BUILDFLAG(IS_ANDROID)
#include "chrome/browser/android/app_menu_bridge.h"

#include "chrome/browser/apps/app_service/app_service_proxy_factory.h"
#include "chrome/browser/search/instant_service_factory.h"
#include "chrome/browser/ui/tabs/pinned_tab_service_factory.h"
#include "chrome/browser/themes/theme_service_factory.h"
#include "chrome/browser/user_education/user_education_service_factory.h"
#include "chrome/browser/sessions/session_service_factory.h"
#include "chrome/browser/sessions/exit_type_service_factory.h"
#endif

#if BUILDFLAG(ENABLE_IPFS)
#include "mises/browser/ipfs/ipfs_service_factory.h"
#endif


namespace mises {

void EnsureMisesBrowserContextKeyedServiceFactoriesBuilt() {
#if BUILDFLAG(ENABLE_IPFS)
  ipfs::IpfsServiceFactory::GetInstance();
#endif
  brave_wallet::AssetRatioServiceFactory::GetInstance();
  brave_wallet::KeyringServiceFactory::GetInstance();
  brave_wallet::JsonRpcServiceFactory::GetInstance();
  brave_wallet::SwapServiceFactory::GetInstance();
  brave_wallet::TxServiceFactory::GetInstance();
  brave_wallet::BraveWalletServiceFactory::GetInstance();

  PermissionLifetimeManagerFactory::GetInstance();

#if BUILDFLAG(IS_ANDROID)
  AppMenuBridge::Factory::GetInstance();
#endif

}
void EnsureBrowserContextKeyedServiceFactoriesBuiltExtra() {
#if BUILDFLAG(IS_ANDROID)
  EnsureBrowserContextKeyedServiceFactoriesBuilt();
  InstantServiceFactory::GetInstance();
  PinnedTabServiceFactory::GetInstance();
  ThemeServiceFactory::GetInstance();
  apps::AppServiceProxyFactory::GetInstance();
  UserEducationServiceFactory::GetInstance();
  SessionServiceFactory::GetInstance();
  ExitTypeServiceFactory::GetInstance();
#endif
  EnsureMisesBrowserContextKeyedServiceFactoriesBuilt();
}

void EnsureBrowserContextKeyedServiceFactoriesBuilt() {
#if !BUILDFLAG(IS_ANDROID)
  ChromeBrowserMainExtraPartsProfiles::EnsureBrowserContextKeyedServiceFactoriesBuilt();
  EnsureMisesBrowserContextKeyedServiceFactoriesBuilt();

#else
  ChromeBrowserMainExtraPartsProfiles::EnsureBrowserContextKeyedServiceFactoriesBuiltAndroid();
  PermissionLifetimeManagerFactory::GetInstance();
#endif
}

}
