import React, { useState, useEffect } from "react";
import {
  Modal,
  Form,
  Button,
  Col,
  Row,
  InputGroup,
  ButtonToolbar,
} from "react-bootstrap";
import {
  HddRack,
  Ethernet,
  People,
  Key,
  CheckCircle,
  XCircle,
} from "react-bootstrap-icons";

import { useTranslation } from "react-i18next";

import * as Yup from "yup";

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
    <div className="container">
      <h1 className="title">{t("服务器搜索")}</h1>

      <Form
        noValidate
        validated={validated}
        onSubmit={(e) => {
          e.preventDefault();
          setValidated(true);
        }}
      >
        <Row className="mb-3">
          <Form.Group as={Col} md="4" controlId="validationServerUrl">
            <Form.Label>
              <HddRack /> {t("Server URL")}
            </Form.Label>
            <Form.Control
              required
              type="text"
              name="serverUrl"
              value={serverUrl}
              onChange={(e) => setServerUrl(e.target.value)}
              // isValid={validated && !errorMessage && serverPort}
              isInvalid={validated && (!!errorMessage || !serverUrl)}
            />
            <Form.Control.Feedback type="invalid">
              {!!errorMessage ? (
                <XCircle />
              ) : (
                t(
                  "Please provide a valid server URL and ensure the server is available"
                )
              )}
            </Form.Control.Feedback>
          </Form.Group>
          <Form.Group as={Col} md="2" controlId="validationServerPort">
            <Form.Label>
              <Ethernet /> {t("Server Port")}
            </Form.Label>
            <Form.Control
              required
              type="number"
              name="serverPort"
              value={serverPort}
              max={65535}
              min={1}
              onChange={(e) => setServerPort(e.target.value)}
              //isValid={validated && !errorMessage && serverPort}
              isInvalid={validated && (!!errorMessage || !serverPort)}
            />
            <Form.Control.Feedback type="invalid">
              {!!errorMessage ? (
                <XCircle />
              ) : (
                t(
                  "Please provide a valid server port and ensure the server is available"
                )
              )}
            </Form.Control.Feedback>
          </Form.Group>
        </Row>
        <Row className="mb-3">
          <Form.Group as={Col} md="4" controlId="validationUsername">
            <Form.Label>
              <People /> {t("Username")}
            </Form.Label>
            <Form.Control
              type="text"
              placeholder="Username"
              name="username"
              value={username}
              onChange={(e) => setUsername(e.target.value)}
              isValid={validated && !!username}
              //isInvalid={validated && !!errors.username}
            />
            <Form.Control.Feedback type="invalid">
              {errorMessage}
            </Form.Control.Feedback>
          </Form.Group>
          <Form.Group as={Col} md="2" controlId="validationSslEnabled">
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
        </Row>

        <Form.Group className="mb-3">
          <Form.Check
            required
            name="terms"
            label="Agree to terms and conditions"
            feedback="You must agree before submitting."
          />
        </Form.Group>

        <ButtonToolbar className="mt-3">
          <Button
            variant="primary"
            onClick={handleSearch}
            disabled={searching || connecting}
            className="mr-2"
          >
            {searching ? t("Searching...") : t("Search")}
          </Button>
          <Button
            variant="success"
            onClick={handleConnect}
            disabled={connecting || !serverPort}
          >
            {connecting ? t("Connecting...") : t("Connect")}
          </Button>
        </ButtonToolbar>
      </Form>

      {errorMessage && (
        <div className="mt-3 text-danger">
          <XCircle /> {errorMessage}
        </div>
      )}

      <Modal show={showModal} onHide={() => setShowModal(false)}>
        <Modal.Header closeButton>
          <Modal.Title>Error</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <XCircle /> {errorMessage}
        </Modal.Body>
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
