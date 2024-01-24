// Dashboard.jsx
import React, { useState } from "react";
import { Container, Row, Col, Button } from "react-bootstrap";
import CameraView from "./CameraView";
import CaptureButton from "./CaptureButton";
import Sidebar from "./Sidebar";
import Toolbar from "./Toolbar";
import FloatingUI from "./FloatingUI";

const Dashboard = () => {
  const [sidebarVisible, setSidebarVisible] = useState(false);
  const [brightness, setBrightness] = useState(50);
  const [contrast, setContrast] = useState(50);
  const [saturation, setSaturation] = useState(50);
  const [toolbarVisible, setToolbarVisible] = useState(false);

  const handleCapture = () => {
    // handle capture logic
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

  return (
    <Container fluid className="camera-screen">
      <Row className="h-100">
        <Col className="d-flex align-items-center justify-content-center">
          <CameraView />
        </Col>
      </Row>

      <Row className="h-100">
        <Col className="d-flex align-items-center justify-content-center">
          <CaptureButton onCapture={handleCapture} />
          <Button variant="secondary" onClick={handleToggleSidebar}>
            调整参数
          </Button>
          <Button variant="secondary" onClick={handleToggleToolbar}>
            {toolbarVisible ? "隐藏工具栏" : "显示工具栏"}
          </Button>
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
    </Container>
  );
};

export default Dashboard;
