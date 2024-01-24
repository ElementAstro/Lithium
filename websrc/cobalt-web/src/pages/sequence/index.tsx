import React, { useState } from "react";
import { Tab, Tabs, Container, Row, Col, Button } from "react-bootstrap";
import {
  BoxArrowInRight,
  PencilSquare,
  ListCheck,
  FileText,
} from "react-bootstrap-icons";

import PAAEditPage from "./editor";
import PAAChoosePage from "./choose";
import PAADebugLogging from "./logging";
import SimpleScriptEditor from "./script";

interface TabPanelProps {
  children?: React.ReactNode;
  index: number;
  value: number;
}

function TabPanel(props: TabPanelProps) {
  const { children, value, index } = props;

  return (
    <div role="tabpanel" hidden={value !== index}>
      {value === index && (
        <Container>
          <Row>
            <Col>{children}</Col>
          </Row>
        </Container>
      )}
    </div>
  );
}

const PAAIndexPage: React.FC = () => {
  const [settingMode, setSettingMode] = useState(0);

  const handleModeChange = (newMode: number) => {
    setSettingMode(newMode);
  };

  return (
    <Container fluid>
      <Row>
        <Col>
          <Tabs variant="tabs" className="mb-3" defaultActiveKey="choose">
            <Tab
              eventKey="choose"
              title={
                <>
                  <ListCheck size={18} /> 选择拍摄目标
                </>
              }
            >
              <PAAChoosePage />
            </Tab>
            <Tab
              eventKey="editor"
              title={
                <>
                  <PencilSquare size={18} /> 编辑拍摄脚本
                </>
              }
            >
              <SimpleScriptEditor />
            </Tab>
            <Tab
              eventKey="script"
              title={
                <>
                  <BoxArrowInRight size={18} /> 查看拍摄流程
                </>
              }
            >
              <PAAEditPage />
            </Tab>
            <Tab
              eventKey="logging"
              title={
                <>
                  <FileText size={18} /> 查看Debug Log
                </>
              }
            >
              <PAADebugLogging />
            </Tab>
          </Tabs>
        </Col>
      </Row>
    </Container>
  );
};

export default PAAIndexPage;
