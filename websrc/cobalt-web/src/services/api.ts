import request from "../services/request";

export const getResouceList = () =>
  request({
    url: "/getResouceList",
    method: "get",
  });

// ---------------设备连接相关的接口
// 具体说明见后端的接口文档
export const getDeviceBrand = () =>
  request<string[]>({
    url: "/driver_connect/device_brand/",
    method: "get",
  });

export const getDeviceList = () =>
  request({
    url: "/driver_connect/device_list/",
    method: "get",
  });

export const getDriverServerStatus = () =>
  request({
    url: "/driver_connect/server_status/",
    method: "get",
  });

// 启动和重启设备服务器，都可以用这个。注意重启必须要双重确认后才行
export const postStartDeviceServer = (body: any) =>
  request({
    url: "/driver_connect/start_server/",
    method: "post",
    data: body,
  });

// 关闭服务器也最好双重确认
export const postCloseDeviceServer = () =>
  request({
    url: "/driver_connect/close_server/",
    method: "post",
  });

export const postDeviceStatus = (body: { device: string }) =>
  request({
    url: "/driver_connect/device_status/",
    method: "post",
    data: body,
  });
// 连接设备接口，必须连接后，才能使用。
export const ConnectDevice = (
  start_or_stop: "start" | "stop",
  device_type: "guider" | "telescope" | "camera" | "focus" | "filter" | "polar",
  device_name: string
) =>
  request({
    url: `/device_connect/${start_or_stop}/${device_type}/${device_name}/`,
    method: "get",
  });

// ---------------

export const GetCurrentDeviceProfile = (): Promise<IndiConnectProfile> =>
  request({
    url: "/driver_connect/current_profile/",
    method: "get",
  });

export const postCheckPhd2 = (body: { camera: string; telescope: string }) =>
  request({
    url: "/phd2/check_profile/",
    method: "post",
    data: body,
  });

export const postConnectPhd2 = () =>
  request({
    url: "/phd2/connect_device/",
    method: "post",
  });

export const postStartPhd2 = () =>
  request({
    url: "/phd2/start/",
    method: "post",
  });
