import * as React from "react";
import { useEchoWebSocket } from "../../../../utils/websocketProvider";
import { Card, Button, ButtonGroup, Form, InputGroup } from "react-bootstrap";
import {
  ArrowLeftCircleFill,
  ArrowRightCircleFill,
  ArrowLeftSquareFill,
  ArrowRightSquareFill,
  CursorFill,
} from "react-bootstrap-icons";
import HelperSnackbar from '../../../device/description/helper_snackbar'

const DeviceFocuserSimpleControlPanel: React.FC = () => {
  const [current_step, set_current_step] = React.useState(0);
  const [max_step, set_max_step] = React.useState(65000);
  const [work_status, set_work_status] = React.useState(false);
  const [help_text, set_help_text] = React.useState("");
  const helper_ref = React.useRef<HelperHandle | null>(null);

  const [to_set_move_step, set_to_set_move_step] = React.useState(0);
  const [to_set_move_rel_step, set_to_set_move_rel_step] = React.useState(0);

  const process_ws_message = (msg: any): void => {
    let response_data = msg;
    if (response_data.device_name == "focus") {
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
          break;
        }
      }
    }
  };

  const { sendMessage, removeListener } = useEchoWebSocket(process_ws_message);

  const MINUTE_MS = 1000;
  React.useEffect(() => {
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
    };
  }, []);

  return (
    <Card
      style={{
        position: "absolute",
        width: "100%",
        top: "50%",
        right: 0,
        transform: "translateY(-50%)",
        zIndex: 20,
      }}
    >
      <HelperSnackbar
        ref={helper_ref}
        help_text={help_text}
        close_signal={false}
      />
      <Card.Body>
        <p className="mb-2">当前位置 {current_step}</p>
        <ButtonGroup className="mb-3 w-100">
          <Button
            variant="primary"
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
                set_help_text("设备移动中，悠着点！");
                helper_ref.current?.open_snackbar();
              }
            }}
          >
            <ArrowLeftCircleFill /> 大步向内
          </Button>
          <Button
            variant="primary"
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
                set_help_text("设备移动中，悠着点！");
                helper_ref.current?.open_snackbar();
              }
            }}
          >
            <ArrowLeftSquareFill /> 小步向内
          </Button>
          <Button
            variant="primary"
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
                set_help_text("设备移动中，悠着点！");
                helper_ref.current?.open_snackbar();
              }
            }}
          >
            小步向外 <ArrowRightSquareFill />
          </Button>
          <Button
            variant="primary"
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
                set_help_text("设备移动中，悠着点！");
                helper_ref.current?.open_snackbar();
              }
            }}
          >
            大步向外 <ArrowRightCircleFill />
          </Button>
        </ButtonGroup>
        <Form.Group className="mb-3">
          <Form.Label>移动到指定步长</Form.Label>
          <InputGroup>
            <InputGroup.Text>
              <CursorFill />
            </InputGroup.Text>
            <Form.Control
              type="number"
              value={to_set_move_step}
              onChange={(event) =>
                set_to_set_move_step(parseInt(event.target.value))
              }
            />
            <Button
              variant="primary"
              onClick={() => {
                if (work_status) {
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
        </Form.Group>
        <Form.Group>
          <Form.Label>移动相对步长</Form.Label>
          <InputGroup>
            <InputGroup.Text>
              <CursorFill />
            </InputGroup.Text>
            <Form.Control
              type="number"
              value={to_set_move_rel_step}
              onChange={(event) =>
                set_to_set_move_rel_step(parseInt(event.target.value))
              }
            />
            <Button
              variant="primary"
              onClick={() => {
                if (work_status) {
                  set_help_text("设备移动中，悠着点！");
                  helper_ref.current?.open_snackbar();
                  return;
                }
                sendMessage(
                  JSON.stringify({
                    device_name: "focus",
                    instruction: "move_relative_step",
                    params: [to_set_move_rel_step],
                  })
                );
                set_work_status(true);
              }}
            >
              执行
            </Button>
          </InputGroup>
        </Form.Group>
      </Card.Body>
    </Card>
  );
};

DeviceFocuserSimpleControlPanel.displayName = "调焦器控制";

export default DeviceFocuserSimpleControlPanel;
