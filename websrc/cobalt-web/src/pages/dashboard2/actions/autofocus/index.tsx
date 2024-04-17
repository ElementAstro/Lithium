import * as React from "react";
import { Card } from "react-bootstrap";
import { Button } from "react-bootstrap";
import { Play, Stop, Gear, QuestionCircle } from "react-bootstrap-icons";
import * as AXIOSPAAF from "../../../../services/paa_fixed_procedure_api";

import AutoFocusSettingDialog from "./AutoFocusSettingDialog";
import ConfirmDialog from "../light/ConfirmDialog";

import { useEchoWebSocket } from "../../../../utils/websocketProvider";
import { GlobalStore } from "../../../../store/globalStore";

const Autofocus = () => {
  //ui control
  const [can_start_paa, set_can_start_paa] = React.useState(false);
  const [show_setting, set_show_setting] = React.useState(0);
  const [manual_show, set_manual_show] = React.useState(0);
  const [dialog_open, set_dialog_open] = React.useState(0);
  const [show_af_curve, set_show_af_curve] = React.useState(false);
  // af data
  const [af_measure_data, set_af_measure_data] = React.useState<
    Array<[number, number]>
  >([]);
  const [af_model, set_af_model] = React.useState<
    null | [number, number, number]
  >(null);
  const [af_r_square, set_af_r_square] = React.useState<number>(0);
  // setting data control
  const [filter_index, set_filter_index] = React.useState<number | "current">(
    1
  );
  const [start_side, set_start_side] = React.useState(true);
  // store
  const console_state = GlobalStore.useAppState((state) => state.console);
  const set_console_state = GlobalStore.actions.console.setState;
  // function
  const process_ws_message = (msg: any): void => {
    if (msg.type == "error") {
      return;
    }
    if (msg.device_name == "PAA" && msg.instruction == "AutoFocus") {
      switch (msg.message) {
        case "initial": {
          set_console_state({
            alert_type: "success",
            alert_message: "初始化中",
          });
          break;
        }
        case "out side": {
          set_console_state({
            alert_type: "success",
            alert_message: "调焦器讲向外移动",
          });
          break;
        }
        case "in side": {
          set_console_state({
            alert_type: "success",
            alert_message: "调焦器讲向内移动",
          });
          break;
        }
        case "move focus": {
          set_console_state({
            alert_type: "success",
            alert_message: "调焦移动中",
          });
          break;
        }
        case "start exposure": {
          set_console_state({
            alert_type: "success",
            alert_message: "开始曝光",
          });
          break;
        }
        case "single exposure finished": {
          set_console_state({
            alert_type: "success",
            alert_message: "单次曝光完成",
          });
          break;
        }
        case "step exposure": {
          // important
          set_af_measure_data((af_measure_data) => [
            ...af_measure_data,
            [msg.data.step, msg.data.mean],
          ]);
          set_console_state({
            alert_type: "success",
            alert_message: "测量完成",
          });
          break;
        }
        case "no useful measure": {
          set_console_state({
            alert_type: "warning",
            alert_message: "没有测量到有效星点大小",
          });
          break;
        }
        case "first side finished!": {
          set_console_state({
            alert_type: "success",
            alert_message: "第一侧测量完成",
          });
          break;
        }
        case "first side failed": {
          set_console_state({
            alert_type: "danger",
            alert_message: "第一侧测量失败",
          });
          break;
        }
        case "other side init": {
          set_console_state({
            alert_type: "success",
            alert_message: "初始化第二侧测量",
          });
          break;
        }
        case "second side finished!": {
          set_console_state({
            alert_type: "success",
            alert_message: "第二侧测量完成",
          });
          break;
        }
        case "focus found": {
          set_console_state({
            alert_type: "success",
            alert_message: "找到焦点！",
          });
          set_af_model(msg.data.model);
          set_af_r_square(msg.data.r_square);
          break;
        }
        case "moveto focus": {
          set_console_state({
            alert_type: "success",
            alert_message: "移动至焦点",
          });
          break;
        }
        case "min focus not found": {
          set_console_state({
            alert_type: "danger",
            alert_message: "焦点没有找到！",
          });
          break;
        }
      }
    }
  };

  // websocket handler
  const { sendMessage, removeListener } = useEchoWebSocket(process_ws_message);

  const update_paa_status_and_clean = async () => {
    set_can_start_paa(false);
    try {
      let paa_running_status = await AXIOSPAAF.get_update_paa_running_status();
      if (paa_running_status.data) {
        // true means in running
        set_can_start_paa(false);
      } else {
        set_can_start_paa(true);
      }
    } catch (err) {
      return null;
    }
  };

  const post_stop_paa = async () => {
    try {
      let stop_result = await AXIOSPAAF.post_stop_paa();
      if (stop_result.success) {
        set_can_start_paa(true);
        update_paa_status_and_clean();
      } else {
        set_can_start_paa(true);
        console.log(stop_result.message);
      }
    } catch (err) {
      return null;
    }
  };
  const on_stop_paa_clicked = () => {
    set_dialog_open(dialog_open + 1);
  };
  const on_start_autofocus_clicked = async () => {
    set_af_model(null);
    try {
      let start_result = await AXIOSPAAF.start_fixed_autofocus({
        filter_index: 0,
        start_side: false,
      });
      if (start_result.success) {
        // successfully started
        set_can_start_paa(false);
      } else {
        console.log(start_result.message);
        get_paa_status_after_start();
      }
    } catch (err) {
      return null;
    }
  };
  const get_paa_status_after_start = async () => {
    set_can_start_paa(false);
    try {
      let paa_status_check = await AXIOSPAAF.get_paa_status();
      if (paa_status_check.data.flag) {
        // true meas in running
        set_can_start_paa(false);
      } else {
        set_can_start_paa(true);
      }
    } catch (err) {
      return null;
    }
  };
  // useEffect
  React.useEffect(() => {
    // open setting tab to confirm
    set_show_setting(show_setting + 1);
    // check if there is paa in progres
    update_paa_status_and_clean();
    return () => {
      removeListener(process_ws_message);
    };
  }, []);

  const on_setting_closed_update = (
    filter_index: "current" | number,
    start_side: boolean
  ) => {
    set_filter_index(filter_index);
    set_start_side(start_side);
  };

  return (
    <>
      <Card style={{ zIndex: 20 }}>
        <Card.Body>
          <div className="d-flex gap-2 flex-wrap">
            {!can_start_paa ? (
              <Button variant="danger" onClick={on_stop_paa_clicked}>
                <Stop /> 停止PAA流程
              </Button>
            ) : (
              <Button variant="success" onClick={on_start_autofocus_clicked}>
                <Play /> 开始自动对焦
              </Button>
            )}
            <Button
              variant="primary"
              onClick={() => {
                set_show_setting(show_setting + 1);
              }}
            >
              <Gear /> 设置
            </Button>
            <Button
              variant="secondary"
              onClick={() => {
                set_manual_show(manual_show + 1);
              }}
            >
              <QuestionCircle />
            </Button>
          </div>
        </Card.Body>
      </Card>

      <AutoFocusSettingDialog
        open_dialog={show_setting}
        filter_index={filter_index}
        start_side={start_side}
        on_update_data={on_setting_closed_update}
      />
      <ConfirmDialog
        open={dialog_open}
        show_text={"请确认是否立即终止现在正在执行的PAA流程"}
        show_title={"确认终止流程"}
        on_confirm_clicked={post_stop_paa}
        on_cancel_clicked={() => {}}
      />
    </>
  );
};

Autofocus.displayName = "AF";

export default Autofocus;
