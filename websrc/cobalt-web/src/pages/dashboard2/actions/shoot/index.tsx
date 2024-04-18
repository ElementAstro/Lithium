import * as React from "react";
import { Button, ButtonGroup } from "react-bootstrap";
import {
  ClockFill,
  Radioactive,
  GraphUp,
  Gear,
  StopFill,
  XLg,
  Camera,
} from "react-bootstrap-icons";
import { Modal, FormControl, FormLabel, FormText } from "react-bootstrap";
import * as AXIOSPAAF from "../../../../services/paa_fixed_procedure_api";
import { OverlayTrigger, Tooltip } from "react-bootstrap";
import { Stack } from "react-bootstrap";
import { ListGroup } from "react-bootstrap";
import { Toast, ToastContainer } from "react-bootstrap";
import "./style.less";

import { ReactSVG } from "react-svg";

import { useEchoWebSocket } from "../../../../utils/websocketProvider";

const Shoot = () => {
  const [can_shoot, set_can_shoot] = React.useState(false);
  const [exposure_time_select_open, set_exposure_time_select_open] =
    React.useState(false);
  const [loop, set_loop] = React.useState(false);
  const [save, set_save] = React.useState(false);
  const [exposure_time, set_exposure_time] = React.useState(1);
  const [in_exposure, set_in_exposure] = React.useState(false);
  const [input_exposure_time, set_input_exposure_time] = React.useState("1");
  const [exposure_finished, set_exposure_finished] = React.useState(false);
  const [abort_signal, set_abort_signal] = React.useState(false);
  // extra ui control
  const [show_hfr_snackbar, set_show_hfr_snackbar] = React.useState(false);
  const [show_chart_snackbar, set_show_chart_snackbar] = React.useState(false);
  const [show_info_snackbar, set_show_info_snackbar] = React.useState(false);
  const [info_show_message, set_info_show_message] = React.useState("");
  const [newest_hf_data, set_newest_hfr_data] =
    React.useState<ICSingleHFRPointData | null>(null);
  // functions
  const check_can_shoot = async () => {
    // check paa in progress?
    set_can_shoot(false);
    try {
      let paa_running_status = await AXIOSPAAF.get_update_paa_running_status();
      if (paa_running_status.data) {
        // true means in running
        set_can_shoot(false);
      } else {
        set_can_shoot(true);
      }
    } catch (err) {
      return null;
    }
    // check camera in exposure?
    sendMessage(
      JSON.stringify({
        device_name: "camera",
        instruction: "get_real_time_info",
        params: [],
      })
    );
  };
  const on_loop_clicked = () => {
    set_loop(!loop);
  };
  const on_save_clicked = () => {
    set_save(!save);
  };
  const on_exposure_time_clicked = () => {
    set_exposure_time_select_open(true);
  };
  const on_exposure_time_changed = (target_value: number) => {
    if (target_value < 0) {
      set_exposure_time(1);
    } else {
      set_exposure_time(target_value);
    }
  };
  const handleClose = () => {
    let target_value = parseFloat(input_exposure_time);
    on_exposure_time_changed(target_value);
    set_exposure_time_select_open(false);
  };
  const start_exposure = () => {
    // check can exposure
    // console.log('start exposure clicked!');
    set_abort_signal(false);
    if (can_shoot) {
      sendMessage(
        JSON.stringify({
          device_name: "camera",
          instruction: "start_single_exposure",
          params: [exposure_time, save],
        })
      );
      set_in_exposure(true);
    }
  };
  const stop_exposure = () => {
    sendMessage(
      JSON.stringify({
        device_name: "camera",
        instruction: "abort_exposure",
        params: [],
      })
    );
    set_in_exposure(false);
    set_abort_signal(true);
  };
  const process_ws_message = (message: any) => {
    if (message.device_name == "camera") {
      if (message.instruction == "get_real_time_info") {
        set_in_exposure(message.data.in_exposure);
      }
    } else if (message.device_name == "Signal") {
      if (
        (message.instruction == "Image Stretch" ||
          message.instruction == "Image Process Failed!") &&
        message.data.camera_type == "camera"
      ) {
        set_exposure_finished(true);
      }
      if (
        message.instruction == "Image Stretch" &&
        message.data.camera_type == "camera"
      ) {
        set_newest_hfr_data(message.data.hfr_info);
      }
    }
  };
  // websocket handler
  const { sendMessage, removeListener } = useEchoWebSocket(process_ws_message);
  // useEffect
  const MINUTE_MS = 1000;
  React.useEffect(() => {
    check_can_shoot();
    const interval = setInterval(() => {
      // console.log('Logs every second');
      sendMessage(
        JSON.stringify({
          device_name: "camera",
          instruction: "get_real_time_info",
          params: [],
        })
      );
    }, MINUTE_MS);
    return () => {
      clearInterval(interval);
      removeListener(process_ws_message);
    };
  }, []);
  React.useEffect(() => {
    if (abort_signal) {
      set_abort_signal(false);
      set_in_exposure(false);
      return;
    }
    if (exposure_finished) {
      // todo, can update some status
      // console.log('got finish exposure signal', loop);
      if (loop) {
        sendMessage(
          JSON.stringify({
            device_name: "camera",
            instruction: "start_single_exposure",
            params: [exposure_time, save],
          })
        );
        set_in_exposure(true);
      } else {
        set_in_exposure(false);
      }
      set_exposure_finished(false);
    }
  }, [exposure_finished]);
  // functions for ui control
  const on_hfr_show_clicked = () => {
    set_show_chart_snackbar(false);
    set_show_hfr_snackbar(true);
  };
  const on_hfr_graph_show_clicked = () => {
    set_show_hfr_snackbar(false);
    set_show_chart_snackbar(true);
  };

  return (
    <>
      <div className="app-console-handler">
        <div className="app-console-shoot-graph"></div>
        <div className="app-console-shoot-actions">
          {can_shoot ? (
            in_exposure ? (
              <Button
                variant="danger"
                onClick={stop_exposure}
                style={{
                  position: "absolute",
                  right: 0,
                  top: "50%",
                }}
              >
                <StopFill />
              </Button>
            ) : (
              <Button
                variant="primary"
                onClick={start_exposure}
                style={{
                  position: "absolute",
                  right: 0,
                  top: "50%",
                }}
              >
                <ReactSVG
                  beforeInjection={(svg) => {
                    svg.classList.add("svg-class-name");
                    svg.setAttribute("style", "width: 24px");
                    svg.setAttribute("style", "height: 24px");
                  }}
                  src="../../../../icons/console/camera_shoot.svg"
                />
              </Button>
            )
          ) : (
            <Button
              variant="danger"
              style={{
                position: "absolute",
                right: 0,
                top: "50%",
              }}
              onClick={() => {
                set_info_show_message("PAA流程正在进行中，请终止PAA流程再试");
                set_show_info_snackbar(true);
              }}
            >
              <XLg />
            </Button>
          )}
          <div className="shoot-button-exposure-time">
            <Button onClick={on_exposure_time_clicked}>
              <ClockFill /> {exposure_time}秒
            </Button>
          </div>
          {loop ? (
            <Button
              variant="success"
              size="sm"
              onClick={on_loop_clicked}
              style={{
                position: "absolute",
                right: 0,
                top: "80%",
              }}
            >
              连拍
            </Button>
          ) : (
            <Button
              variant="secondary"
              size="sm"
              onClick={on_loop_clicked}
              style={{
                position: "absolute",
                right: 0,
                top: "80%",
              }}
            >
              单拍
            </Button>
          )}
          {save ? (
            <Button
              variant="success"
              size="sm"
              onClick={on_save_clicked}
              style={{
                position: "absolute",
                right: 0,
                top: "25%",
              }}
            >
              保存
            </Button>
          ) : (
            <Button
              variant="secondary"
              size="sm"
              onClick={on_save_clicked}
              style={{
                position: "absolute",
                right: 0,
                top: "25%",
              }}
            >
              不存
            </Button>
          )}
        </div>
        <div className="shoot-pic-process-config">
          <ButtonGroup>
            <OverlayTrigger
              placement="top"
              overlay={<Tooltip>星点数据</Tooltip>}
            >
              <Button variant="primary" onClick={on_hfr_show_clicked}>
                <Radioactive />
              </Button>
            </OverlayTrigger>
            <OverlayTrigger
              placement="top"
              overlay={<Tooltip>星点数据图表</Tooltip>}
            >
              <Button variant="primary" onClick={on_hfr_graph_show_clicked}>
                <GraphUp />
              </Button>
            </OverlayTrigger>
          </ButtonGroup>
        </div>
        <div className="app-console-shoot-status">{/* 用来显示拍摄状态 */}</div>
        {/* 用来设置曝光时间的Modal */}
        <Modal show={exposure_time_select_open} onHide={handleClose}>
          <Modal.Header closeButton>
            <Modal.Title>曝光时间输入</Modal.Title>
          </Modal.Header>
          <Modal.Body>
            <p>修改手动曝光时间，可以选择一个常用曝光时间或者手动输入时间。</p>
            <div className="d-flex flex-column align-items-center justify-content-center">
              <ButtonGroup>
                <Button onClick={() => set_input_exposure_time("0.01")}>
                  0.01
                </Button>
                <Button onClick={() => set_input_exposure_time("0.1")}>
                  0.1
                </Button>
                <Button onClick={() => set_input_exposure_time("1")}>1</Button>
                <Button onClick={() => set_input_exposure_time("2")}>2</Button>
                <Button onClick={() => set_input_exposure_time("5")}>5</Button>
                <Button onClick={() => set_input_exposure_time("8")}>8</Button>
                <Button onClick={() => set_input_exposure_time("10")}>
                  10
                </Button>
                <Button onClick={() => set_input_exposure_time("60")}>
                  60
                </Button>
                <Button onClick={() => set_input_exposure_time("120")}>
                  120
                </Button>
              </ButtonGroup>
              <FormControl className="my-3">
                <FormLabel>设置曝光时间</FormLabel>
                <FormControl
                  type="text"
                  placeholder="输入曝光时间"
                  value={input_exposure_time}
                  onChange={(event) =>
                    set_input_exposure_time(event.target.value)
                  }
                />
                <FormText>
                  如果需要拍摄bias，设置0.001秒；
                  <br />
                  一般曝光设置10秒以内，可以用来进行手动对焦，或光轴调试; <br />
                  也可以设置长曝光，用作测试拍摄。
                </FormText>
              </FormControl>
            </div>
          </Modal.Body>
          <Modal.Footer>
            <Button onClick={handleClose}>确认</Button>
          </Modal.Footer>
        </Modal>
        {/* 用来显示最新的HFR数据的Toast */}
      </div>

      <ToastContainer position="bottom-start">
        <Toast show={show_hfr_snackbar} bg="success">
          <Toast.Header>
            <strong className="me-auto">最新曝光星点数据</strong>
          </Toast.Header>
          <Toast.Body>
            <Stack gap={2}>
              <ListGroup>
                <ListGroup.Item>星点数据量</ListGroup.Item>
                <ListGroup.Item>平均HFR</ListGroup.Item>
              </ListGroup>
            </Stack>
          </Toast.Body>
        </Toast>
      </ToastContainer>
      {/* 显示HFR历史记录图标的Toast */}
      <ToastContainer position="bottom-start">
        <Toast show={show_chart_snackbar} bg="success">
          <Toast.Header>
            <strong className="me-auto">最新曝光星点数据</strong>
          </Toast.Header>
        </Toast>
      </ToastContainer>

      <ToastContainer position="bottom-end">
        <Toast
          show={show_info_snackbar}
          onClose={() => set_show_info_snackbar(false)}
          delay={3000}
          autohide
        >
          <Toast.Body>{info_show_message}</Toast.Body>
        </Toast>
      </ToastContainer>
    </>
  );
};

Shoot.displayName = "拍摄";

export default Shoot;
