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

  // Add similar handleChange functions for other select inputs

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
            <Form.Group>
              <Form.Control
                as="select"
                size="sm"
                value={camera}
                onChange={handleCameraChange}
              >
                <option>相机</option>
              </Form.Control>
            </Form.Group>
          </Col>
          <Col sm={6}>
            <Form.Group>
              <Form.Control
                as="select"
                size="sm"
                value={telescope}
                onChange={handleCameraChange}
              >
                <option>望远镜</option>
              </Form.Control>
            </Form.Group>
          </Col>
          <Col sm={6}>
            <Form.Group>
              <Form.Control
                as="select"
                size="sm"
                value={filterwheel}
                onChange={handleCameraChange}
              >
                <option>滤波器</option>
              </Form.Control>
            </Form.Group>
          </Col>
          <Col sm={6}>
            <Form.Group>
              <Form.Control
                as="select"
                size="sm"
                value={focuser}
                onChange={handleCameraChange}
              >
                <option>焦点</option>
              </Form.Control>
            </Form.Group>
          </Col>
          <Col sm={6}>
            <Form.Group>
              <Form.Control
                as="select"
                size="sm"
                value={guider}
                onChange={handleCameraChange}
              >
                <option>巡视器</option>
              </Form.Control>
            </Form.Group>
          </Col>
          <Col sm={6}>
            <Form.Group>
              <Form.Control
                as="select"
                size="sm"
                value={solver}
                onChange={handleCameraChange}
              >
                <option>光学校正</option>
              </Form.Control>
            </Form.Group>
          </Col>

          {/* Add similar Form.Group components for other select inputs */}
        </Row>

        {/* Add similar Rows and Form.Group components for other select inputs */}

        <Row>
          <Col md={12}>
            <Button variant="outline-info" size="sm" type="submit">
              <i className="fas fa-light-switch-on"></i> 启动
            </Button>
          </Col>
          <Col md={12}>
            <Button
              variant="outline-success"
              size="sm"
              onClick={handleClientCommand}
            >
              <i className="fas fa-user"></i> 连接
            </Button>
          </Col>
        </Row>
      </Form>

      <Row>
        <Col md={12}>
          <Alert variant="info">{/* 服务器通知 */}</Alert>
        </Col>
      </Row>

      <Row>
        <Col md={12}>
          <Alert variant="warning">
            <p className="alert alert-warning">客户端未连接</p>
          </Alert>
        </Col>
      </Row>
    </Container>
  );
};

export default DeviceConnection;
