import * as React from "react";
import { useEchoWebSocket } from "../../../../utils/websocketProvider";
import {
  Card,
  Form,
  Button,
  Stack,
  Row,
  Col,
  Container,
} from "react-bootstrap";
import {
  ArrowUp,
  ArrowRight,
  CloudSnow,
  Grid3x3GapFill,
} from "react-bootstrap-icons";

const DeviceCameraSimpleControlPanel: React.FC = () => {
  const [cool_status, set_cool_status] = React.useState(false);
  const [camera_temperature, set_camera_temperature] = React.useState(0);
  const [binning, set_binning] = React.useState(1);
  const [gain, set_gain] = React.useState(100);
  const [offset, set_offset] = React.useState(0);

  // input data
  const [input_target_temperature, set_input_target_temperature] =
    React.useState<string>("-10");
  const [input_binning, set_input_binning] = React.useState<string>("1");
  const [input_gain, set_input_gain] = React.useState<string>("");
  const [input_offset, set_input_offset] = React.useState<string>("");

  // functions
  const process_ws_message = (msg: any): void => {
    if (msg.device_name == "camera") {
      switch (msg.instruction) {
        case "get_real_time_info": {
          set_camera_temperature(msg.data.temperature);
          set_cool_status(msg.data.in_cooling);
          break;
        }
        case "get_set_params": {
          set_gain(msg.data.gain);
          set_offset(msg.data.offset);
          // binning
          if (msg.data.binning !== null) {
            set_binning(msg.data.binning);
          }
        }
      }
    }
  };

  const { sendMessage, removeListener } = useEchoWebSocket(process_ws_message);
  const MINUTE_MS = 1000;
  React.useEffect(() => {
    set_input_gain(String(gain));
  }, [gain]);
  React.useEffect(() => {
    set_input_offset(String(offset));
  }, [offset]);
  React.useEffect(() => {
    set_input_binning(String(binning));
  }, [binning]);
  React.useEffect(() => {
    const interval = setInterval(() => {
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

  return (
    <Container fluid>
      <Card
        style={{
          position: "absolute",
          top: "50%",
          right: 0,
          transform: "translateY(-50%)",
          zIndex: 20,
        }}
      >
        <Card.Body>
          <Stack direction="horizontal" gap={2}>
            {cool_status ? (
              <span className="text-danger">制冷中</span>
            ) : (
              <span className="text-success">未制冷</span>
            )}
            <Form.Check
              type="switch"
              checked={cool_status}
              onChange={(event) => {
                set_cool_status(event.target.checked);
                if (event.target.checked) {
                  sendMessage(
                    JSON.stringify({
                      device_name: "camera",
                      instruction: "start_cool_camera",
                      params: [],
                    })
                  );
                } else {
                  sendMessage(
                    JSON.stringify({
                      device_name: "camera",
                      instruction: "stop_cool_camera",
                      params: [],
                    })
                  );
                }
              }}
              label="开关冷冻"
            />
          </Stack>
          <Row className="mt-3">
            <Col xs={6}>
              <span>相机温度</span>
            </Col>
            <Col xs={6}>
              {camera_temperature < 0 ? (
                <span className="text-success">{camera_temperature} ℃</span>
              ) : (
                <span className="text-danger">{camera_temperature} ℃</span>
              )}
            </Col>
          </Row>
          <Row className="mt-3">
            <Col>
              <Form.Group>
                <Form.Label>修改冷冻目标温度 ℃</Form.Label>
                <Form.Control
                  type="number"
                  value={input_target_temperature}
                  onChange={(event) =>
                    set_input_target_temperature(event.target.value)
                  }
                />
                <Button
                  variant="primary"
                  size="sm"
                  className="mt-2"
                  onClick={() => {
                    if (input_target_temperature !== null) {
                      sendMessage(
                        JSON.stringify({
                          device_name: "camera",
                          instruction: "set_cool_target_temperature",
                          params: [parseInt(input_target_temperature)],
                        })
                      );
                    }
                  }}
                >
                  <CloudSnow size={20} /> 执行
                </Button>
              </Form.Group>
            </Col>
          </Row>
          <Row className="mt-3">
            <Col xs={6}>
              <span>当前增益gain</span>
            </Col>
            <Col xs={6}>
              <span className="text-success">{gain}</span>
            </Col>
          </Row>
          <Row className="mt-3">
            <Col>
              <Form.Group>
                <Form.Label>修改增益</Form.Label>
                <Form.Control
                  type="number"
                  value={input_gain}
                  onChange={(event) => set_input_gain(event.target.value)}
                />
                <Button
                  variant="primary"
                  size="sm"
                  className="mt-2"
                  onClick={() => {
                    if (input_gain !== null) {
                      sendMessage(
                        JSON.stringify({
                          device_name: "camera",
                          instruction: "set_number_parameters",
                          params: ["gain", parseInt(input_gain)],
                        })
                      );
                    }
                  }}
                >
                  <ArrowUp /> 执行
                </Button>
              </Form.Group>
            </Col>
          </Row>
          <Row className="mt-3">
            <Col xs={6}>
              <span>当前偏置值offset</span>
            </Col>
            <Col xs={6}>
              <span className="text-success">{offset}</span>
            </Col>
          </Row>
          <Row className="mt-3">
            <Col>
              <Form.Group>
                <Form.Label>修改偏置值</Form.Label>
                <Form.Control
                  type="number"
                  value={input_offset}
                  onChange={(event) => set_input_offset(event.target.value)}
                />
                <Button
                  variant="primary"
                  size="sm"
                  className="mt-2"
                  onClick={() => {
                    if (input_offset !== null) {
                      sendMessage(
                        JSON.stringify({
                          device_name: "camera",
                          instruction: "set_number_parameters",
                          params: ["offset", parseInt(input_offset)],
                        })
                      );
                    }
                  }}
                >
                  <ArrowRight /> 执行
                </Button>
              </Form.Group>
            </Col>
          </Row>
          <Row className="mt-3">
            <Col xs={6}>
              <span>当前binning</span>
            </Col>
            <Col xs={6}>
              <span className="text-success">{binning}</span>
            </Col>
          </Row>
          <Row className="mt-3">
            <Col>
              <Form.Group>
                <Form.Label>修改binning值</Form.Label>
                <Form.Control
                  type="number"
                  value={input_binning}
                  onChange={(event) => {
                    let real_binning = parseInt(event.target.value);
                    if (real_binning > 4) {
                      real_binning = 4;
                    }
                    if (real_binning == 3) {
                      real_binning = 4;
                    }
                    if (real_binning < 1) {
                      real_binning = 1;
                    }
                    set_input_binning(String(real_binning));
                  }}
                />
                <Button
                  variant="primary"
                  size="sm"
                  className="mt-2"
                  onClick={() => {
                    if (input_binning !== null) {
                      sendMessage(
                        JSON.stringify({
                          device_name: "camera",
                          instruction: "set_number_parameters",
                          params: ["binning", parseInt(input_binning)],
                        })
                      );
                    }
                  }}
                >
                  <Grid3x3GapFill /> 执行
                </Button>
              </Form.Group>
            </Col>
          </Row>
        </Card.Body>
      </Card>
    </Container>
  );
};

DeviceCameraSimpleControlPanel.displayName = "相机控制";

export default DeviceCameraSimpleControlPanel;
