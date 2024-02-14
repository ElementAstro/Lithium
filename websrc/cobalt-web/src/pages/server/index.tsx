import React, { useState, useEffect, useCallback } from "react";
import { Row, Col, Form, Button, Modal } from "react-bootstrap";
import { HddRack, Ethernet, People, Key, XCircle } from "react-bootstrap-icons";
import {
  StyledContainer,
  StyledForm,
  StyledModalBody,
  StyledModalFooter,
  StyledButtonToolbar,
  StyledErrorMessage,
} from "./style";

import { useTranslation } from "react-i18next";

import * as Yup from "yup";
import StarField from "./star";

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
  const [validated, setValidated] = useState(false);
  const { t } = useTranslation();

  useEffect(() => {
    if (showModal) {
      document.body.classList.add("modal-open");
    } else {
      document.body.classList.remove("modal-open");
    }
  }, [showModal]);

  const schema = Yup.object().shape({
    serverUrl: Yup.string()
      .typeError(t("Server URL must be a valid URL"))
      .matches(
        /^(?:http(s)?:\/\/)?[\w.-]+(?:\.[\w\.-]+)+[\w\-\._~:/?#[\]@!\$&'\(\)\*\+,;=.]+$/,
        t("Server URL must be a valid URL")
      )
      .required(),
    serverPort: Yup.number()
      .typeError(t("Port must be a number"))
      .min(1, t("Port must be greater than or equal to 1"))
      .max(65535, t("Port must be less than or equal to 65535"))
      .required(),
    username: Yup.string().matches(
      /^[a-zA-Z0-9_]+$/,
      t("Username can only contain letters, numbers, and underscores")
    ),
  });

  const handleSearch = (event) => {
    event.preventDefault();
    setSearching(true);
    setValidated(false);

    const form = event.currentTarget;
    if (form.checkValidity() === false) {
      event.stopPropagation();
      setValidated(true);
    } else {
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
            setValidated(true);
          } else {
            setErrorMessage(t("服务器搜索失败"));
            setShowModal(true);
          }
          setSearching(false);
        })
        .catch((error) => {
          console.error(t("服务器搜索错误:"), error);
          setErrorMessage(t("服务器搜索错误"));
          setSearching(false);
          setShowModal(true);
        });
    }
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
        setErrorMessage(t("Invalid server"));
        setShowModal(true);
      }

      setConnecting(false);
    }, 2000);
  };

  const handleConfirm = () => {
    setShowModal(false);
  };

  return (
    <>
      <StarField></StarField>
      <StyledContainer fluid>
        <h1>服务器搜索</h1>

        <StyledForm
          noValidate
          validated={validated}
          onSubmit={(e) => {
            e.preventDefault();
            setValidated(true);
          }}
        >
          <Row className="mb-3">
            <Col md="6">
              <Form.Group controlId="validationServerUrl">
                <Form.Label>
                  <HddRack /> Server URL
                </Form.Label>
                <Form.Control
                  required
                  type="text"
                  name="serverUrl"
                  value={serverUrl}
                  onChange={(e) => setServerUrl(e.target.value)}
                  isInvalid={validated && (!!errorMessage || !serverUrl)}
                />
                <Form.Control.Feedback type="invalid">
                  {!!errorMessage ? (
                    <XCircle />
                  ) : (
                    "请提供有效的服务器URL，并确保服务器可用"
                  )}
                </Form.Control.Feedback>
              </Form.Group>
            </Col>
            <Col md="6">
              <Form.Group controlId="validationServerPort">
                <Form.Label>
                  <Ethernet /> Server Port
                </Form.Label>
                <Form.Control
                  required
                  type="number"
                  name="serverPort"
                  value={serverPort}
                  max={65535}
                  min={1}
                  onChange={(e) => setServerPort(e.target.value)}
                  isInvalid={validated && (!!errorMessage || !serverPort)}
                />
                <Form.Control.Feedback type="invalid">
                  {!!errorMessage ? (
                    <XCircle />
                  ) : (
                    "请提供有效的服务器端口，并确保服务器可用"
                  )}
                </Form.Control.Feedback>
              </Form.Group>
            </Col>
          </Row>
          <Row className="mb-3">
            <Col md="6">
              <Form.Group controlId="validationUsername">
                <Form.Label>
                  <People /> Username
                </Form.Label>
                <Form.Control
                  type="text"
                  placeholder="Username"
                  name="username"
                  value={username}
                  onChange={(e) => setUsername(e.target.value)}
                  isValid={validated && !!username}
                />
              </Form.Group>
            </Col>
            <Col md="6">
              <Form.Group controlId="validationSslEnabled">
                <Form.Label>
                  <Key /> SSL Enabled
                </Form.Label>
                <Form.Check
                  type="switch"
                  id="sslSwitch"
                  label=""
                  checked={sslEnabled}
                  onChange={(e) => setSslEnabled(e.target.checked)}
                  className="lg"
                />
              </Form.Group>
            </Col>
          </Row>
          <Form.Group className="mb-3">
            <Form.Check
              required
              name="terms"
              label="同意条款和条件"
              feedback="提交前必须同意"
            />
          </Form.Group>

          <StyledButtonToolbar>
            <Button
              variant="primary"
              onClick={handleSearch}
              disabled={searching || connecting}
              className="mr-2"
            >
              {searching ? "搜索中..." : "搜索"}
            </Button>
            <Button
              variant="success"
              onClick={handleConnect}
              disabled={connecting || !serverPort}
            >
              {connecting ? "连接中..." : "连接"}
            </Button>
          </StyledButtonToolbar>
        </StyledForm>

        {errorMessage && (
          <StyledErrorMessage>
            <XCircle /> {errorMessage}
          </StyledErrorMessage>
        )}

        <Modal show={showModal} onHide={() => setShowModal(false)}>
          <Modal.Header closeButton>
            <Modal.Title>Error</Modal.Title>
          </Modal.Header>
          <StyledModalBody>
            <XCircle /> {errorMessage}
          </StyledModalBody>
          <StyledModalFooter>
            <Button variant="secondary" onClick={() => setShowModal(false)}>
              关闭
            </Button>
            <Button variant="primary" onClick={handleConfirm}>
              确认
            </Button>
          </StyledModalFooter>
        </Modal>
      </StyledContainer>
    </>
  );
};

export default ServerSearch;
