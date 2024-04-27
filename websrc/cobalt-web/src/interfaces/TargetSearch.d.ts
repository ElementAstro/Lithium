declare interface IDSOObjectInfo {
  name: string;
  ra: float;
  dec: float;
}

declare interface IDSOFramingObjectInfo {
  name: string;
  ra: float;
  dec: float;
  rotation: float;
  flag: string; // flag is user editable, any string is ok
  tag: string; // tag is system set filters.
  target_type: string;
  size: float;
  checked: boolean;
  // depreciated
  bmag?: float;
  vmag?: float;
}

declare interface IDSOObjectDetailedInfo {
  name: string;
  alias: string;
  ra: float;
  dec: float;
  target_type: string;
  const: string;
  size: float;
  transit_month: number;
  transit_date: string;
  filter: string;
  focal_length: number;
  altitude: Array<[string, float, float]>;
  Top200: number | null;
  // depreciated
  moon_distance?: float | null;
  bmag?: float | null;
  vmag?: float | null;
}

declare interface IDSOObjectSimpleInfo {
  current: number;
  highest: number;
  available_shoot_time: number;
}

declare interface ILightStarInfo {
  name: string;
  show_name: string;
  ra: float;
  dec: float;
  Const: string;
  Const_Zh: string;
  magnitude: float;
  alt: float;
  az: float;
  sky: string;
}

declare interface ITwilightDataString {
  evening: {
    sun_set_time: string;
    evening_civil_time: string;
    evening_nautical_time: string;
    evening_astro_time: string;
  };
  morning: {
    sun_rise_time: string;
    morning_civil_time: string;
    morning_nautical_time: string;
    morning_astro_time: string;
  };
}

declare interface ITwilightData {
  evening: {
    sun_set_time: Date;
    evening_civil_time: Date;
    evening_nautical_time: Date;
    evening_astro_time: Date;
  };
  morning: {
    sun_rise_time: Date;
    morning_civil_time: Date;
    morning_nautical_time: Date;
    morning_astro_time: Date;
  };
}

// interface from the api

declare interface IOFRequestLightStar {
  sky_range?: Array<string>;
  max_mag?: number;
}

declare interface IOFResponseLightStar {
  success: boolean;
  data: Array<ILightStarInfo>;
}

declare interface IOFResponseFindTargetName {
  success: boolean;
  data: Array<IDSOObjectDetailedInfo>;
}

declare interface IOFRequestFOVpoints {
  x_pixels: number;
  y_pixels: number;
  x_pixel_size: number;
  y_pixel_size: number;
  focal_length: number;
  target_ra: number;
  target_dec: number;
  camera_rotation: number;
}

declare interface IOFResponseFOVpoints {
  success: boolean;
  data: [
    [number, number],
    [number, number],
    [number, number],
    [number, number]
  ];
  message?: string;
}

declare interface IOFRequestFOVpointsTiles {
  x_pixels: number;
  y_pixels: number;
  x_pixel_size: number;
  y_pixel_size: number;
  focal_length: number;
  target_ra: number;
  target_dec: number;
  camera_rotation: number;
  x_tiles: number;
  y_tiles: number;
  overlap: number;
}

declare interface IOFResponseFOVpointsTiles {
  success: boolean;
  data: Array<
    [[number, number], [number, number], [number, number], [number, number]]
  >;
  message?: string;
}

declare interface IOFResponseAltCurve {
  success: boolean;
  data: {
    moon_distance: float;
    altitude: Array<[string, float, float]>;
  };
}

declare interface IOFResponseTwilightData {
  success: boolean;
  data: ITwilightDataString;
}

declare interface IOFResponseOBJSimple {
  success: boolean;
  data: IDSOObjectSimpleInfo;
}
// end of interface from the api
