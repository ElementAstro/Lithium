import React, { useState, useEffect } from "react";
import axios from "axios";
import {
  Container,
  Row,
  Col,
  Card,
  Button,
  Modal,
  Form,
  InputGroup,
  FormControl,
} from "react-bootstrap";
import AceEditor from "react-ace-builds";
import "ace-builds/src-noconflict/mode-sh";
import "ace-builds/src-noconflict/theme-github";

const BASE_URL = "http://localhost:3000"; // Replace with your backend URL

function ScriptManager() {
  const [scripts, setScripts] = useState([]);
  const [showAddModal, setShowAddModal] = useState(false);
  const [showEditModal, setShowEditModal] = useState(false);
  const [selectedScript, setSelectedScript] = useState(null);
  const [searchTerm, setSearchTerm] = useState("");
  const [sortField, setSortField] = useState("name");
  const [sortOrder, setSortOrder] = useState("asc");
  const [newScriptName, setNewScriptName] = useState("");
  const [newScriptDescription, setNewScriptDescription] = useState("");
  const [newScriptCommand, setNewScriptCommand] = useState("");

  useEffect(() => {
    axios.get(`${BASE_URL}/scripts`).then((response) => {
      setScripts(response.data);
    });
  }, []);

  const handleAddScript = () => {
    const newScript = {
      name: newScriptName,
      description: newScriptDescription,
      command: newScriptCommand,
    };

    axios.post(`${BASE_URL}/scripts`, newScript).then((response) => {
      setScripts([...scripts, response.data]);
      setShowAddModal(false);
    });
  };

  const handleEditScript = () => {
    const updatedScript = {
      ...selectedScript,
      name: newScriptName,
      description: newScriptDescription,
      command: newScriptCommand,
    };

    axios
      .put(`${BASE_URL}/scripts/${selectedScript.id}`, updatedScript)
      .then(() => {
        setScripts(
          scripts.map((script) =>
            script.id === selectedScript.id ? updatedScript : script
          )
        );
        setShowEditModal(false);
      });
  };

  const handleDeleteScript = (scriptId) => {
    if (window.confirm("Are you sure you want to delete this script?")) {
      axios.delete(`${BASE_URL}/scripts/${scriptId}`).then(() => {
        setScripts(scripts.filter((script) => script.id !== scriptId));
      });
    }
  };

  const handleScriptClick = (script) => {
    // Handle script click
    console.log(`Clicked on script with ID ${script.id}`);
  };

  const handleAddModalClose = () => {
    setShowAddModal(false);
    setNewScriptName("");
    setNewScriptDescription("");
    setNewScriptCommand("");
  };

  const handleEditModalClose = () => {
    setShowEditModal(false);
    setSelectedScript(null);
    setNewScriptName("");
    setNewScriptDescription("");
    setNewScriptCommand("");
  };

  const handleSortChange = (e) => {
    const field = e.target.value;
    const order = sortOrder === "asc" ? "desc" : "asc";
    setSortField(field);
    setSortOrder(order);
  };

  const filteredScripts = scripts.filter(
    (script) =>
      script.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
      script.description.toLowerCase().includes(searchTerm.toLowerCase())
  );

  const sortedScripts = filteredScripts.sort((a, b) => {
    const fieldA = a[sortField].toLowerCase();
    const fieldB = b[sortField].toLowerCase();

    if (fieldA < fieldB) {
      return sortOrder === "asc" ? -1 : 1;
    } else if (fieldA > fieldB) {
      return sortOrder === "asc" ? 1 : -1;
    } else {
      return 0;
    }
  });

  return (
    <Container fluid>
      <h1>Script Manager</h1>
      <Row>
        <Col xs={12} md={4}>
          <InputGroup className="mb-3">
            <FormControl
              placeholder="Search scripts..."
              value={searchTerm}
              onChange={(e) => setSearchTerm(e.target.value)}
            />
          </InputGroup>
        </Col>
        <Col xs={12} md={8} className="text-right">
          <Form.Group>
            <Form.Label>Sort by:</Form.Label>
            <Form.Control as="select" onChange={handleSortChange}>
              <option value="name">Name</option>
              <option value="description">Description</option>
            </Form.Control>
          </Form.Group>
        </Col>
      </Row>
      <Row>
        {sortedScripts.map((script) => (
          <Col key={script.id} sm={6} md={4} lg={3}>
            <Card>
              <Card.Body>
                <Card.Title>{script.name}</Card.Title>
                <Card.Text>{script.description}</Card.Text>
                <Button
                  variant="primary"
                  onClick={() => handleScriptClick(script)}
                >
                  Run Script
                </Button>
                <Button
                  variant="secondary"
                  onClick={() => {
                    setSelectedScript(script);
                    setNewScriptName(script.name);
                    setNewScriptDescription(script.description);
                    setNewScriptCommand(script.command);
                    setShowEditModal(true);
                  }}
                >
                  Edit
                </Button>
                <Button
                  variant="danger"
                  onClick={() => handleDeleteScript(script.id)}
                >
                  Delete
                </Button>
              </Card.Body>
            </Card>
          </Col>
        ))}
        <Col sm={6} md={4} lg={3}>
          <Card>
            <Card.Body>
              <Button
                variant="outline-success"
                block
                onClick={() => setShowAddModal(true)}
              >
                Add Script
              </Button>
            </Card.Body>
          </Card>
        </Col>
      </Row>
      <Modal show={showAddModal} onHide={handleAddModalClose}>
        <Modal.Header closeButton>
          <Modal.Title>Add New Script</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form onSubmit={(e) => e.preventDefault()}>
            <Form.Group controlId="formScriptName">
              <Form.Label>Name:</Form.Label>
              <Form.Control
                type="text"
                placeholder="Enter script name"
                value={newScriptName}
                onChange={(e) => setNewScriptName(e.target.value)}
                required
              />
            </Form.Group>
            <Form.Group controlId="formScriptDescription">
              <Form.Label>Description:</Form.Label>
              <Form.Control
                type="text"
                placeholder="Enter script description"
                value={newScriptDescription}
                onChange={(e) => setNewScriptDescription(e.target.value)}
                required
              />
            </Form.Group>
            <Form.Group controlId="formScriptCommand">
              <Form.Label>Command:</Form.Label>
              <AceEditor
                mode="sh"
                theme="github"
                value={newScriptCommand}
                onChange={(value) => setNewScriptCommand(value)}
                name="newScriptCommand"
                editorProps={{ $blockScrolling: true }}
                width="100%"
                height="200px"
              />
            </Form.Group>
          </Form>
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={handleAddModalClose}>
            Cancel
          </Button>
          <Button
            variant="primary"
            onClick={handleAddScript}
            disabled={!newScriptName}
          >
            Save
          </Button>
        </Modal.Footer>
      </Modal>
      <Modal show={showEditModal} onHide={handleEditModalClose}>
        <Modal.Header closeButton>
          <Modal.Title>Edit Script</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form onSubmit={(e) => e.preventDefault()}>
            <Form.Group controlId="formScriptName">
              <Form.Label>Name:</Form.Label>
              <Form.Control
                type="text"
                placeholder="Enter script name"
                value={newScriptName}
                onChange={(e) => setNewScriptName(e.target.value)}
                required
              />
            </Form.Group>
            <Form.Group controlId="formScriptDescription">
              <Form.Label>Description:</Form.Label>
              <Form.Control
                type="text"
                placeholder="Enter script description"
                value={newScriptDescription}
                onChange={(e) => setNewScriptDescription(e.target.value)}
                required
              />
            </Form.Group>
            <Form.Group controlId="formScriptCommand">
              <Form.Label>Command:</Form.Label>
              <AceEditor
                mode="sh"
                theme="github"
                value={newScriptCommand}
                onChange={(value) => setNewScriptCommand(value)}
                name="newScriptCommand"
                editorProps={{ $blockScrolling: true }}
                width="100%"
                height="200px"
              />
            </Form.Group>
          </Form>
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={handleEditModalClose}>
            Cancel
          </Button>
          <Button
            variant="primary"
            onClick={handleEditScript}
            disabled={!newScriptName}
          >
            Save Changes
          </Button>
        </Modal.Footer>
      </Modal>
    </Container>
  );
}

export default ScriptManager;
