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
} from "@/services/PAA";
import { Action, action, thunk, Thunk } from "easy-peasy";

// TODO very big one
// 这里要对store的功能做个总管理。整个paa执行数据都在store里，那么前段这里只做调用和获取store的信息

// interface step_info{
//     device: string,
//     id: number,
//     params: {
//         count?: number,
//         repeat?: number,
//     }
//     instruction: string,
//     children?: (step_info)[],
//     after_loop?: [],
//     name?: string,
//     ra?: number,
//     dec?: number,
// }

export interface PAAModel {
  // 变量
  setting_mode: number;

  // 脚本信息
  all_script_info: PAAScriptInfo[];
  // 内容信息
  all_step_data: PAABaseStepsInfo[];
  this_script_type: string;
  this_script_setting: any;

  // 目前在执行的脚本
  current_step: null | PAAlog_info;
  // 目前执行的脚本编号
  current_step_index: number;
  current_step_children_index: number;
  current_step_children_max_index: number;

  // 对应的script_name等信息
  current_script_info: PAAScriptInfo;

  all_logs_info: PAAlog_info[];

  // 所有Post请求
  post_start_PAA: Thunk<PAAModel>;
  post_stop_PAA: Thunk<PAAModel>;
  post_load_script: Thunk<PAAModel, PAAScriptInfo>;
  post_save_script: Thunk<PAAModel, any>;
  post_update_script: Thunk<PAAModel, any>;
  post_delete_script: Thunk<PAAModel, any>;
  post_generate_script: Thunk<PAAModel, any>;

  // 所有Get请求
  get_PAA_status: Thunk<PAAModel>;
  get_saved_scripts: Thunk<PAAModel>;
  get_current_script: Thunk<PAAModel>;

  setState: Action<PAAModel, Partial<PAAModel>>;

  add_paa_logs: Action<PAAModel, any>;
  change_paa_step: Action<PAAModel, any>;

  // modified by gao
  can_start_paa: boolean; // 表示是否具备启动paa的状态。
  paa_in_progress: boolean; // 表示是否有一个paa进程在运行

  // 如果有相机在单独拍摄，但是没有paa在运行，can start paa也会是false。也就是说，即使没有paa在运行，也存在可能没法启动paa。
  check_PAA_start_status: Thunk<PAAModel>; // 这个函数就是用来专门刷新上面两个边练改的。
  turn_on_PAA_in_progress: Action<PAAModel>;
  turn_off_PAA_in_progress: Action<PAAModel>;
  change_paa_loop_step: Action<PAAModel, any>;
  change_paa_step_async: Thunk<PAAModel, PAAlog_info>;
  // about paa step change
  current_step_id: string;
  paa_new_step_info_in: Action<PAAModel, any>;
  paa_new_log_info_in: Action<PAAModel, any>;
}

export const getPAAModel = (): PAAModel => ({
  setting_mode: 0,
  all_script_info: [],
  all_step_data: [],
  this_script_type: "",
  this_script_setting: {},
  current_script_info: {
    file_name: "",
    modified_time: "",
  },
  current_step: null,
  current_step_index: -1,
  current_step_children_index: -1,
  current_step_children_max_index: 0,

  all_logs_info: [],

  post_start_PAA: thunk(async (state, payload, { getState }) => {
    const res = await postPAAStart();
    state.setState({
      current_step_id: "",
    });
    if (res.success) {
      alert("PAA启动成功");
    } else {
      alert("PAA启动失败");
    }
  }),

  post_stop_PAA: thunk(async (state, payload, { getState }) => {
    const res = await postPAAStop();
    if (res.success) {
      console.log("PAA stop successfully");
      state.setState({
        current_step_index: -1,
        current_step: null,
        all_logs_info: [],
        can_start_paa: true,
        paa_in_progress: false,
      });
    }
  }),
  // todo modified need test
  post_delete_script: thunk(async (state, payload, { getState }) => {
    let props = {
      script_name: payload,
    };
    const res = await postPAADeleteScript(props);
    state.get_saved_scripts();
    return res;
  }),
  // todo modified need test
  post_generate_script: thunk(async (state, payload, { getState }) => {
    let props = {
      script_type: payload.name,
      script_setting: payload.data,
    };
    const res = await postPAAGenerate(props);
    // todo, 需要修改成当前脚本就是刚刚生成的
    // 记得后续修改回原来的版本, all_step_data = res.data.script
    state.setState({
      all_step_data: res.data.script,
      this_script_type: res.data.type,
      this_script_setting: res.data.setting,
    });
    return res;
  }),

  // 更换当前运行的脚本
  // todo modified need test
  post_update_script: thunk(async (state, payload, { getState }) => {
    // 把脚本替换成待运行的。这个理论上是不需要用的，在手机版上
    let props = {
      script: payload.data,
    };
    const res = await postPAAUpdate(props);
    if (res.success) {
      console.log("更换运行脚本成功");
      state.setState({
        all_step_data: res.data.script,
        this_script_type: res.data.type,
        this_script_setting: res.data.setting,
      });
    }
  }),
  // todo modifed need test
  post_save_script: thunk(async (state, payload, { getState }) => {
    let props = {
      script_name: payload,
    };
    const res = await postPAASaveScript(props);
    state.get_saved_scripts();
    state.setState({
      setting_mode: 1,
      current_script_info: {
        file_name: payload + ".json",
        modified_time: "Now",
      },
    });
  }),

  // 获得脚本名字对应的脚本内容
  // todo 大改逻辑
  // todo modified need test
  post_load_script: thunk(async (state, payload, { getState }) => {
    const props = {
      script_name: payload.file_name,
    };
    const res = await postPAALoadSavedScript(props);
    if (res.success) {
      state.setState({
        current_script_info: payload,
        setting_mode: 1,
        all_step_data: res.data.script,
        this_script_type: res.data.type,
        this_script_setting: res.data.setting,
      });
    }
  }),

  // 获得当前正在运行的脚本内容
  get_current_script: thunk(async (state, payload, { getState }) => {
    const res = await getCurrentScript();
    if (res.success) {
      if (res.data.script.length > 0) {
        state.setState({
          setting_mode: 1,
          all_step_data: res.data.script,
          this_script_type: res.data.type,
          this_script_setting: res.data.setting,
        });
      } else {
        state.setState({
          setting_mode: 0,
          all_step_data: [],
        });
      }
    } else {
      state.setState({
        setting_mode: 0,
      });
    }
  }),

  get_PAA_status: thunk(async (state, payload, { getState }) => {
    state.setState({
      can_start_paa: false,
    });
    const res = await getPAAStatus();
    if (res.data.flag) {
      state.setState({
        can_start_paa: true,
      });
    } else {
      state.setState({
        can_start_paa: false,
      });
    }
  }),

  // 获得所有脚本
  // todo, modified, need test.
  get_saved_scripts: thunk(async (state, payload, { getState }) => {
    const res = await getSavedScripts();
    if (res.success) {
      state.setState({
        all_script_info: res.data,
      });
      // todo， 这里不需要更新所有的脚本信息。无效的请求。
      // const promises = res.data.map((item: any) => {
      //     return state.post_load_script(item.file_name);
      // });

      // // Wait for all promises to resolve
      // const scriptInfos = await Promise.all(promises);

      // const updatedScriptInfo = res.data.map((item: any, index: number) => ({
      //     file_name: item.file_name,
      //     modified_time: item.modified_time,
      //     script_info: scriptInfos[index].data,
      // }));

      // state.setState({
      //     all_script_info: updatedScriptInfo,
      // });
    }
  }),

  add_paa_logs: action((state, payload) => {
    state.all_logs_info.unshift(payload);
    const regex = /!LOOP STEP! Loop step start (\d+)\/(\d+)/;
    const match = payload.message.match(regex);

    if (match && match.length === 3) {
      const current = parseInt(match[1], 10);
      // 获取 all_step_data 的副本
      const updatedAllStepData = [...state.all_step_data];

      // 获取要修改的项的索引
      const indexToModify = state.current_step_index;
      const subIndexToModify = state.current_step_children_index;

      // 确保索引在有效范围内
      if (indexToModify >= 0 && indexToModify < updatedAllStepData.length) {
        // 修改 count 值
        if (subIndexToModify != -1) {
          updatedAllStepData[indexToModify].children[
            subIndexToModify
          ].params.count = current;
        } else {
          updatedAllStepData[indexToModify].params.count = current;
        }
      }

      // 更新状态
      state.all_step_data = updatedAllStepData;
    }
  }),

  // 1st paa change
  change_paa_step: action((state, payload) => {
    state.current_step_index += 1;
    state.current_step_children_index = -1;
    state.current_step_children_max_index = payload.data.children
      ? payload.data.children.length
      : 0;
    state.current_step = payload;
  }),

  // 2nd paa change (children)
  change_paa_loop_step: action((state, payload) => {
    state.current_step_children_index += 1;
    state.current_step = payload;
  }),

  change_paa_step_async: thunk(async (state, payload, { getState }) => {
    if (
      getState().current_step_children_index + 1 ===
      getState().current_step_children_max_index
    ) {
      state.change_paa_step(payload);
    } else {
      state.change_paa_loop_step(payload);
    }
  }),

  setState: action((state, payload) => {
    state = Object.assign(state, payload);
  }),

  // modified by gao
  can_start_paa: true,
  paa_in_progress: false,
  check_PAA_start_status: thunk(async (state, payload, { getState }) => {
    state.setState({
      paa_in_progress: true,
      can_start_paa: false,
    });
    const res = await getPAAUpdateStatus();
    // true meas in running
    state.setState({
      paa_in_progress: res.data,
      can_start_paa: !res.data,
    });
    // todo, extra check camera status
  }),
  turn_on_PAA_in_progress: action((state) => {
    state.can_start_paa = false;
    state.paa_in_progress = true;
  }),
  turn_off_PAA_in_progress: action((state) => {
    state.can_start_paa = true;
    state.paa_in_progress = false;
  }),
  current_step_id: "",
  paa_new_step_info_in: action((state, payload) => {
    if (payload.message == "step_start")
      state.current_step_id = payload.data.id;
  }),
  paa_new_log_info_in: action((state, payload) => {
    state.all_logs_info.unshift(payload);
  }),
});
