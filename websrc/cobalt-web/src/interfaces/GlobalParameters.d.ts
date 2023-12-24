declare interface IGeoLocation {
  longitude: number;
  latitude: number;
  height: number;
  time_zone: number;
}

declare interface IMeridianFlip {
  before_merdian: number;
  after_merdian: number;
  recenter_after_flip: boolean;
  autofocus_after_flip: boolean;
  settle_time_after: boolean;
}

declare interface ISettlingSetting {
  time: number; // seconds
  timeout: number; //seconds
  pixels: number; // count
}

declare interface ISingleFilterInfo {
  filter_name: string;
  focus_offset: number;
  af_exposure_time: number;
}

declare interface IFilterSetting {
  filter_number: number,
  filter_info: Array<ISingleFilterInfo>,
}

declare interface IAutofocus {
  use_filter_offset: boolean;
  step_size: number;
  initial_steps: number;
  default_exposure_time: number;
  retry_times: number;
  each_step_exposure: number;
  af_after_filter_change: boolean;
  af_offset_base_filter_slot: number;
}

declare interface IPlateSolve {
  use: 'astap' | 'astronomy';
  exposure_time: number;
  use_filter: number | 'current';
  downsample: number;
  tolerance: number;
}

declare interface ITelescopeInfo{
  name: string;
  apeture: number;
  focal_length: number;
  guider_aperture: number;
  guider_focal_length: number;
}

declare interface AnyCameraInfo {
  CCD_MAX_X: number;
  CCD_MAX_Y: number;
  CCD_PIXEL_SIZE: number;
}

declare interface ICameraGet {
  camera: boolean = false;
  guider_camera: boolean = false;
}

declare interface IGlobalParameters {
  [key: string]: any;
  geo_location: IGeoLocation | null;
  meridian_flip: IMeridianFlip | null;
  guider_start_guide_settle: ISettlingSetting | null;
  guider_dither_settle: ISettlingSetting | null;
  filter_setting: IFilterSetting | null;
  autofocus: IAutofocus | null;
  plate_solve: IPlateSolve | null;
  telescope_info: ITelescopeInfo | null;
  info_get: ICameraGet;
  camera_info: AnyCameraInfo | null;
  guider_camera_info: AnyCameraInfo | null;
}

declare interface IGPResponse {
  success: boolean;
  data: Partial<IGlobalParameters>;
  message?: string;
}

declare interface IGPSetParameterAPI {
  parameter_name: string;
  to_set_parameter: Partial<IGlobalParameters>;
}

declare interface IGPSingleParameterExplain {
  name: string;
  tooltips?: string;
  range?: Array<number>;
  unit?: string;
}

declare interface IGPParameterExplain{
  [key: string]: IGPSingleParameterExplain;
}

declare interface IGPPorfileList{
  current_profile: string;
  all_profiles: Array<string>;
}


declare interface IGPFilterSelection {
  label: string;
  value: number | string;
}