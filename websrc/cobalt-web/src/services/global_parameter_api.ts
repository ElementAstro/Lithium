import request from "../services/request";
import { AxiosResponse } from "axios";

export const getGlobalAllParameters = (): Promise<IGPResponse> =>
  request({
    url: "/GlobalParameter/?name=all",
    method: "get",
  });

export const getGlobalOneParameter = (
  param_name: string
): Promise<IGPResponse> =>
  request({
    url: `/GlobalParameter/?name=${param_name}`,
    method: "get",
  });

export const postGlobalLoadProfile = (
  profile_name: string
): Promise<IGPResponse> =>
  request({
    url: "/GlobalParameter/",
    method: "post",
    data: {
      method: "load",
      data: {
        name: profile_name,
      },
    },
  });

export const postGLobalChangeParameter = (
  payload: IGPSetParameterAPI
): Promise<IGPResponse> =>
  request({
    url: "/GlobalParameter/",
    method: "post",
    data: {
      method: "set",
      data: {
        name: payload.parameter_name,
        setting: payload.to_set_parameter,
      },
    },
  });

export const postGLobalParameterOnStart = (): Promise<IGPPorfileList> =>
  request({
    url: "/GlobalParameter/",
    method: "post",
    data: {
      method: "on_start",
    },
  });

export const postGLobalParameterProfileDelete = (
  payload: string
): Promise<IGPPorfileList> =>
  request({
    url: "/GlobalParameter/",
    method: "post",
    data: {
      method: "delete",
      data: {
        name: payload,
      },
    },
  });
