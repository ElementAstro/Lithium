import * as React from "react";
import {
  Container,
  Row,
  Col,
  Card,
  Form,
  Button,
  Modal,
} from "react-bootstrap";
import { Gear, ArrowClockwise, XLg } from "react-bootstrap-icons";
import { useEchoWebSocket } from "../../../../utils/websocketProvider";

import SquareButton from "../../../device/SquareButton";

const DeviceTelescopeSimpleControlPanel: React.FC = () => {
  const [tracking_status, set_tracking_status] = React.useState(false);
  const [moving_time, set_moving_time] = React.useState("1"); // in seconds
  // ui data
  const [fix_time_selection, set_fix_time_selection] = React.useState(true);
  const [slew_speed_selections, set_slew_speed_selections] = React.useState<
    Array<{ label: String; value: String }>
  >([{ label: "1x", value: "1x" }]);
  const [selected_slew_speed, set_selected_slew_speed] =
    React.useState("0(0.25x)");
  const [setting_open, set_setting_open] = React.useState(false);

  const time_selections = [
    { label: "0.001", value: "0.001" },
    { label: "0.01", value: "0.01" },
    { label: "0.05", value: "0.05" },
    { label: "0.1", value: "0.1" },
    { label: "0.5", value: "0.5" },
    { label: "1", value: "1" },
    { label: "2", value: "2" },
  ];
  const process_ws_message = (msg: any): void => {
    if (msg.device_name == "telescope") {
      switch (msg.instruction) {
        case "get_set_params": {
          // update all selections to selects.
          let new_selections = [];
          for (let i = 0; i < msg.data.slew_rate.selections.length; i++) {
            new_selections.push({
              label: msg.data.slew_rate.selections[i],
              value: msg.data.slew_rate.selections[i],
            });
          }
          set_slew_speed_selections(new_selections);
          // set current slection.
          set_selected_slew_speed(msg.data.slew_rate.value);
          break;
        }
        case "get_real_time_info": {
          set_tracking_status(msg.data.tracking);
          break;
        }
      }
    }
  };
  const { sendMessage, removeListener } = useEchoWebSocket(process_ws_message);
  const MINUTE_MS = 1000;
  React.useEffect(() => {
    sendMessage(
      JSON.stringify({
        device_name: "telescope",
        instruction: "get_set_params",
        params: [],
      })
    );
    // This will fire only on mount.
    const interval = setInterval(() => {
      // console.log('Logs every second');
      sendMessage(
        JSON.stringify({
          device_name: "telescope",
          instruction: "get_real_time_info",
          params: [],
        })
      );
    }, MINUTE_MS);

    return () => {
      console.log("clear interval");
      clearInterval(interval);
      removeListener(process_ws_message);
      // if arrow in action, send abort
      if (arrow_in_action) {
        sendMessage(
          JSON.stringify({
            device_name: "telescope",
            instruction: "abort",
            params: [],
          })
        );
      }
    }; // This represents the unmount function, in which you need to clear your interval to prevent memory leaks.
  }, []);
  React.useEffect(() => {
    sendMessage(
      JSON.stringify({
        device_name: "telescope",
        instruction: "set_slew_rate",
        params: [selected_slew_speed],
      })
    );
  }, [selected_slew_speed]);
  const [arrow_in_action, set_arrow_in_action] = React.useState(false);
  const arrow_action_func = (btn_type: string, action: string) => {
    // console.log('!!!arrow', btn_type, action);

    if (fix_time_selection) {
      if (action == "press") {
        if (arrow_in_action) {
          // console.log('in action, no effect');
          return;
        }
        set_arrow_in_action(true);
        let wait_time = 0;
        if (moving_time[0] == null) {
          wait_time = 1000;
        } else {
          wait_time = parseFloat(moving_time[0]);
        }
        // set time out
        // console.log('set timeout');
        sendMessage(
          JSON.stringify({
            device_name: "telescope",
            instruction: "handle_move_button",
            params: [btn_type, true],
          })
        );
        setTimeout(() => {
          // console.log('timeout finished');
          sendMessage(
            JSON.stringify({
              device_name: "telescope",
              instruction: "handle_move_button",
              params: [btn_type, false],
            })
          );
          set_arrow_in_action(false);
        }, wait_time);
      }
    } else {
      if (action == "press") {
        if (arrow_in_action) {
          console.log("in action, no effect");
          return;
        }
        // console.log('press down');
        sendMessage(
          JSON.stringify({
            device_name: "telescope",
            instruction: "handle_move_button",
            params: [btn_type, true],
          })
        );
        set_arrow_in_action(true);
      } else if (action == "release") {
        // console.log('released');
        sendMessage(
          JSON.stringify({
            device_name: "telescope",
            instruction: "handle_move_button",
            params: [btn_type, false],
          })
        );
        set_arrow_in_action(false);
      } else {
        console.log("error!");
      }
    }
  };

  return (
    <Container fluid>
      <Card
        style={{
          position: "absolute",
          top: "50%",
          backgroundColor: "transparent",
          right: 5,
          transform: "translateY(-50%)",
          zIndex: 20,
        }}
      >
        <Row className="mb-1" lg={12}>
          <Col>
            {fix_time_selection ? (
              <h4
                style={{
                  color: "white",
                  textShadow: "2px 2px 4px rgba(0, 0, 0, 0.5)",
                }}
              >
                固定移动时间
              </h4>
            ) : (
              <h4
                style={{
                  color: "white",
                  textShadow: "2px 2px 4px rgba(0, 0, 0, 0.5)",
                }}
              >
                长按手柄模式
              </h4>
            )}
          </Col>
        </Row>
        <Row>
          <Col xs={4}>
            <SquareButton
              variant="secondary"
              style={{ width: "100%" }}
              onClick={() => set_setting_open(true)}
            >
              <Gear /> 设置
            </SquareButton>
          </Col>
          <Col xs={4}>
            <SquareButton
              variant="primary"
              style={{ width: "100%" }}
              onMouseDown={() => arrow_action_func("up", "press")}
              onMouseUp={() => arrow_action_func("up", "release")}
            >
              上
            </SquareButton>
          </Col>
          <Col xs={4}>
            <SquareButton variant="primary" disabled style={{ width: "100%" }}>
              {null}
            </SquareButton>
          </Col>
        </Row>
        <Row>
          <Col xs={4}>
            <SquareButton
              variant="primary"
              style={{ width: "100%" }}
              onMouseDown={() => arrow_action_func("left", "press")}
              onMouseUp={() => arrow_action_func("left", "release")}
            >
              左
            </SquareButton>
          </Col>
          <Col xs={4}>
            <SquareButton
              variant="danger"
              style={{ width: "100%" }}
              onClick={() => {
                sendMessage(
                  JSON.stringify({
                    device_name: "telescope",
                    instruction: "abort",
                    params: [],
                  })
                );
              }}
            >
              停
            </SquareButton>
          </Col>
          <Col xs={4}>
            <SquareButton
              variant="primary"
              style={{ width: "100%" }}
              onMouseDown={() => arrow_action_func("right", "press")}
              onMouseUp={() => arrow_action_func("right", "release")}
            >
              右
            </SquareButton>
          </Col>
        </Row>
        <Row>
          <Col xs={4}>
            {tracking_status ? (
              <SquareButton variant="success" style={{ width: "100%" }}>
                <ArrowClockwise /> 跟踪中
              </SquareButton>
            ) : (
              <SquareButton variant="danger" style={{ width: "100%" }}>
                <XLg /> 未跟踪
              </SquareButton>
            )}
          </Col>
          <Col xs={4}>
            <SquareButton
              variant="primary"
              style={{ width: "100%" }}
              onMouseDown={() => arrow_action_func("down", "press")}
              onMouseUp={() => arrow_action_func("down", "release")}
            >
              下
            </SquareButton>
          </Col>
          <Col xs={4}>
            <SquareButton
              variant="secondary"
              style={{ width: "100%" }}
              onClick={() => {
                if (tracking_status) {
                  sendMessage(
                    JSON.stringify({
                      device_name: "telescope",
                      instruction: "stop_track",
                      params: [],
                    })
                  );
                } else {
                  sendMessage(
                    JSON.stringify({
                      device_name: "telescope",
                      instruction: "start_track",
                      params: [],
                    })
                  );
                }
              }}
            >
              开始跟踪/停止跟踪
            </SquareButton>
          </Col>
        </Row>

        <Modal show={setting_open} onHide={() => set_setting_open(false)}>
          <Modal.Header closeButton>
            <Modal.Title>赤道仪手控器参数设置</Modal.Title>
          </Modal.Header>
          <Modal.Body>
            <Form.Group controlId="selectSlewRate">
              <Form.Label>设置赤道仪移动速率</Form.Label>
              <Form.Control
                as="select"
                value={selected_slew_speed}
                onChange={(event) =>
                  set_selected_slew_speed(event.target.value)
                }
              >
                {slew_speed_selections.map((one_slew_selection, index) => (
                  <option value={one_slew_selection.value} key={index}>
                    {one_slew_selection.label}
                  </option>
                ))}
              </Form.Control>
            </Form.Group>
            <Form.Group controlId="switchMode" className="my-3">
              <Form.Check
                type="switch"
                checked={fix_time_selection}
                onChange={(event) => {
                  if (!arrow_in_action) {
                    set_fix_time_selection(event.target.checked);
                  }
                }}
                label={fix_time_selection ? "固定时间模式" : "模拟手柄模式"}
              />
            </Form.Group>
            {fix_time_selection && (
              <Form.Group controlId="selectSlewTime">
                <Form.Label>选择手动移动时长(s)</Form.Label>
                <Form.Control
                  as="select"
                  value={moving_time}
                  onChange={(event) => set_moving_time(event.target.value)}
                >
                  {time_selections.map((one_time_select, index) => (
                    <option value={one_time_select.value} key={index}>
                      {one_time_select.label}
                    </option>
                  ))}
                </Form.Control>
              </Form.Group>
            )}
          </Modal.Body>
          <Modal.Footer>
            <Button variant="primary" onClick={() => set_setting_open(false)}>
              确认
            </Button>
          </Modal.Footer>
        </Modal>
      </Card>
    </Container>
  );
};

DeviceTelescopeSimpleControlPanel.displayName = "赤道仪控制";

export default DeviceTelescopeSimpleControlPanel;
