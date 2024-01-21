import React, { useState, useEffect } from "react";
import Container from "react-bootstrap/Container";
import Row from "react-bootstrap/Row";
import Col from "react-bootstrap/Col";
import Form from "react-bootstrap/Form";
import Button from "react-bootstrap/Button";
import ButtonGroup from "react-bootstrap/ButtonGroup";
import {
  Camera,
  ClockHistory,
  Check,
  XCircle,
  ArrowsMove,
} from "react-bootstrap-icons";

import { useEchoWebSocket } from "../../utils/websocketProvider";

const DevicePHD2GeneralControlPanel = () => {
  // 要显示phd2的数据流。
  // 要有一些设备的控制参数。比如重启，链接设备等等。
  // 还有可以手动控制校准，可以打开图像查看最新一帧图片

  // ui data
  const [connected, setConnected] = useState(false);
  const [guiderCcd, setGuiderCcd] = useState("");
  const [telescopeName, setTelescopeName] = useState("");
  const [inputExposureTime, setInputExposureTime] = useState("1000");
  const [exposureTime, setExposureTime] = useState(1000);
  const [showGuideGraph, setShowGuideGraph] = useState(false);
  const [showCalibrationResult, setShowCalibrationResult] = useState(false);

  // data
  const [calibrationData, setCalibrationData] = useState({});

  // call functions
  const checkPhd2Status = () => {};

  // const restart_phd2 = () => {

  // }

  const changePhd2ExposureTime = (exposureTime) => {};

  const startExposure = () => {};

  const startGuide = () => {};

  const stopGuide = () => {};

  const startCalibration = () => {};

  const checkCalibrationResult = () => {};

  const processWsMessage = (msg) => {
    if (msg.deviceName === "phd2_event") {
    } else if (msg.deviceName === "PHD2_response") {
    }
  };

  const { sendMessage, removeListener } = useEchoWebSocket(processWsMessage);

  useEffect(() => {
    checkPhd2Status();
    return () => {
      removeListener(processWsMessage);
    };
  }, []);

  return (
    <Container fluid className="mt-2">
      <Row>
        <Col md={6}>
          <div className="d-flex align-items-center">
            <h5 className="mb-0 mr-2">导星系统PHD2</h5>
            {connected ? (
              <Check color="success" size={20} />
            ) : (
              <XCircle color="error" size={20} />
            )}
          </div>

          <p className="mb-0">导星相机：</p>
          <p className="mb-0">赤道仪：</p>
          {/* <Typography level='body-md' gutterBottom>重启导星系统</Typography> */}
        </Col>

        <Col md={6}>
          <p className="mb-2">
            曝光时间{" "}
            <span className="text-success font-weight-bold">
              {exposureTime}ms
            </span>
          </p>
          <Form.Group>
            <Form.Label>设置曝光时间</Form.Label>
            <Form.Control
              type="number"
              placeholder="请输入曝光时间"
              value={inputExposureTime}
              onChange={(event) => setInputExposureTime(event.target.value)}
              min={0}
            />
          </Form.Group>
          <ButtonGroup color="success">
            <Button variant="light" className="d-flex align-items-center">
              <Camera size={20} className="mr-2" />
              连续曝光
            </Button>
            <Button variant="light" className="d-flex align-items-center">
              <ArrowsMove size={20} className="mr-2" />
              开始导星
            </Button>
            <Button variant="light" className="d-flex align-items-center">
              <XCircle size={20} className="mr-2" />
              停止
            </Button>
          </ButtonGroup>

          <p className="mt-3 mb-0">已校准</p>
          <ButtonGroup color="primary">
            <Button>开始校准</Button>
            <Button>查看校准结果</Button>
          </ButtonGroup>

          <p className="mt-3 mb-0">导星中</p>
        </Col>
      </Row>
    </Container>
  );
};

export default DevicePHD2GeneralControlPanel;
