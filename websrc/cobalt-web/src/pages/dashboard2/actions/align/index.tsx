import * as React from "react";
import { Button } from "react-bootstrap";
import { Card } from "react-bootstrap";
import { Row, Col } from "react-bootstrap";
import { Play, Stop, Gear, QuestionCircle } from "react-bootstrap-icons";
import Alert from "react-bootstrap/Alert";
import * as AXIOSPAAF from "../../../../services/paa_fixed_procedure_api";

import ThreePointStepper from "./ThreePointStepper";
import ConfirmDialog from "../light/ConfirmDialog";

import ThreePointSettingDialog from "./ThreePointSettings";
import ThreePointManualDialog from "./ThreePointManualDialog";
import { useEchoWebSocket } from "../../../../utils/websocketProvider";
import { GlobalStore } from "../../../../store/globalStore";

const Align: React.FC = () => {
  //ui control
  const [can_start_paa, set_can_start_paa] = React.useState(false);
  const [dialog_open, set_dialog_open] = React.useState(0);

  const [show_setting, set_show_setting] = React.useState(0);
  const [manual_show, set_manual_show] = React.useState(0);
  // const [alert_color, set_alert_color] = React.useState<ColorPaletteProp>('success');
  // const [alert_text, set_alert_text] = React.useState('待机');

  // progress data
  const [step_number, set_step_number] = React.useState(0);
  const [incoming_data, set_incoming_data] = React.useState<any>({});
  const [direction_switch, set_direction_switch] = React.useState(false);
  const [move_time, set_move_time] = React.useState(10);
  const [solve_retry_time, set_solve_retry_time] = React.useState(3);
  const [manual_start, set_manual_start] = React.useState(false);
  const [search_radius, set_search_radius] = React.useState(30);
  // store
  const console_state = GlobalStore.useAppState((state) => state.console);
  const global_parameter_ui_setting = GlobalStore.useAppState(
    (state) => state.GlobalParameterStore.ui_setting.show_three_point_help
  );
  const set_gp_ui = GlobalStore.actions.GlobalParameterStore.set_ui_setting;
  const set_console_state = GlobalStore.actions.console.setState;
  // function
  const process_ws_message = (msg: any): void => {
    if (msg.type == "error") {
      return;
    }
    if (
      msg.device_name == "PAA" &&
      msg.instruction == "three point alignment"
    ) {
      switch (msg.message) {
        case "start tracking": {
          set_step_number(0);
          set_incoming_data({});
          set_console_state({
            alert_type: "success",
            alert_message: "设置赤道仪开始跟踪",
          });
          // set_alert_color('success');
          // set_alert_text('设置赤道仪开始跟踪');
          break;
        }
        case "manual start move telescope": {
          set_step_number(91);
          set_incoming_data(msg.data);
          set_console_state({
            alert_type: "success",
            alert_message: "自动移动初始位置",
          });
          break;
        }
        case "start first exposure": {
          set_step_number(1);
          set_incoming_data({});
          set_console_state({
            alert_type: "success",
            alert_message: "开始第一次曝光",
          });
          // set_alert_color('success');
          // set_alert_text('开始第一次曝光');
          break;
        }
        case "first exposure solved": {
          set_incoming_data(msg.data);
          set_step_number(11);
          set_console_state({
            alert_type: "success",
            alert_message: "第一次曝光解析成功",
          });
          // set_alert_color('success');
          // set_alert_text('第一次曝光解析成功');
          break;
        }
        case "!FAIL! first exposure failed": {
          set_step_number(19);
          set_incoming_data({});
          set_console_state({
            alert_type: "danger",
            alert_message: "第一次曝光解析失败！终止流程",
          });
          setTimeout(update_paa_status_and_clean, 100);
          // set_alert_color('danger');
          // set_alert_text('第一次曝光解析失败！终止流程');
          break;
        }
        case "start second exposure": {
          set_step_number(2);
          set_incoming_data({});
          set_console_state({
            alert_type: "success",
            alert_message: "开始第二次曝光",
          });
          // set_alert_color('success');
          // set_alert_text('开始第二次曝光');
          break;
        }
        case "second exposure solved": {
          set_incoming_data(msg.data);
          set_step_number(21);
          set_console_state({
            alert_type: "success",
            alert_message: "第二次曝光解析成功",
          });
          // set_alert_color('success');
          // set_alert_text('第二次曝光解析成功');
          break;
        }
        case "!FAIL! second exposure failed": {
          set_step_number(29);
          set_incoming_data({});
          set_console_state({
            alert_type: "danger",
            alert_message: "第二次曝光解析失败！终止流程",
          });
          setTimeout(update_paa_status_and_clean, 100);
          // set_alert_color('danger');
          // set_alert_text('第二次曝光解析失败！终止流程');
          break;
        }
        case "start third exposure": {
          set_step_number(3);
          set_incoming_data({});
          set_console_state({
            alert_type: "success",
            alert_message: "开始第三次曝光",
          });
          // set_alert_color('success');
          // set_alert_text('开始第三次曝光');
          break;
        }
        case "third exposure solved": {
          set_incoming_data(msg.data);
          set_step_number(31);
          set_console_state({
            alert_type: "success",
            alert_message: "第三次曝光解析成功",
          });
          // set_alert_color('success');
          // set_alert_text('第三次曝光解析成功');
          break;
        }
        case "!FAIL! third exposure failed": {
          set_step_number(39);
          set_incoming_data({});
          set_console_state({
            alert_type: "danger",
            alert_message: "第三次曝光解析失败！终止流程",
          });
          setTimeout(update_paa_status_and_clean, 100);
          // set_alert_color('danger');
          // set_alert_text('第三次曝光解析失败！终止流程');
          break;
        }
        case "polar error result": {
          set_incoming_data(msg.data);
          set_step_number(38);
          set_console_state({
            alert_type: "success",
            alert_message: "极轴误差计算成功",
          });
          // set_alert_color('success');
          // set_alert_text('极轴误差计算成功');
          break;
        }
        case "start update exposure": {
          set_step_number(4);
          set_incoming_data({});
          set_console_state({
            alert_type: "success",
            alert_message: "开始校准曝光",
          });
          // set_alert_color('success');
          // set_alert_text('开始校准曝光');
          break;
        }
        case "update exposure solved": {
          set_incoming_data(msg.data);
          set_step_number(41);
          set_console_state({
            alert_type: "success",
            alert_message: "校准曝光解析成功",
          });
          // set_alert_color('success');
          // set_alert_text('校准曝光解析成功');
          break;
        }
        case "polar error update": {
          set_incoming_data(msg.data);
          set_step_number(42);
          set_console_state({
            alert_type: "success",
            alert_message: "极轴误差更新",
          });
          // set_alert_color('warning');
          // set_alert_text('极轴误差校准更新失败');
          break;
        }
        case "!FAIL! update exposure failed": {
          set_step_number(49);
          set_incoming_data({});
          set_console_state({
            alert_type: "warning",
            alert_message: "极轴校准曝光解析失败",
          });
          // set_alert_color('warning');
          // set_alert_text('极轴校准曝光解析失败');
          break;
        }
        case "moving telescope": {
          set_step_number(15);
          set_incoming_data({});
          set_console_state({
            alert_type: "success",
            alert_message: "移动赤道仪中",
          });
          // set_alert_color('success');
          // set_alert_text('移动赤道仪中');
          break;
        }
        case "moving telescope finished, settling": {
          set_step_number(16);
          set_incoming_data({});
          set_console_state({
            alert_type: "success",
            alert_message: "赤道仪移动完成，稳定中",
          });
          // set_alert_color('success');
          // set_alert_text('赤道仪移动完成，稳定中');
          break;
        }
        case "settle telescope finished": {
          set_step_number(17);
          set_incoming_data({});
          set_console_state({
            alert_type: "success",
            alert_message: "赤道仪稳定完成",
          });
          // set_alert_color('success');
          // set_alert_text('赤道仪稳定完成');
          break;
        }
        case "update timeout": {
          set_step_number(99);
          set_incoming_data({});
          set_console_state({
            alert_type: "warning",
            alert_message: "单次对极轴时间过长，自动终止",
          });
          setTimeout(update_paa_status_and_clean, 100);
          // set_alert_color('warning');
          // set_alert_text('单次对极轴时间过长，自动终止');
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

  // useEffect
  React.useEffect(() => {
    // check the initial status.
    if (global_parameter_ui_setting) {
      set_manual_show(manual_show + 1);
    } else {
      // open setting tab to confirm
      set_show_setting(show_setting + 1);
    }
    // check if there is paa in progres
    update_paa_status_and_clean();
    return () => {
      removeListener(process_ws_message);
    };
  }, []);

  // functions
  const on_start_polar_clicked = async () => {
    try {
      let start_result = await AXIOSPAAF.start_fixed_PolarAlignment({
        start_from: direction_switch ? "East" : "West",
        move_time: move_time,
        solve_retry: solve_retry_time,
        manual_start: manual_start,
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
  const on_stop_polar_clicked = () => {
    set_dialog_open(dialog_open + 1);
  };
  const on_setting_closed_update = (
    direction_switch: boolean,
    move_time: number,
    solve_retry_time: number,
    manual_start: boolean,
    search_radius: number
  ) => {
    set_direction_switch(direction_switch);
    set_move_time(move_time);
    set_solve_retry_time(solve_retry_time);
    set_manual_start(manual_start);
    set_search_radius(search_radius);
  };

  return (
    <div style={{ zIndex: 20 }}>
      <Button
        variant="primary"
        size="sm"
        style={{
          position: "fixed",
          left: "20px",
          top: "40px",
        }}
        onClick={() => {
          set_manual_show(manual_show + 1);
        }}
      >
        <QuestionCircle />
      </Button>
      <Card style={{ backgroundColor: "transparent" }}>
        <Card.Body>
          <Row>
            <Col>
              {!can_start_paa ? (
                <Button variant="danger" onClick={on_stop_polar_clicked}>
                  <Stop /> 停止PAA流程
                </Button>
              ) : (
                <Button variant="success" onClick={on_start_polar_clicked}>
                  <Play /> 开始三星对极轴
                </Button>
              )}
            </Col>
            <Col>
              <Button
                variant="primary"
                onClick={() => {
                  set_show_setting(show_setting + 1);
                }}
              >
                设置 <Gear />
              </Button>
            </Col>
          </Row>
        </Card.Body>
      </Card>
      <hr />
      <ThreePointStepper
        current_step={step_number}
        incoming_data={incoming_data}
      />
      <hr />
      <ThreePointSettingDialog
        open_dialog={show_setting}
        move_time={move_time}
        direction_switch={direction_switch}
        solve_retry_time={solve_retry_time}
        manual_start={manual_start}
        search_radius={search_radius}
        on_update_data={on_setting_closed_update}
      />
      <ConfirmDialog
        open={dialog_open}
        show_text={"请确认是否立即终止现在正在执行的PAA流程"}
        show_title={"确认终止流程"}
        on_confirm_clicked={post_stop_paa}
        on_cancel_clicked={() => {}}
      />
      <ThreePointManualDialog
        open_dialog={manual_show}
        handleClose={(flag: boolean) => {
          if (flag) {
            set_gp_ui({
              show_three_point_help: false,
            });
          }
          set_show_setting(show_setting + 1);
        }}
      />
    </div>
  );
};

Align.displayName = "对极轴";

export default Align;
