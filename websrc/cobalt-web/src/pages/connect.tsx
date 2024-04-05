import React, { useState, useEffect } from "react";
import {
  Container,
  Row,
  Col,
  Button,
  Alert,
  Form,
  FormControl,
} from "react-bootstrap";
import { Plug, Power } from "react-bootstrap-icons";

const DeviceConnection = () => {
  const [camera, setCamera] = useState("");
  const [telescope, setTelescope] = useState("");
  const [filterwheel, setFilterwheel] = useState("");
  const [focuser, setFocuser] = useState("");
  const [guider, setGuider] = useState("");
  const [solver, setSolver] = useState("");
  const [plugins, setPlugins] = useState([]);

  useEffect(() => {
    // Send a request to the backend to get device types in JSON format
    // Update the state variables accordingly
    const fetchDeviceTypes = async () => {
      try {
        const response = await fetch("/api/device-types");
        if (!response.ok) {
          throw new Error("Failed to fetch device types");
        }
        const data = await response.json();
        setCamera(data.camera);
        setTelescope(data.telescope);
        setFilterwheel(data.filterwheel);
        setFocuser(data.focuser);
        setGuider(data.guider);
        setSolver(data.solver);
        setPlugins(data.plugins);
      } catch (error) {
        console.error(error);
      }
    };
    fetchDeviceTypes();
  }, []);

  const handleCameraChange = (e) => {
    setCamera(e.target.value);
  };

  const handleServerCommand = () => {
    // Perform server command action
  };

  const handleClientCommand = () => {
    // Perform client command action
  };

  const handleTelescopeChange = (e) => {
    setTelescope(e.target.value);
  };

  const handleFilterwheelChange = (e) => {
    setFilterwheel(e.target.value);
  };

  const handleFocuserChange = (e) => {
    setFocuser(e.target.value);
  };

  const handleGuiderChange = (e) => {
    setGuider(e.target.value);
  };

  const handleSolverChange = (e) => {
    setSolver(e.target.value);
  };

  const handleSubmit = async (e) => {
    e.preventDefault();
    const jsonData = {
      camera,
      telescope,
      filterwheel,
      focuser,
      guider,
      solver,
      plugins,
    };
    try {
      const response = await fetch("/api/device-connection", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify(jsonData),
      });
      if (!response.ok) {
        throw new Error("Failed to send device connection data");
      }
      // Handle successful response
    } catch (error) {
      console.error(error);
    }
  };

  return (
    <Container>
      <Form onSubmit={handleSubmit}>
        <Row>
          <Col sm={6}>
            <Form.Group controlId="cameraSelect">
              <Form.Label>相机</Form.Label>
              <Form.Control
                as="select"
                size="sm"
                value={camera}
                onChange={handleCameraChange}
              >
                <option>选择相机</option>
                {/* 添加相机选项 */}
              </Form.Control>
            </Form.Group>
          </Col>
          <Col sm={6}>
            <Form.Group controlId="telescopeSelect">
              <Form.Label>赤道仪</Form.Label>
              <Form.Control
                as="select"
                size="sm"
                value={telescope}
                onChange={handleTelescopeChange}
              >
                <option>选择赤道仪</option>
                {/* 添加赤道仪选项 */}
              </Form.Control>
            </Form.Group>
          </Col>
        </Row>
        <Row>
          <Col sm={6}>
            <Form.Group controlId="filterwheelSelect">
              <Form.Label>滤镜轮</Form.Label>
              <Form.Control
                as="select"
                size="sm"
                value={filterwheel}
                onChange={handleFilterwheelChange}
              >
                <option>选择滤镜轮</option>
                {/* 添加滤镜轮选项 */}
              </Form.Control>
            </Form.Group>
          </Col>
          <Col sm={6}>
            <Form.Group controlId="focuserSelect">
              <Form.Label>电调</Form.Label>
              <Form.Control
                as="select"
                size="sm"
                value={focuser}
                onChange={handleFocuserChange}
              >
                <option>选择电调</option>
                {/* 添加电调选项 */}
              </Form.Control>
            </Form.Group>
          </Col>
        </Row>
        <Row>
          <Col sm={6}>
            <Form.Group controlId="guiderSelect">
              <Form.Label>导星</Form.Label>
              <Form.Control
                as="select"
                size="sm"
                value={guider}
                onChange={handleGuiderChange}
              >
                <option>选择导星</option>
                {/* 添加导星选项 */}
              </Form.Control>
            </Form.Group>
          </Col>
          <Col sm={6}>
            <Form.Group controlId="solverSelect">
              <Form.Label>板解器</Form.Label>
              <Form.Control
                as="select"
                size="sm"
                value={solver}
                onChange={handleSolverChange}
              >
                <option>选择板解器</option>
                {/* 添加板解器选项 */}
              </Form.Control>
            </Form.Group>
          </Col>
        </Row>
        <Row>
          <Col>
            <Button variant="primary" size="sm" type="submit">
              <Power size={20} /> 启动
            </Button>{" "}
            <Button variant="success" size="sm" onClick={handleClientCommand}>
              <Plug size={20} /> 连接
            </Button>
          </Col>
        </Row>
      </Form>

      <Row>
        <Col>
          <Alert variant="info">{/* 服务器通知 */}</Alert>
        </Col>
      </Row>

      <Row>
        <Col>
          <Alert variant="warning">
            <i className="fas fa-exclamation-triangle"></i> 客户端未连接
          </Alert>
        </Col>
      </Row>
    </Container>
  );
};

export default DeviceConnection;
