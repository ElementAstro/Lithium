import React, { useState, useEffect } from "react";
import { Modal } from "react-bootstrap";
import Button from "react-bootstrap/Button";
import Col from "react-bootstrap/Col";
import Form from "react-bootstrap/Form";
import InputGroup from "react-bootstrap/InputGroup";
import Row from "react-bootstrap/Row";
import "bootstrap/dist/css/bootstrap.min.css";
import { HddRack, Ethernet, People, Key } from "react-bootstrap-icons";
import * as formik from "formik";
import * as yup from "yup";

const ServerSearch = () => {
  const [serverUrl, setServerUrl] = useState("");
  const [serverPort, setServerPort] = useState("");
  const [sslEnabled, setSslEnabled] = useState(false);
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const [searching, setSearching] = useState(false);
  const [connecting, setConnecting] = useState(false);
  const [errorMessage, setErrorMessage] = useState("");
  const [showModal, setShowModal] = useState(false);

  const { Formik } = formik;

  const schema = yup.object().shape({
    serverPort: yup.string().required(),
    serverUrl: yup.string().required(),
    username: yup.string().required(),
    password: yup.string().optional(),
    sslEnabled: yup.bool().optional(),
  });

  useEffect(() => {
    if (showModal) {
      document.body.classList.add("modal-open");
    } else {
      document.body.classList.remove("modal-open");
    }
  }, [showModal]);

  const handleSearch = () => {
    setSearching(true);
    fetch("/api/search-server", {
      method: "POST",
      body: JSON.stringify({ serverUrl }),
      headers: {
        "Content-Type": "application/json",
      },
    })
      .then((response) => response.json())
      .then((data) => {
        if (data.success) {
          setServerPort(data.serverPort);
        } else {
          setErrorMessage("服务器搜索失败");
          setShowModal(true);
        }
        setSearching(false);
      })
      .catch((error) => {
        console.error("服务器搜索错误:", error);
        setErrorMessage("服务器搜索错误");
        setSearching(false);
        setShowModal(true);
      });
  };

  const handleConnect = () => {
    setConnecting(true);

    setTimeout(() => {
      const isValidServer = true;

      if (isValidServer) {
        if (sslEnabled) {
          window.location.href = `https://${serverUrl}:${serverPort}/client`;
        } else {
          window.location.href = `http://${serverUrl}:${serverPort}/client`;
        }
      } else {
        setErrorMessage("Invalid server");
        setShowModal(true);
      }

      setConnecting(false);
    }, 2000);
  };

  const handleConfirm = () => {
    setShowModal(false);
  };

  return (
    <div className="container">
      <h1 className="title">服务器搜索</h1>

      <Formik
        validationSchema={schema}
        onSubmit={console.log}
        initialValues={{
          serverUrl: "",
          serverPort: "",
          sslEnabled: false,
          username: "",
          password: "",
        }}
      >
        {({ handleSubmit, handleChange, values, touched, errors }) => (
          <Form noValidate onSubmit={handleSubmit}>
            <Row className="mb-3">
              <Form.Group as={Col} md="4" controlId="validationFormik01">
                <Form.Label>Server URL</Form.Label>
                <Form.Control
                  type="text"
                  name="serverUrl"
                  value={values.serverUrl}
                  onChange={handleChange}
                  isValid={touched.serverUrl && !errors.serverUrl}
                />
              </Form.Group>
              <Form.Group as={Col} md="4" controlId="validationFormik02">
                <Form.Label>Server Port</Form.Label>
                <Form.Control
                  type="number"
                  name="serverPort"
                  value={values.serverPort}
                  max={65535}
                  min={1}
                  onChange={handleChange}
                  isValid={touched.serverPort && !errors.serverPort}
                />
              </Form.Group>
            </Row>
            <Row className="mb-3">
              
            </Row>
            <Row className="mb-3">
              <Form.Group as={Col} md="4" controlId="validationFormikUsername">
                <Form.Label>Username</Form.Label>
                <InputGroup hasValidation>
                  <InputGroup.Text id="inputGroupPrepend">@</InputGroup.Text>
                  <Form.Control
                    type="text"
                    placeholder="Username"
                    aria-describedby="inputGroupPrepend"
                    name="username"
                    value={values.username}
                    onChange={handleChange}
                    isInvalid={!!errors.username}
                  />
                  <Form.Control.Feedback type="invalid">
                    {errors.username}
                  </Form.Control.Feedback>
                </InputGroup>
              </Form.Group>
            </Row>

            <Form.Group className="mb-3">
              <Form.Check
                required
                name="sslEnabled"
                label="SSL Enabled"
                onChange={handleChange}
                isInvalid={!!errors.sslEnabled}
                feedback={errors.sslEnabled}
                feedbackType="invalid"
                id="validationFormik0"
              />
            </Form.Group>
            <InputGroup className="mt-3 col-md-12">
              <Button
                variant="primary"
                onClick={handleSearch}
                disabled={searching || connecting}
                className="flex-fill"
              >
                {searching ? "Searching..." : "Search"}
              </Button>
              <Button
                variant="success"
                onClick={handleConnect}
                disabled={connecting || !serverPort}
                className="flex-fill"
              >
                {connecting ? "Connecting..." : "Connect"}
              </Button>
            </InputGroup>
          </Form>
        )}
      </Formik>

      {errorMessage && <div className="mt-3 text-danger">{errorMessage}</div>}

      <Modal show={showModal} onHide={() => setShowModal(false)}>
        <Modal.Header closeButton>
          <Modal.Title>Error</Modal.Title>
        </Modal.Header>
        <Modal.Body>{errorMessage}</Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={() => setShowModal(false)}>
            Close
          </Button>
          <Button variant="primary" onClick={handleConfirm}>
            Confirm
          </Button>
        </Modal.Footer>
      </Modal>
    </div>
  );
};

export default ServerSearch;
