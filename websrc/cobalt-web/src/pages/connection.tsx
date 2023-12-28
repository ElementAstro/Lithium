import React, { useState, useEffect } from "react";
import { Button, Form, InputGroup, Modal } from "react-bootstrap";
import { HddRack, Ethernet, People, Key } from "react-bootstrap-icons";

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

      <Form>
        <Form.Group controlId="serverUrl">
          <HddRack size={20} className="mr-1" />
          <Form.Label>Server Address</Form.Label>
          <Form.Control
            type="url"
            placeholder="Enter server address"
            value={serverUrl}
            onChange={(e) => setServerUrl(e.target.value)}
          />
        </Form.Group>

        <Form.Group controlId="serverPort">
          <Ethernet size={20} className="mr-1" />
          <Form.Label>Server Port</Form.Label>
          <Form.Control
            type="number"
            placeholder="Enter server port"
            value={serverPort}
            onChange={(e) => setServerPort(e.target.value)}
            disabled={searching}
          />
        </Form.Group>

        <Form.Group controlId="sslEnabled">
          <Form.Check
            type="checkbox"
            label="Enable SSL"
            checked={sslEnabled}
            onChange={(e) => setSslEnabled(e.target.checked)}
            disabled={searching}
          />
        </Form.Group>

        <Form.Group controlId="username">
          <People size={20} className="mr-1" />
          <Form.Label>Username</Form.Label>
          <Form.Control
            type="text"
            placeholder="Enter username"
            value={username}
            onChange={(e) => setUsername(e.target.value)}
            disabled={searching}
          />
        </Form.Group>

        <Form.Group controlId="password">
          <Key size={20} className="mr-1" />
          <Form.Label>Password</Form.Label>
          <Form.Control
            type="password"
            placeholder="Enter password"
            value={password}
            onChange={(e) => setPassword(e.target.value)}
            disabled={searching}
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
