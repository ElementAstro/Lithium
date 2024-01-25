import { createStore, action, thunk, computed } from "easy-peasy";
import { Thunk, Action, Computed } from "easy-peasy";
import * as AXIOSPAAF from "../services/paa_fixed_procedure_api";

export interface IProcessDataSaveModels {
  // process_ws_message: (message: any) => void;
  //
  // newest_camera_jpg: string |null;
  camera_jpg_updated: number;
  newest_camera_jpg_url: string;
  guider_jpg_updates: number;
  // newest_guider_jpg:  string |null;
  newest_guider_jpg_url: string;
  show_camera: number; // 0 means camera, 1 means guider
  HFR_points_list: ICHFRDataPointList;
  newest_HFR_info: ICSingleHFRPointData;
  PHD2_guide_data_list: ICPHD2GuideDataPointList;
  send_ws_message_handler: ((message: any) => void) | null;
  registered: Computed<IProcessDataSaveModels, boolean>;
  at_start_add_ws_listener: Action<
    IProcessDataSaveModels,
    (message: any) => void
  >;
  send_message_to_ws: Action<IProcessDataSaveModels, any>;
  fetch_history_HFR_data: Thunk<IProcessDataSaveModels>;
  append_newest_history_HFR_data: Action<
    IProcessDataSaveModels,
    ICSingleHFRPointData
  >;
  fetch_history_PHD2_guide_data: Thunk<IProcessDataSaveModels>;
  append_newest_PHD2_guide_data: Action<
    IProcessDataSaveModels,
    ICPHD2InterfaceGuideStep
  >;
  // update_jpg_data: Action<IProcessDataSaveModels, {device: string, data: Blob}>;
  // process_ws_blob: Thunk<IProcessDataSaveModels, Blob>;
  update_jpg_data: Action<
    IProcessDataSaveModels,
    { device: string; data: string }
  >;
  get_newest_jpg: Thunk<IProcessDataSaveModels, string>;
  switch_camera_display: Action<IProcessDataSaveModels, number>;
}

export const ProcessDataSaveModel = (): IProcessDataSaveModels => ({
  // newest_camera_jpg: null,
  newest_camera_jpg_url: "",
  // newest_guider_jpg: null,
  newest_guider_jpg_url: "",
  camera_jpg_updated: 0,
  guider_jpg_updates: 0,
  show_camera: 0, // 0 means camera, 1 means guider
  HFR_points_list: {
    star_count: [],
    average_hfr: [],
  },
  newest_HFR_info: {
    star_count: 0,
    average_hfr: 0,
    max_star: 0,
    min_star: 0,
    average_star: 0,
  },
  PHD2_guide_data_list: {
    dx: [],
    dy: [],
    RaDistance: [],
    DecDistance: [],
    RaControl: [],
    DecControl: [],
  },
  send_ws_message_handler: null,
  registered: computed((state) => {
    if (state.send_ws_message_handler != null) {
      return true;
    } else {
      return false;
    }
  }),
  at_start_add_ws_listener: action((state, payload) => {
    // const {sendMessage, removeListener} = useEchoWebSocket(process_ws_message);
    state.send_ws_message_handler = payload;
    // send_ws_message_handler = sendMessage;
  }),
  send_message_to_ws: action((state, payload) => {
    // deprecated
    if (state.send_ws_message_handler != null) {
      state.send_ws_message_handler(JSON.stringify(payload));
    }
  }),
  fetch_history_HFR_data: thunk(async (actions) => {}),
  append_newest_history_HFR_data: action((state, payload) => {
    state.HFR_points_list.average_hfr.push(payload.average_hfr);
    state.HFR_points_list.star_count.push(payload.star_count);
    state.newest_HFR_info = payload;
  }),
  fetch_history_PHD2_guide_data: thunk(async (actions) => {}),
  append_newest_PHD2_guide_data: action((state, payload) => {
    state.PHD2_guide_data_list.dx.push(payload.dx);
    state.PHD2_guide_data_list.dy.push(payload.dy);
    state.PHD2_guide_data_list.RaDistance.push(payload.RADistanceRaw);
    state.PHD2_guide_data_list.DecDistance.push(payload.DECDistanceRaw);
    if (payload.RADirection == "East") {
      state.PHD2_guide_data_list.RaControl.push(payload.RADuration);
    } else {
      state.PHD2_guide_data_list.RaControl.push(-payload.RADuration);
    }
    if (payload.DECDirection == "North") {
      state.PHD2_guide_data_list.DecControl.push(payload.DECDuration);
    } else {
      state.PHD2_guide_data_list.DecControl.push(-payload.DECDuration);
    }
  }),
  // update_jpg_data: action((state, payload) => {
  //   if (payload.device.includes('camera')){
  //     state.newest_camera_jpg = payload.data;
  //   }else if (payload.device.includes('guider')){
  //     state.newest_guider_jpg = payload.data;
  //   }
  // }),
  // process_ws_blob: thunk(async(actions, payload) => {
  //   let data_type = payload.slice(0, 10);
  //   let jpg_data = payload.slice(10);
  //   let data_type_str = await data_type.text();
  //   console.log('in getting bolb', data_type_str, jpg_data);
  //   actions.update_jpg_data({device: data_type_str, data: jpg_data});
  // }),
  get_newest_jpg: thunk(async (actions, payload) => {
    if (payload == "camera") {
      try {
        let newest_url = "/PAA/newest_camera_jpg/?" + new Date().getTime();
        actions.update_jpg_data({ device: "camera", data: newest_url });
      } catch (e) {}
    } else if (payload == "guider") {
      try {
        let newest_url = "/PAA/newest_guider_jpg/?" + new Date().getTime();
        actions.update_jpg_data({ device: "guider", data: newest_url });
      } catch (e) {}
    }
  }),
  update_jpg_data: action((state, payload) => {
    if (payload.device.includes("camera")) {
      state.newest_camera_jpg_url = payload.data;
      state.camera_jpg_updated = state.camera_jpg_updated + 1;
      console.log("before generating url", typeof payload.data);
      // let new_url = URL.createObjectURL(payload.data);
      // console.log(new_url);
      // if (state.newest_camera_jpg_url !== null){
      //   URL.revokeObjectURL(state.newest_camera_jpg_url);
      // }
      // state.newest_camera_jpg_url = new_url;
    } else if (payload.device.includes("guider")) {
      state.newest_guider_jpg_url = payload.data;
      state.guider_jpg_updates = state.guider_jpg_updates + 1;
      // let new_url = URL.createObjectURL(payload.data);
      // if (state.newest_guider_jpg_url !== null){
      //   URL.revokeObjectURL(state.newest_guider_jpg_url);
      // }
      // state.newest_guider_jpg_url = new_url;
    }
  }),
  switch_camera_display: action((state, payload) => {
    if (payload != 0 && payload != 1) {
      payload = 0;
    }
    state.show_camera = payload;
  }),
});
