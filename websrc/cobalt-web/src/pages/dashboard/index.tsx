import React, { useState } from "react";
import styled from "styled-components";
import {
  Container,
  Row,
  Col,
  Button,
  Form,
  InputGroup,
  FormControl,
} from "react-bootstrap";

const Sidebar = styled.div`
  position: fixed;
  top: 60px;
  right: ${(props) => (props.visible ? "0" : "-250px")};
  width: 250px;
  height: 100%;
  background-color: #f8f9fa;
  border-left: 1px solid #dee2e6;
  box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
  transition: all 0.3s ease-in-out;
`;

const Toolbar = styled.div`
  position: fixed;
  top: 60px;
  left: ${(props) => (props.visible ? "0" : "-250px")};
  width: 250px;
  height: calc(100% - 60px);
  background-color: #f8f9fa;
  border-right: 1px solid #dee2e6;
  box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
  transition: all 0.3s ease-in-out;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: flex-start;
  overflow-y: scroll;
`;

const ToolbarButton = styled(Button)`
  margin-bottom: 10px;
`;

const Dashboard = () => {
  const [sidebarVisible, setSidebarVisible] = useState(false);
  const [brightness, setBrightness] = useState(50);
  const [contrast, setContrast] = useState(50);
  const [saturation, setSaturation] = useState(50);
  const [toolbarVisible, setToolbarVisible] = useState(false);
  const [showToolbar, setShowToolbar] = useState(false);

  const handleCapture = () => {
    // 处理拍摄逻辑
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
          <div className="camera-view"></div>
        </Col>
      </Row>
      <Row className="h-100">
        <Col className="d-flex align-items-center justify-content-center">
          <Button variant="primary" onClick={handleCapture}>
            拍摄
          </Button>
          <Button variant="secondary" onClick={handleToggleSidebar}>
            调整参数
          </Button>
          <Button variant="secondary" onClick={handleToggleToolbar}>
            {showToolbar ? "隐藏工具栏" : "显示工具栏"}
          </Button>
        </Col>
      </Row>
      <Sidebar visible={sidebarVisible}>
        <Form>
          <Form.Group controlId="brightness">
            <Form.Label>亮度</Form.Label>
            <InputGroup>
              <FormControl
                type="range"
                min="0"
                max="100"
                value={brightness}
                onChange={handleBrightnessChange}
              />
              <FormControl value={brightness} readOnly />
            </InputGroup>
          </Form.Group>
          <Form.Group controlId="contrast">
            <Form.Label>对比度</Form.Label>
            <InputGroup>
              <FormControl
                type="range"
                min="0"
                max="100"
                value={contrast}
                onChange={handleContrastChange}
              />
              <FormControl value={contrast} readOnly />
            </InputGroup>
          </Form.Group>
          <Form.Group controlId="saturation">
            <Form.Label>饱和度</Form.Label>
            <InputGroup>
              <FormControl
                type="range"
                min="0"
                max="100"
                value={saturation}
                onChange={handleSaturationChange}
              />
              <FormControl value={saturation} readOnly />
            </InputGroup>
          </Form.Group>
        </Form>
      </Sidebar>
      <Toolbar visible={true}>
        <Button variant="primary">工具0</Button>
        {/* 添加其他工具按钮 */}
      </Toolbar>
    </Container>
  );
};

export default Dashboard;
