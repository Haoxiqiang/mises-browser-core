import * as React from "react";
import { FunctionComponent } from "react";
import Svg, { Path } from "react-native-svg";

export const GoogleIcon: FunctionComponent<{
  width?: number | string;
  height?: number | string;
}> = ({ width = 21, height = 20 }) => {
  return (
    <Svg
      viewBox="0 0 21 20"
      style={{
        width,
        height,
      }}
    >
      <Path
        d="M18.329 8.368h-.647v-.035h-7.227v3.334h4.538C14.33 13.607 12.552 15 10.454 15c-2.66 0-4.818-2.239-4.818-5s2.158-5 4.818-5c1.229 0 2.346.48 3.197 1.266l2.271-2.357c-1.434-1.387-3.352-2.242-5.468-2.242-4.434 0-8.03 3.73-8.03 8.333 0 4.602 3.596 8.333 8.03 8.333 4.435 0 8.03-3.73 8.03-8.333 0-.559-.055-1.104-.155-1.632z"
        fill="#FBC02D"
      />
      <Path
        d="M3.35 6.121L5.988 8.13C6.702 6.295 8.431 5 10.454 5c1.229 0 2.346.48 3.197 1.266l2.271-2.357c-1.434-1.387-3.352-2.242-5.468-2.242-3.084 0-5.759 1.807-7.104 4.454z"
        fill="#E53935"
      />
      <Path
        d="M10.454 18.333c2.075 0 3.96-.823 5.384-2.163l-2.485-2.182A4.67 4.67 0 0110.454 15c-2.088 0-3.862-1.382-4.53-3.31l-2.618 2.093c1.329 2.699 4.028 4.55 7.149 4.55z"
        fill="#4CAF50"
      />
      <Path
        d="M18.329 8.368l-.007-.035h-7.867v3.334h4.538a5.01 5.01 0 01-1.641 2.32h.001l2.485 2.183c-.175.165 2.647-2.003 2.647-6.17 0-.559-.056-1.104-.156-1.632z"
        fill="#1565C0"
      />
    </Svg>
  );
};
