import request from './request';

// ---------------PAA相关的HTTP接口
// 具体说明见后端的接口文档
export const getPAAStatus = () =>request({
  url: '/PAA/status/',
  method: 'get'
});

export const getCurrentScript = (): Promise<IRequestResponse<any>> =>request({
  url: '/PAA/get_current_script/',
  method: 'get'
});

export const getSavedScripts = (): Promise<IRequestResponse<any>> =>request({
  url: '/PAA/get_all_saved_scripts/',
  method: 'get'
});

export const postPAAStart = () => request({
  url: '/PAA/start/',
  method: 'post'
});
  
export const postPAAStop = () => request({
  url: '/PAA/stop/',
  method: 'post'
});

export const postPAAUpdate = (body: any): Promise<IRequestResponse<any>> =>request({
  url: '/PAA/update_script/',
  method: 'post',
  data: body
});

export const postPAAGenerate = (body: {script_type: any, script_setting: any}) =>request({
  url: '/PAA/generate_script/',
  method: 'post',
  data: body
});

export const postPAALoadSavedScript = (body: {script_name: any}): Promise<IRequestResponse<any>> =>request({
  url: '/PAA/load_saved_script/',
  method: 'post',
  data: body
});

export const postPAASaveScript = (body: {script_name: any}): Promise<IRequestResponse<any>> =>request({
  url: '/PAA/save_script/',
  method: 'post',
  data: body
});

export const postPAADeleteScript = (body: {script_name: any}) =>request({
  url: '/PAA/delete_script/',
  method: 'post',
  data: body
});
