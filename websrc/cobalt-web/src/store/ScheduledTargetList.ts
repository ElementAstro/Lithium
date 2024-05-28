import { createStore, action, thunk, computed } from "easy-peasy";
import { Thunk, Action, Computed } from "easy-peasy";
import { DateTime } from "luxon";
import * as AXIOSOF from "@/services/object_finding_api";

interface update_string {
  index: number;
  update_string: string;
}

const all_available_tags = [
  "星云",
  "星系",
  "黑白目标",
  "彩机目标",
  "LRGB",
  "HSO",
  "双窄",
];

function isFramingObject(
  target_info: IDSOFramingObjectInfo | IDSOObjectInfo
): target_info is IDSOFramingObjectInfo {
  return "rotation" in target_info;
}

export interface ISTargetListModels {
  current_focus_target: IDSOFramingObjectInfo;
  current_focus_index: number | null;
  need_focus: boolean;
  all_tags: Array<string>;
  all_saved_targets: Array<IDSOFramingObjectInfo>;
  all_searched_targets: Array<IDSOObjectDetailedInfo>;
  twilight_data: ITwilightData;
  //--------------------------------
  change_searched_target_info: Action<
    ISTargetListModels,
    { index: number; newValue: IDSOObjectDetailedInfo }
  >;
  change_saved_target_info: Action<
    ISTargetListModels,
    { index: number; newValue: IDSOObjectDetailedInfo }
  >;
  add_target_and_focus: Action<ISTargetListModels, IDSOFramingObjectInfo>;
  change_focus_target: Action<ISTargetListModels, IDSOFramingObjectInfo>;
  change_saved_focus_target: Action<ISTargetListModels, number>;
  update_focus_target_by_framing: Action<
    ISTargetListModels,
    IDSOFramingObjectInfo
  >;
  remove_target: Action<ISTargetListModels, number>;
  target_rename: Action<ISTargetListModels, update_string>;
  target_set_flag: Action<ISTargetListModels, update_string>;
  target_set_tag: Action<ISTargetListModels, update_string>;
  clear_focus_target: Action<ISTargetListModels>;
  check_one_target: Action<ISTargetListModels, number>;
  clear_all_checked: Action<ISTargetListModels>;
  fetch_twilight_data: Thunk<ISTargetListModels>;
  update_twilight_data: Action<ISTargetListModels, ITwilightDataString>;
  // get_targets_by_flag: Action<ISTargetListModels, string>;
  // get_targets_by_tag: Action<ISTargetListModels, string>;
  // load_user_local_data
  save_all_targets: Action<ISTargetListModels>;
  load_all_targets: Action<ISTargetListModels>;

  now_load_target_index: number;
  search_word: string;

  setState: Action<ISTargetListModels, Partial<ISTargetListModels>>;
}

// todo, load user local saved data.
export const ScheduledTargetListStore = (): ISTargetListModels => ({
  current_focus_target: {
    name: "",
    ra: 0,
    dec: 0,
    rotation: undefined,
    flag: "",
    tag: "",
    target_type: "",
    size: undefined,
    checked: false,
  },
  current_focus_index: null,
  need_focus: false,
  all_saved_targets: [],
  all_searched_targets: [],
  all_tags: all_available_tags,
  now_load_target_index: 0,
  search_word: "NGC6888",
  twilight_data: {
    evening: {
      sun_set_time: new Date(2023, 1, 1, 18, 0, 0),
      evening_civil_time: new Date(2023, 1, 1, 18, 0, 0),
      evening_nautical_time: new Date(2023, 1, 1, 18, 0, 0),
      evening_astro_time: new Date(2023, 1, 1, 18, 0, 0),
    },
    morning: {
      sun_rise_time: new Date(2023, 1, 1, 18, 0, 0),
      morning_civil_time: new Date(2023, 1, 1, 18, 0, 0),
      morning_nautical_time: new Date(2023, 1, 1, 18, 0, 0),
      morning_astro_time: new Date(2023, 1, 1, 18, 0, 0),
    },
  },
  change_searched_target_info: action((state, payload) => {
    const { index, newValue } = payload;
    state.all_searched_targets[index] = newValue;
  }),
  change_saved_target_info: action((state, payload) => {
    const { index, newValue } = payload;
    state.all_saved_targets[index].altitude = newValue.altitude;
  }),
  add_target_and_focus: action((state, payload) => {
    state.all_saved_targets.push(payload);
    state.current_focus_target = payload;
    state.current_focus_index = state.all_saved_targets.length - 1;
    state.need_focus = true;
  }),
  change_focus_target: action((state, payload) => {
    state.current_focus_target = payload;
    state.current_focus_index = null;
    state.need_focus = true;
  }),
  change_saved_focus_target: action((state, payload) => {
    state.current_focus_target = state.all_saved_targets[payload];
    state.current_focus_index = payload;
    state.need_focus = true;
  }),
  update_focus_target_by_framing: action((state, payload) => {
    // note do not use, it is not tested and verified.
    state.current_focus_target = payload;
    state.need_focus = true;
  }),
  remove_target: action((state, payload) => {
    if (payload < state.all_saved_targets.length) {
      state.all_saved_targets.splice(payload, 1);
      state.current_focus_index = null;
    }
  }),
  target_rename: action((state, payload) => {
    // state.all_saved_targets.map((one_target, index) => {
    //     if (index == payload.index){
    //         one_target.name = payload.update_string;
    //     }
    // })
    if (payload.index < state.all_saved_targets.length) {
      state.all_saved_targets[payload.index].name = payload.update_string;
    }
  }),
  target_set_flag: action((state, payload) => {
    // state.all_saved_targets.map((one_target, index) => {
    //     if (index == payload.index){
    //         one_target.flag = payload.update_string;
    //     }
    // })
    if (payload.index < state.all_saved_targets.length) {
      state.all_saved_targets[payload.index].flag = payload.update_string;
    }
  }),
  target_set_tag: action((state, payload) => {
    // if (payload.update_string in all_available_tags){
    //     state.all_saved_targets.map((one_target, index) => {
    //         if (index == payload.index){
    //             one_target.tag = payload.update_string;
    //         }
    //     })
    // }
    if (payload.update_string in all_available_tags) {
      if (payload.index < state.all_saved_targets.length) {
        state.all_saved_targets[payload.index].tag = payload.update_string;
      }
    }
  }),
  clear_focus_target: action((state) => {
    state.current_focus_target = {
      name: "-",
      ra: 0,
      dec: 0,
      rotation: undefined,
      flag: "",
      tag: "",
      target_type: "",
      size: undefined,
      checked: false,
    };
    state.current_focus_index = null;
  }),
  check_one_target: action((state, payload) => {
    state.all_saved_targets[payload].checked = true;
  }),
  clear_all_checked: action((state) => {
    for (let i = 0; i < state.all_saved_targets.length; i++) {
      state.all_saved_targets[i].checked = false;
    }
  }),
  fetch_twilight_data: thunk(async (actions) => {
    let res_data = await AXIOSOF.getTwilightData();
    if (res_data.success) {
      actions.update_twilight_data(res_data.data);
    }
  }),
  update_twilight_data: action((state, payload) => {
    state.twilight_data = {
      evening: {
        sun_set_time: DateTime.fromFormat(
          payload.evening.sun_set_time,
          "yyyy-MM-dd HH:mm:ss"
        ).toJSDate(),
        evening_civil_time: DateTime.fromFormat(
          payload.evening.evening_civil_time,
          "yyyy-MM-dd HH:mm:ss"
        ).toJSDate(),
        evening_nautical_time: DateTime.fromFormat(
          payload.evening.evening_nautical_time,
          "yyyy-MM-dd HH:mm:ss"
        ).toJSDate(),
        evening_astro_time: DateTime.fromFormat(
          payload.evening.evening_astro_time,
          "yyyy-MM-dd HH:mm:ss"
        ).toJSDate(),
      },
      morning: {
        sun_rise_time: DateTime.fromFormat(
          payload.morning.sun_rise_time,
          "yyyy-MM-dd HH:mm:ss"
        ).toJSDate(),
        morning_civil_time: DateTime.fromFormat(
          payload.morning.morning_civil_time,
          "yyyy-MM-dd HH:mm:ss"
        ).toJSDate(),
        morning_nautical_time: DateTime.fromFormat(
          payload.morning.morning_nautical_time,
          "yyyy-MM-dd HH:mm:ss"
        ).toJSDate(),
        morning_astro_time: DateTime.fromFormat(
          payload.morning.morning_astro_time,
          "yyyy-MM-dd HH:mm:ss"
        ).toJSDate(),
      },
    };
  }),
  // get_targets_by_flag: action((state, payload) => {
  //     let result = [];
  //     for(let i=0;i<state.all_saved_targets.length;i++){
  //         if (payload == state.all_saved_targets[i].flag){
  //             result.push(state.all_saved_targets[i]);
  //         }
  //     }
  //     return result;
  // }),
  // get_targets_by_tag: action((state, payload) => {

  // }),
  save_all_targets: action((state) => {
    let to_save_array = state.all_saved_targets.map((obj) => ({
      name: obj.name,
      ra: obj.ra,
      dec: obj.dec,
      rotation: obj.rotation,
      flag: obj.flag,
      tag: obj.tag,
      target_type: obj.target_type,
      size: obj.size,
      checked: false,
    }));
    // console.log('saving', to_save_array);
    localStorage.setItem("TargetList", JSON.stringify(to_save_array));
  }),
  load_all_targets: action((state) => {
    let data = localStorage.getItem("TargetList");
    if (data != null) {
      let list_data = JSON.parse(data) as IDSOFramingObjectInfo[];
      // console.log('loaded list', list_data);
      state.all_saved_targets = list_data;
    }
  }),

  setState: action((state, payload) => {
    state = Object.assign(state, payload);
  }),
});
