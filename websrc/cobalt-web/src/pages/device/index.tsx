import React, { useState, useEffect } from "react";
import Tabs from "react-bootstrap/Tabs";
import Tab from "react-bootstrap/Tab";
import Button from "react-bootstrap/Button";
import Form from "react-bootstrap/Form";
import Alert from "react-bootstrap/Alert";
import { Camera, Question, XCircle } from "react-bootstrap-icons";

import DeviceCameraGeneralControlPanel from "./camera";
import DeviceFilterGeneralControlPanel from "./filterwheel";
import DeviceFocuserGeneralControlPanel from "./focuser";
// import DeviceGuiderGeneralControlPanel from "./guider";
import DevicePHD2GeneralControlPanel from "./phd2";
import DeviceTelescopeGeneralControlPanel from "./telescope";

import DeviceCameraDetailedControlPanel from "./detail/camera";
import DeviceTelescopeDetailedControlPanel from "./detail/telescope";
import DeviceFilterDetailedControlPanel from "./detail/filterwheel";
import DeviceFocuserDetailedControlPanel from "./detail/focuser";
import DeviceGuiderDetailedControlPanel from "./detail/guider";
import DevicePHD2DetailedControlPanel from "./detail/phd2";
import { Col, Container, Row } from "react-bootstrap";

const DeviceControlPanelPage = () => {
  const device_disable_state = [false, false, false, false, false, false];
  const [show_detail, set_show_detail] = useState(false);
  const [show_alert, set_show_alert] = useState(false);
  const [activeTab, setActiveTab] = useState("camera");

  const handleAlertClose = () => {
    set_show_alert(false);
    set_show_detail(true);
  };

  const check_device_connection = (device_key) => {
    // Replace with your own logic
    if (device_key === "camera") {
      return true;
    } else if (device_key === "telescope") {
      return true;
    } else if (device_key === "filter") {
      return true;
    } else if (device_key === "focus") {
      return true;
    } else if (device_key === "guider") {
      return true;
    } else if (device_key === "phd2") {
      return true;
    }
  };

  const show_device_icon_if_not_connected = (device_key) => {
    // Replace with your own logic
    if (device_key === "camera") {
      return (
        <Container>
          <Camera color="disabled" />
          <Alert
            variant="danger"
            onClose={handleAlertClose}
            dismissible
          >
            <Alert.Heading>设备未连接</Alert.Heading>
            <p>设备未连接</p>
          </Alert>
        </Container>
      );
    } else if (device_key === "telescope") {
      return <XCircle color="error" />;
    } else if (device_key === "filter") {
      return <Question color="disabled" />;
    } else if (device_key === "focuser") {
      return <XCircle color="error" />;
    } else if (device_key === "guider") {
      return <Question color="disabled" />;
    } else if (device_key === "phd2") {
      return <XCircle color="error" />;
    }
  };

  useEffect(() => {
    // Replace with your own logic
  }, []);

  return (
    <Container>
      <Row>
        <Col md={3}>
          <div className="d-flex flex-row">
            <div className="p-3 w-25 border-end">
              <Form>
                <Form.Check
                  type="radio"
                  label="基础参数"
                  name="select-set-format"
                  value="basic"
                  checked={!show_detail}
                  onChange={() => set_show_detail(false)}
                />
                <Form.Check
                  type="radio"
                  label="详细配置"
                  name="select-set-format"
                  value="detailed"
                  checked={show_detail}
                  onChange={() => set_show_alert(true)}
                />
              </Form>
              <Alert
                show={show_alert}
                variant="warning"
                onClose={handleAlertClose}
                dismissible
              >
                <Alert.Heading>警告</Alert.Heading>
                <p>
                  详细配置是专业模式，除非必要不建议用户修改！修改会导致不可预料的故障！是否确认进入？
                </p>
                <div className="d-flex justify-content-end">
                  <Button onClick={handleAlertClose} variant="outline-warning">
                    确认
                  </Button>
                </div>
              </Alert>
            </div>
            <div className="p-3 w-75">
              <Tab.Content>
                <Tab.Pane eventKey={0}>
                  {!show_detail ? (
                    <DeviceCameraGeneralControlPanel />
                  ) : (
                    <DeviceCameraDetailedControlPanel />
                  )}
                </Tab.Pane>
                <Tab.Pane eventKey={1}>
                  {!show_detail ? (
                    <DeviceTelescopeGeneralControlPanel />
                  ) : (
                    <DeviceTelescopeDetailedControlPanel />
                  )}
                </Tab.Pane>
                <Tab.Pane eventKey={2}>
                  {!show_detail ? (
                    <DeviceFocuserGeneralControlPanel />
                  ) : (
                    <DeviceFocuserDetailedControlPanel />
                  )}
                </Tab.Pane>
                <Tab.Pane eventKey={3}>
                  {!show_detail ? (
                    <DeviceFilterGeneralControlPanel />
                  ) : (
                    <DeviceFilterDetailedControlPanel />
                  )}
                </Tab.Pane>
                <Tab.Pane eventKey={4}></Tab.Pane>
                <Tab.Pane eventKey={5}>
                  {!show_detail ? (
                    <DevicePHD2GeneralControlPanel />
                  ) : (
                    <DevicePHD2DetailedControlPanel />
                  )}
                </Tab.Pane>
              </Tab.Content>
            </div>
          </div>
        </Col>
        <Col md={9}>
          <Tabs
            defaultActiveKey="camera"
            className="d-flex justify-content-between align-items-center bg-light rounded-xl p-2 mb-3"
          >
            <Tab
              eventKey="camera"
              // disabled={!check_device_connection("camera")}
              title="主相机"
            >
              {show_device_icon_if_not_connected("camera")}
            </Tab>
            <Tab
              eventKey="telescope"
              // disabled={!check_device_connection("telescope")}
              title="赤道仪"
            >
              {show_device_icon_if_not_connected("telescope")}
            </Tab>
            <Tab
              eventKey="filter"
              // disabled={!check_device_connection("filter")}
              title="滤镜轮"
            >
              {show_device_icon_if_not_connected("filter")}
            </Tab>
            <Tab
              eventKey="focuser"
              // disabled={!check_device_connection("focuser")}
              title="电调"
            >
              {show_device_icon_if_not_connected("focuser")}
            </Tab>
            <Tab
              eventKey="guider"
              // disabled={!check_device_connection("guider")}
              title="导星"
            >
              {show_device_icon_if_not_connected("guider")}
            </Tab>
            <Tab
              eventKey="phd2"
              // disabled={!check_device_connection("phd2")}
              title="PHD2导星控制"
            >
              {show_device_icon_if_not_connected("phd2")}
            </Tab>
          </Tabs>
        </Col>
      </Row>
    </Container>
  );
};

export default DeviceControlPanelPage;
