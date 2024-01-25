import request from "../services/request";
import { AxiosResponse } from "axios";

export const get_paa_status = (): Promise<IPAAFixedStatus> =>
  request({
    url: "/PAA/status/",
    method: "get",
  });

export const get_update_paa_running_status =
  (): Promise<IPAAFixedUpdateRunningStatus> =>
    request({
      url: "/PAA/update_paa_running_status/",
      method: "get",
    });

export const post_stop_paa = (): Promise<IPAAFixedStopResponse> =>
  request({
    url: "/PAA/stop/",
    method: "post",
  });

export const start_fixed_Goto = (
  goto_point: IPAAFixedGotoRequest
): Promise<IPAAFixedStartResponse> =>
  request({
    url: "/PAA/start_fixed_script/",
    method: "post",
    data: {
      script_name: "Goto",
      params: {
        target_ra: goto_point.ra,
        target_dec: goto_point.dec,
        center: false,
      },
    },
  });

export const start_fixed_Goto_Center = (
  goto_point: IPAAFixedGotoRequest
): Promise<IPAAFixedStartResponse> =>
  request({
    url: "/PAA/start_fixed_script/",
    method: "post",
    data: {
      script_name: "GotoCenter",
      params: {
        target_ra: goto_point.ra,
        target_dec: goto_point.dec,
        center: true,
      },
    },
  });

export const start_fixed_PolarAlignment = (
  polar_align_setting: IPAAFixedPolarAlignmentRequest
): Promise<IPAAFixedStartResponse> =>
  request({
    url: "/PAA/start_fixed_script/",
    method: "post",
    data: {
      script_name: "PolarAlignment",
      params: polar_align_setting,
    },
  });

export const get_newest_camera_jpg_data = (): Promise<Blob> =>
  request({
    url: "/PAA/newest_camera_jpg/",
    method: "get",
  });

export const get_newest_guider_jpg_data = (): Promise<Blob> =>
  request({
    url: "/PAA/newest_guider_jpg/",
    method: "get",
  });
