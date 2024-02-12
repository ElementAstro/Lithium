import React, { useEffect, useState } from "react";
import { Container, Row, Col, Button, Form } from "react-bootstrap";
import CameraView from "./CameraView";
import { SCaptureButton } from "./CaptureButton";
import Toolbar from "./Toolbar";
import FloatingUI from "./FloatingUI";
import SystemPanel from "./SystemPanel";
import FloatingWindow from "./FloatingWindow";
import { CameraContainer } from "./style/main";
import { Footer } from "./Footer";
import {
  RightSidebar,
  FullHeightContainer,
  StyledRow,
} from "./style/RightSidebar";
import { ArrowRepeat, Crop, GearFill, XCircle } from "react-bootstrap-icons";
import { SquareButton } from "./style/SquareButton";

const Dashboard = () => {
  const [sidebarVisible, setSidebarVisible] = useState(false);
  const [toolbarVisible, setToolbarVisible] = useState(false);

  const [modalShow, setExpModalShow] = useState(false);

  const systemInfo = "© 2024 Lithium & Cobalt. All rights reserved.";

  const handleCapture = () => {
    // 处理捕捉逻辑
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
    <>
      <CameraContainer fluid>
        <Row className="camera-view-row" fluid>
          <Col className="d-flex align-items-center justify-content-center">
            <CameraView />
            <RightSidebar>
              <FullHeightContainer>
                <StyledRow>
                  <SquareButton>
                    <Crop /> 裁剪
                  </SquareButton>
                </StyledRow>
                <StyledRow>
                  <Button>
                    <ArrowRepeat /> 刷新
                  </Button>
                </StyledRow>
                <StyledRow>
                  <SCaptureButton onClick={handleCapture}>
                    Capture
                  </SCaptureButton>
                </StyledRow>
                <StyledRow>
                  <Button variant="secondary" onClick={handleToggleSidebar}>
                    调整参数
                  </Button>
                </StyledRow>
                <StyledRow>
                  <Button variant="secondary" onClick={handleToggleToolbar}>
                    {toolbarVisible ? (
                      <XCircle size={20} />
                    ) : (
                      <GearFill size={20} />
                    )}
                  </Button>
                </StyledRow>
              </FullHeightContainer>
            </RightSidebar>
          </Col>
        </Row>

        <Toolbar visible={toolbarVisible} onToggle={handleToggleToolbar} />
        <FloatingUI />
        <FloatingWindow>
          <SystemPanel />
        </FloatingWindow>
      </CameraContainer>
      <Footer systemInfo={systemInfo} />
    </>
  );
};

export default Dashboard;
