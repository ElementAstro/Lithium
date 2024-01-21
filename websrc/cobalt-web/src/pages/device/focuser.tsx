import * as React from "react";
import {
  Container,
  Row,
  Col,
  Button,
  FormControl,
  InputGroup,
  Alert,
} from "react-bootstrap";
import {
  ArrowLeftCircle,
  ArrowRightCircle,
  ArrowRight,
  ArrowLeft,
  ArrowsExpand,
  Printer,
  CheckCircle,
  Clock,
  Hourglass,
  Mouse,
} from "react-bootstrap-icons";
import HelperSnackbar from "./description/helper_snackbar";
import { useEchoWebSocket } from "../../utils/websocketProvider";

const DeviceFocuserGeneralControlPanel: React.FC = () => {
  const [current_step, set_current_step] = React.useState(0);
  const [max_step, set_max_step] = React.useState(65000);
  const [focus_temperature, set_focus_temperature] = React.useState(0);
  const [work_status, set_work_status] = React.useState(false);
  const [backslash_flag, set_backslash_flag] = React.useState(false);
  const [backslash_step, set_backslash_step] = React.useState(0);
  const [move_small_step, set_move_small_step] = React.useState(0);
  const [move_large_step, set_move_large_step] = React.useState(0);
  const [help_text, set_help_text] = React.useState("");
  const helper_ref = React.useRef<HelperHandle | null>(null);

  // input label data
  const [to_set_max_step, set_to_set_max_step] = React.useState(0);
  const [to_set_move_step, set_to_set_move_step] = React.useState(0);
  const [to_set_move_rel_step, set_to_set_move_rel_step] = React.useState(0);
  const [to_set_small_move, set_to_set_small_move] = React.useState(0);
  const [to_set_large_move, set_to_set_large_move] = React.useState(0);
  const [to_switch_backslash, set_to_switch_backslash] = React.useState(false);
  const [to_set_backslash_step, set_to_set_backslash_step] = React.useState(0);

  const process_ws_message = (msg: any): void => {
    if (msg.type == "error") {
      return;
    }
    let response_data = msg;
    // console.log(response_data);
    if (response_data.device_name == "focus") {
      // process only this device response
      switch (response_data.instruction) {
        case "get_params": {
          set_current_step(response_data.data.focus_position);
          set_max_step(response_data.data.focus_max);
          if (
            response_data.data.focus_state == "Idle" ||
            response_data.data.focus_state == "Ok"
          ) {
            set_work_status(false);
          } else {
            set_work_status(true);
          }
          set_backslash_flag(response_data.data.backslash_switch);
          set_backslash_step(response_data.data.backslash);
          set_move_small_step(response_data.data.small_step);
          set_move_large_step(response_data.data.large_step);
          break;
        }
        case "set_max_step": {
          if (response_data.message == "max step must be larger than 0!") {
            // report error
          }
          break;
        }
        case "set_move_small_step": {
          if (response_data.message == "step value must be larger than 0!") {
            // report error
          }
          break;
        }
        case "set_move_large_step": {
          if (response_data.message == "step value must be larger than 0!") {
            // report error
          }
          break;
        }
        case "move_to_position": {
          if (response_data.message == "focus is in moving! Cannot move!") {
            // report error
          } else if (
            response_data.message == "step value must be larger than 0!"
          ) {
          } else if (
            response_data.message ==
            "step value cannot be larger than max step!"
          ) {
          }
          break;
        }
        case "move_relative_step": {
          if (response_data.message == "focus is in moving! Cannot move!") {
            // report error
          }
          break;
        }
        case "move_in_small": {
          if (response_data.message == "focus is in moving! Cannot move!") {
            // report error
          }
          break;
        }
        case "move_in_large": {
          if (response_data.message == "focus is in moving! Cannot move!") {
            // report error
          }
          break;
        }
        case "move_out_small": {
          if (response_data.message == "focus is in moving! Cannot move!") {
            // report error
          }
          break;
        }
        case "move_out_large": {
          if (response_data.message == "focus is in moving! Cannot move!") {
            // report error
          }
          break;
        }
        case "turn_on_backslash": {
          if (response_data.message == "focus is in moving! Cannot move!") {
            // report error
          }
          break;
        }
        case "turn_off_backslash": {
          if (response_data.message == "focus is in moving! Cannot move!") {
            // report error
          }
          break;
        }
        case "set_backslash": {
          if (response_data.message == "focus is in moving! Cannot move!") {
            // report error
          }
          break;
        }
        default: {
        }
      }
    }
  };

  const { sendMessage, removeListener } = useEchoWebSocket(process_ws_message);

  const MINUTE_MS = 1000;
  React.useEffect(() => {
    // This will fire only on mount.
    const interval = setInterval(() => {
      console.log("Logs every second");
      sendMessage(
        JSON.stringify({
          device_name: "focus",
          instruction: "get_params",
          params: [],
        })
      );
    }, MINUTE_MS);

    return () => {
      console.log("clear interval");
      clearInterval(interval);
      removeListener(process_ws_message);
    }; // This represents the unmount function, in which you need to clear your interval to prevent memory leaks.
  }, []);

  // all watches to most parameters
  React.useEffect(() => {
    set_to_set_max_step(max_step);
  }, [max_step]);
  React.useEffect(() => {
    set_to_set_small_move(move_small_step);
  }, [move_small_step]);
  React.useEffect(() => {
    set_to_set_large_move(move_large_step);
  }, [move_large_step]);
  React.useEffect(() => {
    set_to_set_backslash_step(backslash_step);
  }, [backslash_step]);
  React.useEffect(() => {
    set_to_switch_backslash(backslash_flag);
  }, [backslash_flag]);

  return (
    <div>
      <HelperSnackbar
        ref={helper_ref}
        help_text={help_text}
        close_signal={false}
      />
      <Container fluid className="mt-1">
        <Row>
          <Col xs={12} md={6}>
            <div className="p-2 border">
              <h4>调焦器信息</h4>
              <p>当前位置 {current_step}</p>
              <p>最大步数 {max_step}</p>
              <p>测量温度 {focus_temperature} ℃</p>
              <p>移动小步步长 {move_small_step}</p>
              <p>移动大步步长 {move_large_step}</p>
              <p>
                {work_status ? (
                  <Button variant="danger">
                    <Hourglass />
                    正在运行
                  </Button>
                ) : (
                  <Button variant="primary">
                    <CheckCircle />
                    待机
                  </Button>
                )}
              </p>
            </div>
          </Col>
          <Col xs={12} md={6}>
            <div className="p-2 border">
              <Row className="mb-2">
                <Col>
                  <Button
                    variant="dark"
                    className="me-2"
                    onClick={() => {
                      if (!work_status) {
                        sendMessage(
                          JSON.stringify({
                            device_name: "focus",
                            instruction: "move_in_large",
                            params: [],
                          })
                        );
                        set_work_status(true);
                      } else {
                        // popup notification
                        set_help_text("设备移动中，悠着点！");
                        helper_ref.current?.open_snackbar();
                      }
                    }}
                  >
                    <ArrowLeft />
                    移近 大步
                  </Button>
                  <Button
                    variant="dark"
                    className="me-2"
                    onClick={() => {
                      if (!work_status) {
                        sendMessage(
                          JSON.stringify({
                            device_name: "focus",
                            instruction: "move_in_small",
                            params: [],
                          })
                        );
                        set_work_status(true);
                      } else {
                        // popup notification
                        set_help_text("设备移动中，悠着点！");
                        helper_ref.current?.open_snackbar();
                      }
                    }}
                  >
                    <ArrowLeftCircle />
                    移近 小步
                  </Button>
                  <Button
                    variant="dark"
                    className="me-2"
                    onClick={() => {
                      if (!work_status) {
                        sendMessage(
                          JSON.stringify({
                            device_name: "focus",
                            instruction: "move_out_small",
                            params: [],
                          })
                        );
                        set_work_status(true);
                      } else {
                        // popup notification
                        set_help_text("设备移动中，悠着点！");
                        helper_ref.current?.open_snackbar();
                      }
                    }}
                  >
                    移出 小步
                    <ArrowRightCircle />
                  </Button>
                  <Button
                    variant="dark"
                    onClick={() => {
                      if (!work_status) {
                        sendMessage(
                          JSON.stringify({
                            device_name: "focus",
                            instruction: "move_out_large",
                            params: [],
                          })
                        );
                        set_work_status(true);
                      } else {
                        // popup notification
                        set_help_text("设备移动中，悠着点！");
                        helper_ref.current?.open_snackbar();
                      }
                    }}
                  >
                    移出 大步
                    <ArrowRight />
                  </Button>
                </Col>
              </Row>
              <Row className="mb-2">
                <Col>
                  <InputGroup className="mb-3">
                    <InputGroup.Text id="basic-addon1">
                      <ArrowsExpand />
                    </InputGroup.Text>
                    <FormControl
                      placeholder="设置最大步长"
                      aria-label="设置最大步长"
                      aria-describedby="basic-addon1"
                      value={to_set_max_step}
                      onChange={(event) =>
                        set_to_set_max_step(parseInt(event.target.value))
                      }
                    />
                    <Button
                      variant="dark"
                      id="button-addon1"
                      onClick={() => {
                        if (!work_status) {
                          sendMessage(
                            JSON.stringify({
                              device_name: "focus",
                              instruction: "set_max_step",
                              params: [to_set_max_step],
                            })
                          );
                          set_work_status(true);
                        } else {
                          // popup notification
                          set_help_text("设备移动中，悠着点！");
                          helper_ref.current?.open_snackbar();
                        }
                      }}
                    >
                      执行
                    </Button>
                  </InputGroup>
                </Col>
              </Row>
              <Row className="mb-2">
                <Col>
                  <InputGroup className="mb-3">
                    <InputGroup.Text id="basic-addon1">
                      <Printer />
                    </InputGroup.Text>
                    <FormControl
                      placeholder="移动到指定步长"
                      aria-label="移动到指定步长"
                      aria-describedby="basic-addon1"
                      value={to_set_move_step}
                      onChange={(event) =>
                        set_to_set_move_step(parseInt(event.target.value))
                      }
                    />
                    <Button
                      variant="dark"
                      id="button-addon1"
                      onClick={() => {
                        if (work_status) {
                          // popup notification
                          set_help_text("设备移动中，悠着点！");
                          helper_ref.current?.open_snackbar();
                          return;
                        }
                        if (to_set_move_step > max_step) {
                          set_help_text("移动步长不能超过设置的最大步长！");
                          helper_ref.current?.open_snackbar();
                          return;
                        }
                        sendMessage(
                          JSON.stringify({
                            device_name: "focus",
                            instruction: "move_to_position",
                            params: [to_set_move_step],
                          })
                        );
                        set_work_status(true);
                      }}
                    >
                      执行
                    </Button>
                  </InputGroup>
                </Col>
              </Row>
              <Row className="mb-2">
                <Col>
                  <InputGroup className="mb-3">
                    <InputGroup.Text id="basic-addon1">
                      <Mouse />
                    </InputGroup.Text>
                    <FormControl
                      placeholder="移动相对步长"
                      aria-label="移动相对步长"
                      aria-describedby="basic-addon1"
                      value={to_set_move_rel_step}
                      onChange={(event) =>
                        set_to_set_move_rel_step(parseInt(event.target.value))
                      }
                    />
                    <Button
                      variant="dark"
                      id="button-addon1"
                      onClick={() => {
                        if (work_status) {
                          //                      set_help_text("设备移动中，悠着点！");
                          helper_ref.current?.open_snackbar();
                          return;
                        }
                        const new_position =
                          current_step + to_set_move_rel_step;
                        if (new_position < 0 || new_position > max_step) {
                          set_help_text("移动步长超出范围！");
                          helper_ref.current?.open_snackbar();
                          return;
                        }
                        sendMessage(
                          JSON.stringify({
                            device_name: "focus",
                            instruction: "move_relative",
                            params: [to_set_move_rel_step],
                          })
                        );
                        set_work_status(true);
                      }}
                    >
                      执行
                    </Button>
                  </InputGroup>
                </Col>
              </Row>
              <Row className="mb-2">
                <Col>
                  <InputGroup className="mb-3">
                    <InputGroup.Text id="basic-addon1">
                      <Clock />
                    </InputGroup.Text>
                    <FormControl
                      placeholder="停止移动"
                      aria-label="停止移动"
                      aria-describedby="basic-addon1"
                      value={""}
                      disabled
                    />
                    <Button
                      variant="danger"
                      id="button-addon1"
                      onClick={() => {
                        if (!work_status) {
                          // popup notification
                          set_help_text("设备已经停止移动！");
                          helper_ref.current?.open_snackbar();
                        } else {
                          sendMessage(
                            JSON.stringify({
                              device_name: "focus",
                              instruction: "stop_movement",
                              params: [],
                            })
                          );
                          set_work_status(false);
                        }
                      }}
                    >
                      停止
                    </Button>
                  </InputGroup>
                </Col>
              </Row>
            </div>
          </Col>
        </Row>
      </Container>
    </div>
  );
};

export default DeviceFocuserGeneralControlPanel;
