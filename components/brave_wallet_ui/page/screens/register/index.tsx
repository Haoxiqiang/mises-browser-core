import * as React from "react";
import { FunctionComponent } from "react";
import { useHeaderHeight } from "@react-navigation/elements";
import { PageWithScrollView } from "../../components/page";
import { useStyle } from "../../styles";
import { View, Dimensions, Image } from "react-native";
import { Button } from "../../components/button";
import { useSmartNavigation } from "../../navigation";
import { useSafeAreaInsets } from "react-native-safe-area-context";

export const RegisterIntroScreen: FunctionComponent = () => {
  const style = useStyle();

  const smartNavigation = useSmartNavigation();

  // const registerConfig = useRegisterConfig(keyRingStore, []);

  const safeAreaInsets = useSafeAreaInsets();
  const headerHeight = useHeaderHeight();
  const actualHeightHeight = headerHeight - safeAreaInsets.top;

  return (
    <PageWithScrollView
      backgroundMode="gradient"
      contentContainerStyle={style.get("flex-grow-1")}
      style={{
        ...style.flatten(["padding-x-42"]),
        paddingTop: Dimensions.get("window").height * 0.22 - actualHeightHeight,
        paddingBottom: Dimensions.get("window").height * 0.11,
      }}
    >
      <View
        style={style.flatten(["flex-grow-1", "items-center", "padding-x-18"])}
      >
        <Image
          source={
            style.theme === "dark"
              ? require("../../assets/logo/keplr-logo-dark-mode.png")
              : require("../../assets/logo/keplr-logo.png")
          }
          style={{
            height: 90,
            aspectRatio: 2.977,
          }}
          resizeMode="contain"
          fadeDuration={0}
        />
      </View>
      <Button
        containerStyle={style.flatten(["margin-bottom-16"])}
        text="Create a new wallet"
        size="large"
        onPress={() => {
          // analyticsStore.logEvent("Create account started", {
          //   registerType: "seed",
          // });
          smartNavigation.navigateSmart("Register.NewMnemonic");
        }}
      />
      <Button
        containerStyle={style.flatten(["margin-bottom-16"])}
        text="Import existing wallet"
        size="large"
        mode="light"
        onPress={() => {
          // analyticsStore.logEvent("Import account started", {
          //   registerType: "seed",
          // });
          smartNavigation.navigateSmart("Register.RecoverMnemonic");
        }}
      />
      {/* <Button
        text="Import Ledger Nano X"
        size="large"
        mode="text"
        onPress={() => {
          smartNavigation.navigateSmart("Register.NewLedger", {
            registerConfig,
          });
        }}
      /> */}
    </PageWithScrollView>
  );
};
