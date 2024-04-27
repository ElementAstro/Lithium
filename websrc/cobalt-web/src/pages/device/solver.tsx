import React, { useState, useEffect } from "react";
import Container from "react-bootstrap/Container";
import Row from "react-bootstrap/Row";
import Col from "react-bootstrap/Col";
import Form from "react-bootstrap/Form";
import Button from "react-bootstrap/Button";
import ButtonGroup from "react-bootstrap/ButtonGroup";
import { Check, XCircle } from "react-bootstrap-icons";

import { useEchoWebSocket } from "../../utils/websocketProvider";

const DeviceSolverGeneralControlPanel = () => {
  const [connected, setConnected] = useState(false);
  const [solver, setSolver] = useState("astrometry"); // Default solver
  const [solveTimeout, setSolveTimeout] = useState(60); // Default solve timeout in seconds
  const [downsample, setDownsample] = useState(2); // Default downsample value
  const [showAdvancedOptions, setShowAdvancedOptions] = useState(false);

  const checkSolverStatus = () => {
    // Placeholder for checking the status of selected solver
  };

  const saveSettings = () => {
    // Placeholder for saving solver settings
  };

  const processWsMessage = (msg) => {
    // Placeholder for processing WebSocket messages
  };

  const { sendMessage, removeListener } = useEchoWebSocket(processWsMessage);

  useEffect(() => {
    checkSolverStatus();
    return () => {
      removeListener(processWsMessage);
    };
  }, []);

  return (
    <Container fluid className="mt-2">
      <Row>
        <Col md={6}>
          <Row>
            <div className="d-flex align-items-center justify-content-between">
              <h5 className="mb-0 mr-3">解析器设置</h5>
              {connected ? (
                <Check color="success" size={20} />
              ) : (
                <XCircle color="error" size={20} />
              )}
            </div>
          </Row>
          <Row className="mt-3">
            <Form.Group>
              <Form.Label>解析器选择</Form.Label>
              <Form.Control
                as="select"
                value={solver}
                onChange={(e) => setSolver(e.target.value)}
              >
                <option value="astrometry">Astrometry.net</option>
                <option value="astap">Astap</option>
                <option value="stellarsolver">StellarSolver</option>
              </Form.Control>
            </Form.Group>
          </Row>
          <Row className="mt-3">
            <Form.Group>
              <Form.Label>超时设置 (秒)</Form.Label>
              <Form.Control
                type="number"
                placeholder="Enter solve timeout"
                value={solveTimeout}
                onChange={(event) => setSolveTimeout(event.target.value)}
                min={0}
              />
            </Form.Group>
            <Form.Group>
              <Form.Label>降采样</Form.Label>
              <Form.Control as="select">
                <option value="1">1</option>
                <option value="2">2</option>
                <option value="4">4</option>
                <option value="8">8</option>
                <option value="auto">Auto (Only Astap Support)</option>
              </Form.Control>
            </Form.Group>
            {/*
                <ButtonGroup>
              <Button
                variant="light"
                onClick={() => setShowAdvancedOptions((prev) => !prev)}
              >
                {showAdvancedOptions ? "隐藏" : "显示"} 高级设置
              </Button>
              <Button variant="primary" onClick={saveSettings}>
                保存设置
              </Button>
            </ButtonGroup>
            {showAdvancedOptions && (
            <div>
              {solver === "astrometry" && (
                <Row>
                  <Form.Group>
                    <Form.Label>Astrometry.net API Key</Form.Label>
                    <Form.Control
                      type="text"
                      placeholder="Enter Astrometry.net API Key"
                    />
                  </Form.Group>
                </Row>
              )}
            </div>
          )}
                */}
          </Row>
        </Col>
        <Col md={6}>
          {/* Right side for specific solver parameters */}
          <div>
            {/* Specific solver parameters */}

            {solver === "astrometry" && (
              <div>
                {/* Astrometry.net parameters */}
                <p className="mb-2">Astrometry.net特有参数</p>
                <Form.Group>
                  <Form.Label>位深</Form.Label>
                  <Form.Control as="select">
                    <option value="8">8 bit</option>
                    <option value="12">12 bit</option>
                    <option value="16">16 bit</option>
                  </Form.Control>
                </Form.Group>
                <Form.Group>
                  <Form.Label>拉伸</Form.Label>
                  <Form.Control
                    type="number"
                    min={0}
                    max={100}
                    placeholder="stretch"
                  />
                </Form.Group>
                {/* Add more Astrometry.net parameters as needed */}
              </div>
            )}
            {solver === "astap" && (
              <div>
                {/* Astap parameters */}
                <Form.Group>
                  <Form.Label>Specific parameter A</Form.Label>
                  <Form.Control
                    type="text"
                    placeholder="Enter specific parameter A"
                  />
                </Form.Group>
                <Form.Group>
                  <Form.Label>Specific parameter B</Form.Label>
                  <Form.Control
                    type="text"
                    placeholder="Enter specific parameter B"
                  />
                </Form.Group>
                {/* Add more Astap parameters as needed */}
              </div>
            )}
            {solver === "stellarsolver" && (
              <div>
                {/* StellarSolver parameters */}
                <Form.Group>
                  <Form.Label>Specific parameter X</Form.Label>
                  <Form.Control
                    type="text"
                    placeholder="Enter specific parameter X"
                  />
                </Form.Group>
                <Form.Group>
                  <Form.Label>Specific parameter Y</Form.Label>
                  <Form.Control
                    type="text"
                    placeholder="Enter specific parameter Y"
                  />
                </Form.Group>
                {/* Add more StellarSolver parameters as needed */}
              </div>
            )}
          </div>
        </Col>
      </Row>
    </Container>
  );
};

export default DeviceSolverGeneralControlPanel;
