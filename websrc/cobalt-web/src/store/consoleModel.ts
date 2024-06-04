import { Action, action, Thunk, thunk } from "easy-peasy";
import * as AXIOSPHD2 from "@/services/phd2_api";

interface ShootInfo {
  exposure_time: number;
}

export interface ConsoleModel {
  menu:
    | "star"
    | "shoot"
    | "align"
    | "autofocus"
    | "camera"
    | "telescope"
    | "guider"
    | "phd2"
    | "focus"
    | "filter"
    | "sync"
    | "paa";
  drawerVisible: boolean;
  setState: Action<ConsoleModel, Partial<ConsoleModel>>;
  // by gao
  alert_type: string;
  alert_message: string;
  selec_console_drawer_open: boolean;
  phd2_process_status: boolean;
  phd2_working_status: string;

  update_phd2_process_status: Thunk<ConsoleModel>;

  // by sun
  shoot_info: ShootInfo;
  menuItems: "shoot" | "align";
  leftMenu: "display";
  rightMenu:
    | "shoot"
    | "align"
    | "autofocus"
    | "camera"
    | "telescope"
    | "guider"
    | "phd2"
    | "focus"
    | "filter"
    | "sync"
    | "paa";
}

export const getConsoleModel = (): ConsoleModel => ({
  menu: "star",
  drawerVisible: true,
  setState: action((state, payload) => {
    state = Object.assign(state, payload);
  }),

  // by gao
  alert_type: "neutral",
  alert_message: "待机",
  selec_console_drawer_open: false,
  phd2_working_status: "状态未更新",
  phd2_process_status: false,
  update_phd2_process_status: thunk(async (actions) => {
    try {
      const response = await AXIOSPHD2.getPHD2Process();
      actions.setState({
        phd2_process_status: response.data.flag,
      });
    } catch (err) {}
  }),

  // by sun
  shoot_info: {
    exposure_time: 1,
  },
  leftMenu: "display",
  rightMenu: "shoot",
  menuItems: "shoot",
});
