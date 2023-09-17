// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import { useDispatch } from 'react-redux'

// Types
import { BraveWallet, PanelTypes } from '../../../constants/types'

// actions
import { WalletActions } from '../../../common/actions'

// utils
import { getLocale } from '../../../../common/locale'
import { useGetSelectedChainQuery } from '../../../common/slices/api.slice'

// Styled Components
import {
  StyledWrapper,
  PopupButton,
  PopupButtonText,
  SettingsIcon,
  LockIcon,
  ExplorerIcon,
  BackupIcon,
  ConnectedSitesIcon,
  HelpCenterIcon
} from './style'

export interface Props {
  onClickViewOnBlockExplorer?: () => void
  onClickViewOnActivity?: (path: PanelTypes) => void
  onClickBackup?: () => void
  onClickSetting?: () => void
  onClickConnectedSites?: () => void
  onClosePopup?: () => void
  yPosition?: number
}

export const WalletMorePopup = (props: Props) => {
  const {
    onClickViewOnBlockExplorer,
    // onClickViewOnActivity,
    onClickBackup,
    onClosePopup,
    onClickSetting,
    onClickConnectedSites,
    yPosition
  } = props

  // redux
  const dispatch = useDispatch()

  // queries
  const { data: selectedNetwork } = useGetSelectedChainQuery()

  // methods
  const lockWallet = React.useCallback(() => {
    dispatch(WalletActions.lockWallet())
  }, [])

  // const onClickConnectedSites = React.useCallback(() => {
  //   if (!selectedNetwork) {
  //     return
  //   }

  //   const route = selectedNetwork.coin === BraveWallet.CoinType.ETH
  //     ? 'ethereum'
  //     : 'solana'

  //   chrome.tabs.create({ url: `brave://settings/content/${route}` }, () => {
  //     if (chrome.runtime.lastError) {
  //       console.error(
  //         'tabs.create failed: ' +
  //         chrome.runtime.lastError.message
  //       )
  //     }
  //   })
  //   if (onClosePopup) {
  //     onClosePopup()
  //   }
  // }, [selectedNetwork, onClosePopup])

  const onClickHelpCenter = React.useCallback(() => {
    chrome.tabs.create(
      {
        url: 'https://support.brave.com/hc/en-us/articles/4415497656461-Brave-Wallet-FAQ'
      }, () => {
        if (chrome.runtime.lastError) {
          console.error(
            'tabs.create failed: '
            + chrome.runtime.lastError.message
          )
        }
      })
    if (onClosePopup) {
      onClosePopup()
    }
  }, [onClosePopup])

  // const onClickSettings = React.useCallback(() => {
  //   // chrome.tabs.create({ url: 'chrome://settings/wallet' }, () => {
  //   //   if (chrome.runtime.lastError) {
  //   //     console.error(
  //   //       'tabs.create failed: ' +
  //   //       chrome.runtime.lastError.message
  //   //     )
  //   //   }
  //   // })
  //   if (onClosePopup) {
  //     onClosePopup()
  //   }
  // }, [onClosePopup])

  return (
    <StyledWrapper yPosition={yPosition}>

      <PopupButton onPress={lockWallet}>
        <LockIcon />
        <PopupButtonText>
          {getLocale('braveWalletWalletPopupLock')}
        </PopupButtonText>
      </PopupButton>

      {onClickBackup &&
        <PopupButton onPress={onClickBackup}>
          <BackupIcon />
          <PopupButtonText>
            {getLocale('braveWalletBackupButton')}
          </PopupButtonText>
        </PopupButton>
      }

      {
        selectedNetwork &&
        selectedNetwork.coin !== BraveWallet.CoinType.FIL &&
        <PopupButton onPress={() => {
          onClickConnectedSites?.()
          if (onClosePopup) {
            onClosePopup()
          }
        }}>
          <ConnectedSitesIcon />
          <PopupButtonText>
            {getLocale('braveWalletWalletPopupConnectedSites')}
          </PopupButtonText>
        </PopupButton>
      }

      <PopupButton onPress={() => {
          onClickSetting?.()
          if (onClosePopup) {
            onClosePopup()
          }
        }}>
        <SettingsIcon />
        <PopupButtonText>
          {getLocale('braveWalletWalletPopupSettings')}
        </PopupButtonText>
      </PopupButton>
      {/* <PopupButton onPress={() => onClickViewOnActivity?.('activity')}>
        <SettingsIcon />
        <PopupButtonText>
          {getLocale('braveWalletWalletPopupActivity')}
        </PopupButtonText>
      </PopupButton> */}

      {onClickViewOnBlockExplorer &&
        <PopupButton onPress={onClickViewOnBlockExplorer}>
          <ExplorerIcon />
          <PopupButtonText>
            {getLocale('braveWalletTransactionExplorer')}
          </PopupButtonText>
        </PopupButton>
      }

      <PopupButton onPress={onClickHelpCenter}>
        <HelpCenterIcon />
        <PopupButtonText>
          {getLocale('braveWalletHelpCenter')}
        </PopupButtonText>
      </PopupButton>
    </StyledWrapper>
  )
}

export default WalletMorePopup
