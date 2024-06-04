import { createStore, action, thunk, computed } from "easy-peasy";
import * as AXIOSGP from "@/services/global_parameter_api";
import { assignDeep } from "./persistence";
import { Thunk, Action, Computed } from "easy-peasy";

export interface IGLobalParametersModels {
  global_parameter: IGlobalParameters;
  tab_value: number;
  // global_changed: IGPChanged;
  update_one_paramter: Action<
    IGLobalParametersModels,
    Partial<IGlobalParameters>
  >;
  get_all_paramters: Thunk<IGLobalParametersModels>;
  get_parameter: Thunk<IGLobalParametersModels, string>;
  ask_load_profile_parameter: Thunk<IGLobalParametersModels, string>;
  set_parameter: Thunk<IGLobalParametersModels, IGPSetParameterAPI>;
  get_filter_names_full: Computed<
    IGLobalParametersModels,
    IGPFilterSelection[]
  >;
  get_filter_names_simple: Computed<
    IGLobalParametersModels,
    IGPFilterSelection[]
  >;
  set_tab_value: Action<IGLobalParametersModels, number>;

  // for phone side local storage
  ui_setting: IGPManualHelpSetting;
  initial_ui_setting: Action<IGLobalParametersModels>;
  load_ui_setting: Action<IGLobalParametersModels>;
  save_ui_setting: Action<IGLobalParametersModels>;
  set_ui_setting: Action<
    IGLobalParametersModels,
    Partial<IGPManualHelpSetting>
  >;

  guide_pixel_multi: Computed<IGLobalParametersModels, number>;
}

export const GlobalParameterStore = (): IGLobalParametersModels => ({
  global_parameter: {
    geo_location: null,
    meridian_flip: null,
    guider_dither_settle: null,
    guider_start_guide_settle: null,
    filter_setting: null,
    autofocus: null,
    plate_solve: null,
    telescope_info: null,
    info_get: { camera: false, guider_camera: false },
    camera_info: null,
    guider_camera_info: null,
  },
  tab_value: 1,
  set_tab_value: action((state, payload) => {
    state.tab_value = payload;
  }),
  // global_changed: {
  //   geo_location: false,
  //   meridian_flip: false,
  //   filter_setting: false,
  //   autofocus: false,
  //   plate_solve: false,
  //   telescope_info: false,
  //   camera_info: false,
  //   guider_camera_info: false,
  // },
  update_one_paramter: action((state, payload) => {
    console.log("in update", payload);
    state.global_parameter = Object.assign({}, state.global_parameter, {
      ...payload,
    });
  }),
  get_all_paramters: thunk(async (actions) => {
    try {
      const response = await AXIOSGP.getGlobalAllParameters();
      actions.update_one_paramter(response.data);
    } catch (err) {
      console.error("Error fetching data:", err);
    }
  }),
  get_parameter: thunk(async (actions, paramter_name: string) => {
    try {
      const response = await AXIOSGP.getGlobalOneParameter(paramter_name);
      let new_setting = Object();
      new_setting[paramter_name] = response.data;
      // console.log(new_setting);
      actions.update_one_paramter(new_setting);
    } catch (err) {
      console.error("Error fetching data:", err);
    }
  }),
  // not modified yet
  ask_load_profile_parameter: thunk(async (actions, payload) => {
    try {
      const response = await AXIOSGP.postGlobalLoadProfile(payload);
    } catch (err) {
      console.error("Error in load profile:", err);
    }
  }),
  set_parameter: thunk(async (actions, payload) => {
    try {
      const response = await AXIOSGP.postGLobalChangeParameter(payload);
      console.log("set finished");
      const update_res = await AXIOSGP.getGlobalOneParameter(
        payload.parameter_name
      );
      let new_setting = Object();
      new_setting[payload.parameter_name] = update_res.data;
      // console.log(new_setting);
      actions.update_one_paramter(new_setting);
    } catch (err) {
      console.error("Error in set parameter:", err);
    }
  }),
  get_filter_names_full: computed((state) => {
    let ret_struct: IGPFilterSelection[] = [];
    if (state.global_parameter.filter_setting != null) {
      for (
        let i = 0;
        i < state.global_parameter.filter_setting.filter_info.length;
        i++
      ) {
        ret_struct.push({
          label:
            state.global_parameter.filter_setting.filter_info[i].filter_name,
          value: i,
        });
      }
      return ret_struct;
    } else {
      return ret_struct;
    }
  }),
  get_filter_names_simple: computed((state) => {
    let ret_struct: IGPFilterSelection[] = [];
    if (state.global_parameter.filter_setting != null) {
      for (
        let i = 0;
        i < state.global_parameter.filter_setting.filter_info.length;
        i++
      ) {
        ret_struct.push({
          label:
            state.global_parameter.filter_setting.filter_info[
              i
            ].filter_name.charAt(0),
          value: i,
        });
      }
      return ret_struct;
    } else {
      return ret_struct;
    }
  }),

  // ui setting part
  ui_setting: {
    show_three_point_help: true,
  },
  initial_ui_setting: action((state) => {
    state.ui_setting = {
      show_three_point_help: true,
    };
  }),
  load_ui_setting: action((state) => {
    let loaded_ui_setting_string = localStorage.getItem("UISetting");
    if (loaded_ui_setting_string != null) {
      let loaded_ui_setting = JSON.parse(
        loaded_ui_setting_string
      ) as Partial<IGPManualHelpSetting>;
      state.ui_setting = Object.assign({}, state.ui_setting, {
        ...loaded_ui_setting,
      });
    }
  }),
  save_ui_setting: action((state) => {
    localStorage.setItem("UISetting", JSON.stringify(state.ui_setting));
  }),
  set_ui_setting: action((state, payload) => {
    state.ui_setting = Object.assign({}, state.ui_setting, { ...payload });
    localStorage.setItem("UISetting", JSON.stringify(state.ui_setting));
  }),
  guide_pixel_multi: computed((state) => {
    if (
      state.global_parameter.telescope_info?.guider_focal_length &&
      state.global_parameter.guider_camera_info?.CCD_PIXEL_SIZE
    ) {
      return (
        (state.global_parameter.guider_camera_info?.CCD_PIXEL_SIZE /
          state.global_parameter.telescope_info?.guider_focal_length) *
        206.265
      );
    } else {
      return 1;
    }
  }),
});
