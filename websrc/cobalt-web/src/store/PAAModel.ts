import {
  getPAAStatus,
  getCurrentScript,
  getSavedScripts,
  postPAAStart,
  postPAAStop,
  postPAADeleteScript,
  postPAAGenerate,
  postPAALoadSavedScript,
  postPAASaveScript,
  postPAAUpdate,
  getPAAUpdateStatus,
} from "../services/PAA";
import { Action, action, thunk, Thunk } from "easy-peasy";

interface script_info {
  file_name: string;
  modified_time: string;
}
export interface PAAModel {
  // 变量
  setting_mode: number;
  get_object: boolean;

  // 脚本信息
  all_script_info: (never | script_info)[];

  // 内容信息
  current_script_data: [];
  // 对应的script_name等信息
  current_script_info: script_info;

  // 所有Post请求
  post_start_PAA: Thunk<PAAModel>;
  post_stop_PAA: Thunk<PAAModel>;
  post_load_script: Thunk<PAAModel, any>;
  post_save_script: Thunk<PAAModel, any>;
  post_update_script: Thunk<PAAModel, any>;
  post_delete_script: Thunk<PAAModel, any>;
  post_generate_script: Thunk<PAAModel, any>;

  // 所有Get请求
  get_PAA_status: Thunk<PAAModel>;
  get_saved_scripts: Thunk<PAAModel>;
  get_current_script: Thunk<PAAModel>;

  setState: Action<PAAModel, Partial<PAAModel>>;
}

export const getPAAModel = (): PAAModel => ({
  setting_mode: 0,
  get_object: false,
  all_script_info: [],
  current_script_data: [],
  current_script_info: {
    file_name: "",
    modified_time: "",
  },

  post_start_PAA: thunk(async (state, payload, { getState }) => {
    const res = await postPAAStart();
    console.log(res);
  }),

  post_stop_PAA: thunk(async (state, payload, { getState }) => {
    const res = await postPAAStop();
    console.log(res);
  }),

  post_delete_script: thunk(async (state, payload, { getState }) => {
    let props = {
      script_name: payload,
    };
    const res = await postPAADeleteScript(props);
    return res;
  }),

  post_generate_script: thunk(async (state, payload, { getState }) => {
    let props = {
      script_type: "dark",
      script_setting: payload,
    };
    const res = await postPAAGenerate(props);
    console.log(res);
  }),

  // 更换当前运行的脚本
  post_update_script: thunk(async (state, payload, { getState }) => {
    let props = {
      script: payload.data,
    };
    const res = await postPAAUpdate(props);
    if (res.success) {
      alert("success");
      state.setState({
        setting_mode: 2,
        current_script_info: getState().all_script_info.find(
          (item) => item.file_name == payload.file_name
        ),
      });
    }
  }),

  post_save_script: thunk(async (state, payload, { getState }) => {
    let props = {
      script_name: payload,
    };
    const res = await postPAASaveScript(props);
  }),

  // 获得脚本名字对应的脚本内容
  post_load_script: thunk(async (state, payload, { getState }) => {
    const props = {
      script_name: payload,
    };
    const res = await postPAALoadSavedScript(props);
    return res;
  }),

  // 获得当前正在运行的脚本内容
  get_current_script: thunk(async (state, payload, { getState }) => {
    const res = await getCurrentScript();
    // 如果current_script不在这个队列中怎么办?
    state.setState({
      current_script_data: res.data,
    });
  }),

  get_PAA_status: thunk(async (state, payload, { getState }) => {
    const res = await getPAAStatus();
    console.log(res);
  }),

  // 获得所有脚本
  get_saved_scripts: thunk(async (state, payload, { getState }) => {
    const res = await getSavedScripts();
    if (res.success) {
      const promises = res.data.map((item: any) => {
        return state.post_load_script(item.file_name);
      });

      // Wait for all promises to resolve
      const scriptInfos = await Promise.all(promises);

      const updatedScriptInfo = res.data.map((item: any, index: number) => ({
        file_name: item.file_name,
        modified_time: item.modified_time,
        script_info: scriptInfos[index].data,
      }));

      state.setState({
        all_script_info: updatedScriptInfo,
      });
    }
  }),

  setState: action((state, payload) => {
    state = Object.assign(state, payload);
  }),
});
