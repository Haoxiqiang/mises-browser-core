// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import { useDispatch, useSelector } from 'react-redux'

// actions
import { WalletPageActions } from '../../../actions'

// utils
import { getLocale } from '../../../../../common/locale'
import { useApiProxy } from '../../../../common/hooks/use-api-proxy'

// components
import { WalletPageLayout } from '../../../../components/desktop'
import { NavButton } from '../../../../components/extension'
import {
  OnboardingDisclosures,
  OnboardingDisclosuresNextSteps
} from '../disclosures/disclosures'

// routes
import { OnboardingAction, PageState, WalletRoutes } from '../../../../constants/types'

// styles
import { VerticalSpace } from '../../../../components/shared/style'
import { OnboardingWrapper } from '../onboarding.style'
import {
  // Title,
  // ButtonContainer,
  // LearnMoreLink,
  // BlockQuote,
  // BlockQuoteTextContainer,
  // VerticalRule,
  // SubDivider,
  SubDividerText
} from './onboarding-welcome.style'
import { Image } from 'react-native'
import { StyleSheet } from 'react-native'
import { useStyle } from '../../../../page/styles'

export const OnboardingWelcome = () => {
  // redux
  const dispatch = useDispatch()
  const setupStillInProgress = useSelector(({ page }: { page: PageState }) => page.setupStillInProgress)

  // state
  const [nextStep, setNextStep] = React.useState<OnboardingDisclosuresNextSteps | undefined>(undefined)

  // methods
  const hideDisclosures = React.useCallback(() => setNextStep(undefined), [])

  const showNewWalletDisclosures = React.useCallback(
    () => setNextStep(WalletRoutes.OnboardingCreatePassword),
    []
  )

  const showRestoredWalletDisclosures = React.useCallback(
    () => setNextStep(WalletRoutes.OnboardingImportOrRestore),
    []
  )

  // const showConnectHardwareDisclosures = React.useCallback(() => {
  //   setNextStep(WalletRoutes.OnboardingConnectHarwareWalletCreatePassword)
  // }, [])

  // custom hooks
  const { braveWalletP3A } = useApiProxy()

  // effects
  React.useEffect(() => {
    // start wallet setup
    if (!setupStillInProgress) {
      dispatch(WalletPageActions.walletSetupComplete(false))
    }
    braveWalletP3A.reportOnboardingAction(OnboardingAction.SHOWN)
  }, [setupStillInProgress])

  const style = useStyle()

  // render
  if (nextStep !== undefined) {
    return <OnboardingDisclosures nextStep={nextStep} onBack={hideDisclosures} />
  }

  return <WalletPageLayout>
    <OnboardingWrapper>
      {/* <WalletWelcomeGraphic />

      <Title maxWidth='467px'>
        {getLocale('braveWalletWelcomeTitle')}
      </Title>

      <VerticalSpace space='34px' />
      <BlockQuote>
        <VerticalRule />
        <BlockQuoteTextContainer>
          <span>
            {getLocale('braveWalletPerksTokens')}
          </span>
          <span>
            {getLocale('braveWalletMultiChain')}
          </span>
          <span>
            {getLocale('braveWalletPerksBrowserNative')}
          </span>
        </BlockQuoteTextContainer>
      </BlockQuote> */}
      <Image
        style={StyleSheet.flatten([
          style.flatten(["height-90"]),
          {
            width: '50%'
          }
        ])}
        fadeDuration={0}
        resizeMode="contain"
        source={require("../../../assets/logo/logo.png")}
      />

      <VerticalSpace space='34px' />

      <NavButton
        buttonType='primary'
        text={getLocale('braveWalletWelcomeButton')}
        onSubmit={showNewWalletDisclosures}
        maxHeight={'48px'}
        minWidth={'267px'}
      />

      <SubDividerText>{getLocale('braveWalletWelcomeDividerText')}</SubDividerText>
      <NavButton
        buttonType='secondary'
        text={getLocale('braveWalletImportExistingWallet')}
        onSubmit={showRestoredWalletDisclosures}
        maxHeight={'48px'}
        minWidth={'267px'}
      />

      <VerticalSpace space='34px' />

      {/* <NavButton
        buttonType='primary'
        text={getLocale('braveWalletConnectHardwareWallet')}
        onSubmit={showConnectHardwareDisclosures}
        maxHeight={'48px'}
        minWidth={'267px'}
      /> */}

      {/* <VerticalSpace space='20px' /> */}

      {/* <LearnMoreLink
        href='https://support.brave.com/hc/en-us/categories/360001059151-Brave-Wallet'
        target='_blank'
        rel='noreferrer'
      >
        {getLocale('braveWalletLearnMoreAboutBraveWallet')}
      </LearnMoreLink> */}

    </OnboardingWrapper>

  </WalletPageLayout>
}
