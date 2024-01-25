import request from "../services/request";
import { AxiosResponse } from "axios";

export const getPHD2connection = (): Promise<IResponseFlagGeneral> =>
  request({
    url: "/phd2/connection/",
    method: "get",
  });

export const getPHD2Status = (): Promise<IPHD2ResponseStatus> =>
  request({
    url: "/phd2/status/",
    method: "get",
  });

export const getPHD2Profile = (): Promise<IResponseDataAny> =>
  request({
    url: "/phd2/current_profile/",
    method: "get",
  });

export const postPHD2Start = (): Promise<IResponseFlagGeneral> =>
  request({
    url: "/phd2/start/",
    method: "post",
  });

export const postPHD2Stop = (): Promise<IResponseFlagGeneral> =>
  request({
    url: "/phd2/stop/",
    method: "post",
  });

export const postPHD2ConnectDevice = (): Promise<IResponseFlagGeneral> =>
  request({
    url: "/phd2/connect_device/",
    method: "post",
  });

export const postPHD2DisconnectDevice = (): Promise<IResponseFlagGeneral> =>
  request({
    url: "/phd2/disconnect_device/",
    method: "post",
  });
