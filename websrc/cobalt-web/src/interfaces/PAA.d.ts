declare interface PAABaseStepsInfo {
  device: string;
  instruction: string;
  params: any;
  id: string;
  count?: number;
  repeat?: number;
  children?: Array<PAABaseStepsInfo>;
  conditions?: Array<PAABseCondiitonInfo>;
  name?: string;
  ra?: number;
  dec?: number;
  after_loop?: Array<PAABaseAfterLoopInfo>;
}

declare interface PAABseCondiitonInfo {
  if_condition: string;
  params: any;
}

declare interface PAABaseAfterLoopInfo {
  if_condition: string;
  params: any;
}

declare interface PAAScriptInfo {
  file_name: string;
  modified_time: string;
}

declare interface PAAlog_info {
  data: {
    params?: {
      count: number;
    };
  };
  device_name: string;
  instruction: string;
  message: string;
  type: string;
}

// script type
declare interface exposureProps {
  exposure_time: number;
  exposure_count: number;
  dither: number;
  type: string;
  binning: number;
  filter?: number;
}

declare interface targetProps {
  target_name: string;
  ra: number;
  dec: number;
  rotation?: number;
  exposure_list: Array<exposureProps>;
}

declare interface IPAAColorScriptDataSetting {
  target_list: Array<targetProps>;
  warm_camera: boolean;
  cool_camera: boolean;
  cool_temperature: number;
  af_on_start: boolean;
  af_on_hfr_change: number;
  af_after_exposure: number;
  af_after_temperature: number;
  recenter_if_drift: number;
  unpark: boolean;
  park: boolean;
  find_home: boolean;
  meridian_flip: boolean;
  guide_exposure?: number;
  restore_guide?: boolean;
}

declare interface IPAAMonoScriptDataSetting {
  target_list: Array<targetProps>;
  warm_camera: boolean;
  cool_camera: boolean;
  cool_temperature: number;
  af_on_start: boolean;
  af_on_hfr_change: number;
  af_after_exposure: number;
  af_after_temperature: number;
  af_after_filter_change: boolean;
  recenter_if_drift: number;
  unpark: boolean;
  park: boolean;
  find_home: boolean;
  meridian_flip: boolean;
  guide_exposure?: number;
  restore_guide?: boolean;
}

declare interface IPAADarkScriptDataSetting {
  cool_temperature: number;
  dark_time: Array<number>;
  repeat: number;
  warm_camera: boolean;
  bias: boolean;
}

// -----------------------
// IO related

declare interface IPAAScriptDataType {
  script: Array<PAABaseStepsInfo>;
  setting: any;
  type: string;
}

declare interface IPAAScripteResponse {
  success: boolean;
  data: IPAAScriptDataType;
  message?: string;
}
