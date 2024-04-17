import * as React from "react";
import {
  Container,
  Button,
  ButtonGroup,
  Alert,
  Accordion,
  Card,
  CardBody,
  CardHeader,
} from "react-bootstrap";
import { useEchoWebSocket } from "../../../../utils/websocketProvider";
import { GlobalStore } from "../../../../store/globalStore";

import { ReactComponent as GuideSVG } from "../../../../../icons/console/guide.svg";
import { ReactComponent as CameraShootSVG } from "../../../../../icons/console/camera_shoot.svg";
import PHD2GuideResultGraph from "../../comps/graph/PHD2_guide_graph";

const DevicePHD2SimpleControlPanel: React.FC = () => {
  // ui control
  const [guide_graph, set_guide_graph] = React.useState(false);
  const [calibration_result, set_calibration_result] = React.useState(false);

  const close_guide_graph = () => {
    set_guide_graph(false);
  };
  const open_guide_graph = () => {
    set_guide_graph(!guide_graph);
  };
  const open_calibration_result = () => {
    set_calibration_result(!calibration_result);
  };
  const close_calibration_result = () => {
    set_calibration_result(false);
  };
  const check_phd2_status = () => {};
  const process_ws_message = (msg: any): void => {
    if (msg.device_name == "phd2_event") {
    } else if (msg.device_name == "PHD2_response") {
    }
  };
  const start_exposure = () => {
    sendMessage(
      JSON.stringify({
        device_name: "phd2",
        instruction: "loop_exposure",
        params: [],
      })
    );
  };
  const start_calibration = () => {
    sendMessage(
      JSON.stringify({
        device_name: "phd2",
        instruction: "start_guide",
        params: [true],
      })
    );
  };
  const start_guide = () => {
    sendMessage(
      JSON.stringify({
        device_name: "phd2",
        instruction: "start_guide",
        params: [false],
      })
    );
  };
  const stop_guide = () => {
    sendMessage(
      JSON.stringify({
        device_name: "phd2",
        instruction: "stop_guide",
        params: [],
      })
    );
  };
  const { sendMessage, removeListener } = useEchoWebSocket(process_ws_message);
  React.useEffect(() => {
    check_phd2_status();
    return () => {
      removeListener(process_ws_message);
    };
  }, []);

  return (
    <div
      style={{
        zIndex: 20,
      }}
    >
      {/* control of the graph */}
      <div>
        <Container>
          {/* control of the graph */}
          <ButtonGroup className="mb-3">
            <Button onClick={open_guide_graph}>导星数据</Button>
            <Button onClick={open_calibration_result}>校准结果</Button>
          </ButtonGroup>
          {/* main control */}
          <Accordion>
            <Card>
              <Card.Header>
                <Card.Title as="h5">主控制面板</Card.Title>
              </Card.Header>
              <Card.Body>
                <ButtonGroup className="mb-3">
                  <Button onClick={start_exposure}>连续曝光</Button>
                  <Button onClick={start_calibration}>导星校准</Button>
                </ButtonGroup>
                <ButtonGroup className="mb-3">
                  <Button onClick={start_guide}>开始导星</Button>
                  <Button onClick={stop_guide} variant="danger">
                    停止
                  </Button>
                </ButtonGroup>
              </Card.Body>
            </Card>
          </Accordion>
          <Alert variant="info" show={guide_graph} onClose={close_guide_graph}>
            <Alert.Heading>导星数据</Alert.Heading>
          </Alert>
          <Alert
            variant="info"
            show={calibration_result}
            onClose={close_calibration_result}
          >
            <Alert.Heading>校准结果</Alert.Heading>
          </Alert>
        </Container>
      </div>
    </div>
  );
};

DevicePHD2SimpleControlPanel.displayName = "PHD2控制";

export default DevicePHD2SimpleControlPanel;
