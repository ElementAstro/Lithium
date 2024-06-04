import request from "@/services/request";

// ---------------PAA相关的HTTP接口
// 具体说明见后端的接口文档
export const getPAAStatus = () =>
  request({
    url: "/PAA/status/",
    method: "get",
  });

// written by gao
// 这个接口跟上面的区别就是，他会清空已经完成的paa，然后把清空结果给出来
// 然后true表示在运行。
export const getPAAUpdateStatus = (): Promise<IPAAFixedUpdateRunningStatus> =>
  request({
    url: "/PAA/update_paa_running_status/",
    method: "get",
  });

export const getCurrentScript = (): Promise<IPAAScripteResponse> =>
  request({
    url: "/PAA/get_current_script/",
    method: "get",
  });

export const getSavedScripts = (): Promise<IRequestResponse<any>> =>
  request({
    url: "/PAA/get_all_saved_scripts/",
    method: "get",
  });

export const postPAAStart = (): Promise<IRequestResponse<any>> =>
  request({
    url: "/PAA/start/",
    method: "post",
  });

export const postPAAStop = (): Promise<IRequestResponse<any>> =>
  request({
    url: "/PAA/stop/",
    method: "post",
  });

export const postPAAUpdate = (body: any): Promise<IRequestResponse<any>> =>
  request({
    url: "/PAA/update_script/",
    method: "post",
    data: body,
  });

export const postPAAGenerate = (body: {
  script_type: any;
  script_setting: any;
}): Promise<IPAAScripteResponse> =>
  request({
    url: "/PAA/generate_script/",
    method: "post",
    data: body,
  });

export const postPAALoadSavedScript = (body: {
  script_name: any;
}): Promise<IPAAScripteResponse> =>
  request({
    url: "/PAA/load_saved_script/",
    method: "post",
    data: body,
  });

export const postPAASaveScript = (body: {
  script_name: any;
}): Promise<IPAAScripteResponse> =>
  request({
    url: "/PAA/save_script/",
    method: "post",
    data: body,
  });

export const postPAADeleteScript = (body: { script_name: any }) =>
  request({
    url: "/PAA/delete_script/",
    method: "post",
    data: body,
  });
