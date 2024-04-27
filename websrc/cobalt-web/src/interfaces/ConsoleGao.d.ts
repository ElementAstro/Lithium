declare interface ICThreePointTarget {
  ra: number;
  dec: number;
}
// hfr星点数据
declare interface ICHFRDataPointList {
  star_count: Array<number>;
  average_hfr: Array<number>;
}
declare interface ICSingleHFRPointData {
  star_count: number;
  average_hfr: number;
  max_star: number;
  min_star: number;
  average_star: number;
}

// phd2导星数据记录
declare interface ICPHD2GuideDataPointList {
  dx: Array<number>;
  dy: Array<number>;
  RaDistance: Array<number>;
  DecDistance: Array<number>;
  RaControl: Array<number>;
  DecControl: Array<number>;
}

declare interface ICPHD2InterfaceGuideStep {
  Frame: number;
  Time: number;
  Mount: string;
  dx: number;
  dy: number;
  RADistanceRaw: number;
  DECDistanceRaw: number;
  RADistanceGuide: number;
  DECDistanceGuide: number;
  RADuration: number;
  RADirection: string;
  DECDuration: number;
  DECDirection: string;
  StarMass: number;
  SNR: number;
  HFD: number;
  AvgDist: number;
  RALimited: bool;
  DecLimited: bool;
  ErrorCode: number;
}

declare interface ICPHD2InterfaceCalibrationResult {
  calibrated: boolean;
  xAngle: number;
  xRate: number;
  xParity: "+" | "-";
  yAngle: number;
  yRate: number;
  yParity: "+" | "-";
  declination: number;
}

// general interface
declare interface IResponseFlagGeneral {
  success: boolean;
  data: {
    flag: boolean;
  };
}

declare interface IResponseDataAny {
  success: boolean;
  data: any;
}

// phd2 related http interface
declare interface IPHD2ResponseStatus {
  success: boolean;
  data: {
    flag:
      | "Stopped"
      | "Selected"
      | "Calibrating"
      | "Guiding"
      | "LostLock"
      | "Paused"
      | "Looping"
      | "TakingDark"
      | "Settling"
      | "SettleDone";
  };
}
