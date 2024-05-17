import React, { useState } from "react";
import { Button, Modal, Form } from "react-bootstrap";
import { Save } from "react-bootstrap-icons";

const ConfigCreationModal = () => {
  const [show, setShow] = useState(false);
  const handleClose = () => setShow(false);
  const handleShow = () => setShow(true);

  // Initial state for the configuration values
  const [config, setConfig] = useState({
    server: {
      configpath: "config.json",
      host: "0.0.0.0",
      maxprocess: 10,
      maxthread: 10,
      modulepath: "modules",
      port: 3000,
    },
    terminal: {
      enabled: true,
    },
  });

  // Function to handle changes in the configuration values
  const handleChange = (e) => {
    const { name, value, checked } = e.target;
    const section = name.split(".")[0];
    const key = name.split(".")[1];
    setConfig((prevConfig) => ({
      ...prevConfig,
      [section]: {
        ...prevConfig[section],
        [key]: value === "true" || value === "false" ? checked : value,
      },
    }));
  };

  // Function to submit the configuration
  const handleSubmit = (e) => {
    e.preventDefault();
    console.log(config); // You can send the config to the server or perform other actions here
    handleClose(); // Close the modal after submission
  };

  return (
    <>
    <div className="d-grid gap-2 mt-4">
        <Button variant="primary" onClick={handleShow}>
        创建新的配置文件
      </Button>
    </div>


      <Modal show={show} onHide={handleClose}>
        <Modal.Header closeButton>
          <Modal.Title>Configuration Creation</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form onSubmit={handleSubmit}>
            {/* Server Configuration Section */}
            <Form.Group controlId="configpath">
              <Form.Label>Config Path</Form.Label>
              <Form.Control
                type="text"
                name="server.configpath"
                value={config.server.configpath}
                onChange={handleChange}
              />
            </Form.Group>
            <Form.Group controlId="host">
              <Form.Label>Host</Form.Label>
              <Form.Control
                type="text"
                name="server.host"
                value={config.server.host}
                onChange={handleChange}
              />
            </Form.Group>
            <Form.Group controlId="maxprocess">
              <Form.Label>Max Process</Form.Label>
              <Form.Control
                type="number"
                name="server.maxprocess"
                value={config.server.maxprocess}
                onChange={handleChange}
              />
            </Form.Group>
            <Form.Group controlId="maxthread">
              <Form.Label>Max Thread</Form.Label>
              <Form.Control
                type="number"
                name="server.maxthread"
                value={config.server.maxthread}
                onChange={handleChange}
              />
            </Form.Group>
            <Form.Group controlId="modulepath">
              <Form.Label>Module Path</Form.Label>
              <Form.Control
                type="text"
                name="server.modulepath"
                value={config.server.modulepath}
                onChange={handleChange}
              />
            </Form.Group>
            <Form.Group controlId="port">
              <Form.Label>Port</Form.Label>
              <Form.Control
                type="number"
                name="server.port"
                value={config.server.port}
                onChange={handleChange}
              />
            </Form.Group>

            {/* Terminal Configuration Section */}
            <Form.Group controlId="enabled">
              <Form.Check
                type="checkbox"
                label="Enabled"
                name="terminal.enabled"
                checked={config.terminal.enabled}
                onChange={handleChange}
              />
            </Form.Group>

            {/* Submit Button */}
            <Button variant="primary" type="submit">
              <Save className="me-2" /> Save Configuration
            </Button>
          </Form>
        </Modal.Body>
      </Modal>
    </>
  );
};

export default ConfigCreationModal;
