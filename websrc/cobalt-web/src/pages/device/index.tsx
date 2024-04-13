import React, { useState, useEffect } from "react";
import Tabs from "react-bootstrap/Tabs";
import Tab from "react-bootstrap/Tab";
import Button from "react-bootstrap/Button";
import Form from "react-bootstrap/Form";
import Alert from "react-bootstrap/Alert";
import Col from "react-bootstrap/Col";
import Container from "react-bootstrap/Container";
import Row from "react-bootstrap/Row";

import {
  Camera,
  DeviceHdd,
  Filter,
  Rulers,
  Star,
  XCircle,
} from "react-bootstrap-icons";

import DeviceCameraGeneralControlPanel from "./camera";
import DeviceFilterGeneralControlPanel from "./filterwheel";
import DeviceFocuserGeneralControlPanel from "./focuser";
import DeviceGuiderGeneralControlPanel from "./guider";
import DevicePHD2GeneralControlPanel from "./phd2";
import DeviceTelescopeGeneralControlPanel from "./telescope";
import DeviceSolverGeneralControlPanel from "./solver";

import DeviceCameraDetailedControlPanel from "./detail/camera";
import DeviceTelescopeDetailedControlPanel from "./detail/telescope";
import DeviceFilterDetailedControlPanel from "./detail/filterwheel";
import DeviceFocuserDetailedControlPanel from "./detail/focuser";
import DeviceGuiderDetailedControlPanel from "./detail/guider";
import DevicePHD2DetailedControlPanel from "./detail/phd2";

import DeviceAdvancedControlModal from "../../components/device/modal";

const DeviceControlPanelPage = () => {
  const device_disable_state = [false, false, false, false, false, false];
  const [show_detail, set_show_detail] = useState(false);
  // const [show_alert, set_show_alert] = useState(false);
  // const [activeTab, setActiveTab] = useState("camera");
  const [showAdvancedMode, setShowAdvancedMode] = useState(false);

  const handleShowAdvancedMode = () => {
    setShowAdvancedMode(true);
  };

  const handleCloseAdvancedMode = () => {
    setShowAdvancedMode(false);
  };

  const handlAdvanceModeOk = () => {
    setShowAdvancedMode(false);
    set_show_detail(true);
  };

  const check_device_connection = (device_key: string) => {
    // Replace with your own logic
    if (device_key === "camera") {
      return true;
    } else if (device_key === "telescope") {
      return true;
    } else if (device_key === "filter") {
      return true;
    } else if (device_key === "focuser") {
      return true;
    } else if (device_key === "guider") {
      return true;
    } else if (device_key === "phd2") {
      return true;
    } else if (device_key == "solver") {
      return true;
    }
  };

  const show_device_icon_if_not_connected = (device_key: string) => {
    // Replace with your own logic
    if (device_key === "camera") {
      return (
        <Container>
          <Alert variant="danger">
            <Alert.Heading>
              <Camera color="disabled" />
              相机未连接
            </Alert.Heading>
            <p>相机未连接</p>
          </Alert>
        </Container>
      );
    } else if (device_key === "telescope") {
      return (
        <Container>
          <Alert variant="danger">
            <Alert.Heading>
              <Star color="disabled" />
              赤道仪未连接
            </Alert.Heading>
            <p>赤道仪未连接</p>
          </Alert>
        </Container>
      );
    } else if (device_key === "filter") {
      return (
        <Container>
          <Alert variant="danger">
            <Alert.Heading>
              <Filter color="disabled" />
              滤镜轮未连接
            </Alert.Heading>
            <p>滤镜轮未连接</p>
          </Alert>
        </Container>
      );
    } else if (device_key === "focuser") {
      return (
        <Container>
          <Alert variant="danger">
            <Alert.Heading>
              <DeviceHdd color="disabled" />
              电调未连接
            </Alert.Heading>
            <p>电调未连接</p>
          </Alert>
        </Container>
      );
    } else if (device_key === "guider") {
      return (
        <Container>
          <Alert variant="danger">
            <Alert.Heading>
              <Rulers color="disabled" />
              导星未连接
            </Alert.Heading>
            <p>导星未连接</p>
          </Alert>
        </Container>
      );
    } else if (device_key === "phd2") {
      return (
        <Container>
          <Alert variant="danger">
            <Alert.Heading>
              <XCircle color="disabled" />
              PHD2未连接
            </Alert.Heading>
            <p>PHD2未连接</p>
          </Alert>
        </Container>
      );
    } else if (device_key == "solver") {
      return (
        <Container>
          <Alert variant="danger">
            <Alert.Heading>
              <XCircle color="disabled" />
              Solver未连接
            </Alert.Heading>
            <p>Solver未连接</p>
          </Alert>
        </Container>
      );
    }
  };

  useEffect(() => {
    // Replace with your own logic
  }, []);

  return (
    <Row lg={12}>
      <Col sm={2}>
        <Row className="justify-content-center ml-1 mt-3">
          <Col>
            <div className="d-grid gap-2 ml-1">
              <Button
                variant={show_detail ? "outline-primary" : "primary"}
                onClick={() => set_show_detail(false)}
                className="mr-2"
              >
                基础参数
              </Button>
            </div>
          </Col>
        </Row>
        <Row className="justify-content-center ml-1 mt-3">
          <div className="d-grid gap-2">
            <Button
              variant={show_detail ? "danger" : "outline-danger"}
              onClick={() => setShowAdvancedMode(true)}
            >
              详细配置
            </Button>
          </div>
        </Row>

        <Row md={"auto"}>
          <DeviceAdvancedControlModal
            showModal={showAdvancedMode}
            handleCloseModal={handleCloseAdvancedMode}
            handleOkModal={handlAdvanceModeOk}
          />
        </Row>
      </Col>
      <Col sm={10}>
        <Tabs defaultActiveKey="camera">
          <Tab
            eventKey="camera"
            // disabled={!check_device_connection("camera")}
            title="主相机"
          >
            {
              //show_device_icon_if_not_connected("camera")
            }
            {!show_detail ? (
              <DeviceCameraGeneralControlPanel />
            ) : (
              <DeviceCameraDetailedControlPanel />
            )}
          </Tab>
          <Tab
            eventKey="telescope"
            // disabled={!check_device_connection("telescope")}
            title="赤道仪"
          >
            {
              //show_device_icon_if_not_connected("telescope")
            }
            {!show_detail ? (
              <DeviceTelescopeGeneralControlPanel />
            ) : (
              <DeviceTelescopeDetailedControlPanel />
            )}
          </Tab>
          <Tab
            eventKey="filter"
            // disabled={!check_device_connection("filter")}
            title="滤镜轮"
          >
            {
              //show_device_icon_if_not_connected("focuser")
            }
            {!show_detail ? (
              <DeviceFilterGeneralControlPanel />
            ) : (
              <DeviceFilterDetailedControlPanel />
            )}
          </Tab>
          <Tab
            eventKey="focuser"
            // disabled={!check_device_connection("focuser")}
            title="电调"
          >
            {
              //show_device_icon_if_not_connected("filter")
            }
            {!show_detail ? (
              <DeviceFocuserGeneralControlPanel />
            ) : (
              <DeviceFocuserDetailedControlPanel />
            )}
          </Tab>
          <Tab
            eventKey="guider"
            // disabled={!check_device_connection("guider")}
            title="导星"
          >
            {
              //show_device_icon_if_not_connected("guider")//
            }
            {!show_detail ? (
              <DeviceGuiderGeneralControlPanel />
            ) : (
              <DeviceGuiderDetailedControlPanel />
            )}
          </Tab>
          <Tab
            eventKey="phd2"
            // disabled={!check_device_connection("phd2")}
            title="PHD2导星控制"
          >
            {
              //show_device_icon_if_not_connected("phd2")
            }
            {!show_detail ? (
              <DevicePHD2GeneralControlPanel />
            ) : (
              <DevicePHD2DetailedControlPanel />
            )}
          </Tab>
          <Tab
            eventKey="solver"
            // disabled={!check_device_connection("solver")}
            title="解析"
          >
            {
              //show_device_icon_if_not_connected("solver")
            }
            {!show_detail ? (
              <DeviceSolverGeneralControlPanel />
            ) : (
              <DeviceSolverDetailedControlPanel />
            )}
          </Tab>
        </Tabs>
      </Col>
    </Row>
  );
};

export default DeviceControlPanelPage;
