import * as React from "react";
import { FunctionComponent } from "react";
import Svg, { Path } from "react-native-svg";

export const VectorB: FunctionComponent<{
  height: number;
  color: string;
}> = ({ height, color }) => {
  return (
    <Svg
      fillRule="evenodd"
      strokeLinejoin="round"
      strokeMiterlimit="2"
      clipRule="evenodd"
      viewBox="0 0 27 36"
      style={{
        height,
        aspectRatio: 27 / 36,
      }}
    >
      <Path
        fill={color}
        fillRule="nonzero"
        d="M170.869 168.436a88.72 88.72 0 01-5.55-.175c-1.866-.117-3.5-.358-4.9-.725v-33.65a60.069 60.069 0 012.35-.375 41.268 41.268 0 012.55-.275 67.302 67.302 0 015-.2c2.1 0 4.05.158 5.85.475 1.8.317 3.359.842 4.675 1.575 1.317.733 2.342 1.7 3.075 2.9.734 1.2 1.1 2.683 1.1 4.45 0 1.633-.391 3.033-1.175 4.2-.783 1.167-1.875 2.1-3.275 2.8 2.1.7 3.65 1.733 4.65 3.1 1 1.367 1.5 3.1 1.5 5.2 0 3.567-1.3 6.242-3.9 8.025-2.6 1.783-6.583 2.675-11.95 2.675zm-4.25-16v10.55c.7.067 1.45.117 2.25.15.8.033 1.534.05 2.2.05 1.3 0 2.509-.083 3.625-.25 1.117-.167 2.084-.458 2.9-.875.817-.417 1.467-.975 1.95-1.675.484-.7.725-1.6.725-2.7 0-1.967-.716-3.333-2.15-4.1-1.433-.767-3.416-1.15-5.95-1.15h-5.55zm0-4.95h4.45c2.4 0 4.284-.342 5.65-1.025 1.367-.683 2.05-1.908 2.05-3.675 0-1.667-.708-2.85-2.125-3.55-1.416-.7-3.258-1.05-5.525-1.05-.966 0-1.825.017-2.575.05-.75.033-1.391.083-1.925.15v9.1z"
        transform="translate(-160.419 -133.036)"
      />
    </Svg>
  );
};
