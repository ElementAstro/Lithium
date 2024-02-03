import React, { useEffect, useState } from "react";
import { Container, Row, Col, Button, Form } from "react-bootstrap";
import CameraView from "./CameraView";
import CaptureButton from "./CaptureButton";
import Sidebar from "./Sidebar";
import Toolbar from "./Toolbar";
import FloatingUI from "./FloatingUI";
import SystemPanel from "./SystemPanel";
import FloatingWindow from "./FloatingWindow";

const Dashboard = () => {
  const [sidebarVisible, setSidebarVisible] = useState(false);
  const [brightness, setBrightness] = useState(50);
  const [contrast, setContrast] = useState(50);
  const [saturation, setSaturation] = useState(50);
  const [toolbarVisible, setToolbarVisible] = useState(false);

  const handleCapture = () => {
    // 处理捕捉逻辑
  };

  const handleBrightnessChange = (event) => {
    setBrightness(event.target.value);
  };

  const handleContrastChange = (event) => {
    setContrast(event.target.value);
  };

  const handleSaturationChange = (event) => {
    setSaturation(event.target.value);
  };

  const handleToggleSidebar = () => {
    setSidebarVisible(!sidebarVisible);
  };

  const handleToggleToolbar = () => {
    setToolbarVisible(!toolbarVisible);
  };

  function disableScroll() {
    document.body.style.overflow = "hidden";
  }

  function enableScroll() {
    document.body.style.overflow = "auto";
  }

  // 在组件挂载时添加事件监听器
  useEffect(() => {
    disableScroll();

    // 在组件卸载时删除事件监听器
    return () => {
      enableScroll();
    };
  }, []);

  return (
    <Container fluid className="camera-screen">
      <Row className="top-bar">
        <Col className="d-flex align-items-center">
          <Button variant="secondary" onClick={handleToggleSidebar}>
            调整参数
          </Button>
          <Button variant="secondary" onClick={handleToggleToolbar}>
            {toolbarVisible ? "隐藏工具栏" : "显示工具栏"}
          </Button>
        </Col>
      </Row>

      <Row className="camera-view-row">
        <Col className="d-flex align-items-center justify-content-center">
          <CameraView />
        </Col>
      </Row>

      <Row className="controls-row">
        <Col className="d-flex align-items-center justify-content-center">
          <CaptureButton onCapture={handleCapture} />
        </Col>
      </Row>

      <Sidebar
        visible={sidebarVisible}
        brightness={brightness}
        contrast={contrast}
        saturation={saturation}
        onBrightnessChange={handleBrightnessChange}
        onContrastChange={handleContrastChange}
        onSaturationChange={handleSaturationChange}
      />

      <Toolbar visible={toolbarVisible} onToggle={handleToggleToolbar} />
      <FloatingUI />
      <FloatingWindow>
        <SystemPanel />
      </FloatingWindow>
    </Container>
  );
};

export default Dashboard;
