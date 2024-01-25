import React, { useState } from "react";
import {
  Button,
  Card,
  Collapse,
  Form,
  Nav,
  Tab,
  Tabs,
  Modal,
  Row,
  Col,
  Accordion,
} from "react-bootstrap";
import { Prism as SyntaxHighlighter } from "react-syntax-highlighter";
import { vs } from "react-syntax-highlighter/dist/esm/styles/prism";
import {
  CheckCircle,
  XCircle,
  Trash,
  Eye,
  EyeSlash,
  Vr,
  Tree,
} from "react-bootstrap-icons";
import DeleteModal from "./modal";

function ModuleCard(props) {
  const { module, onEnable, onDisable, onDelete } = props;
  const [showJson, setShowJson] = useState(false);
  const [showDeleteModal, setShowDeleteModal] = useState(false);

  const handleToggleJson = () => {
    setShowJson(!showJson);
  };

  const handleToggleDeleteModal = () => {
    setShowDeleteModal(!showDeleteModal);
  };

  const handleConfirmDelete = () => {
    setShowDeleteModal(false);
    onDelete(module.id);
  };

  const DependencyList = (module: { package: { dependencies: any } }) => {
    console.log(module);
    const dependencies = module.package.dependencies;
    let dependencyList = null;

    if (Array.isArray(dependencies)) {
      // The dependencies are already in array format
      dependencyList =
        dependencies.length > 0 ? (
          <>
            <span className="mr-1">Dependencies: </span>
            {dependencies.map((dep) => (
              <span key={dep}>{dep} </span>
            ))}
          </>
        ) : (
          <span>Dependencies: None</span>
        );
    } else if (typeof dependencies === "object" && dependencies !== null) {
      // The dependencies are in object format, so we need to extract the keys
      const dependencyKeys = Object.keys(dependencies);
      dependencyList =
        dependencyKeys.length > 0 ? (
          <>
            <span className="mr-1">Dependencies: </span>
            {dependencyKeys.map((dep) => (
              <span key={dep}>
                {dep} ({dependencies[dep]}){" "}
              </span>
            ))}
          </>
        ) : (
          <span>Dependencies: None</span>
        );
    } else {
      // The dependencies are in an unknown format, so we assume there are none
      dependencyList = <span>Dependencies: None</span>;
    }

    return <div>{dependencyList}</div>;
  };

  return (
    <Card className="mb-3">
      <Card.Header>
        <Card.Title>
          <Row>
            <Col>{module.name}</Col>
            <Col justify="end">
              <Button
                variant={module.enabled ? "secondary" : "primary"}
                onClick={() =>
                  module.enabled ? onDisable(module.id) : onEnable(module.id)
                }
              >
                {module.enabled ? (
                  <>
                    <XCircle size={20} className="mr-1" />
                    Disable
                  </>
                ) : (
                  <>
                    <CheckCircle size={20} className="mr-1" />
                    Enable
                  </>
                )}
              </Button>
              <Button variant="danger" onClick={handleToggleDeleteModal}>
                <Trash size={20} />
              </Button>
            </Col>

            <div className="ml-auto justify-content-end"></div>
          </Row>
        </Card.Title>
      </Card.Header>

      <Card.Body>
        <Row md={12}>
          <Col lg={6}>
            <Card.Subtitle className="mb-2 text-muted">
              {module.author}
            </Card.Subtitle>
            <Card.Subtitle className="mb-2 text-muted">
              {module.description}
            </Card.Subtitle>
            <Card.Subtitle>
              {module.description ? module.description : "No description"}
            </Card.Subtitle>
            <Card.Text>
              <Vr size={20} className="mr-1" />
              Version: {module.version} | Downloads: {module.downloads}
            </Card.Text>
            <Card.Text>
              <Tree size={20} className="mr-1" />
              {/*<DependencyList module={module} />*/}
            </Card.Text>
            <Card.Text></Card.Text>
          </Col>
          <Col lg={6}>
            <div className="mt-3">
              <Nav variant="tabs">
                <Nav.Item>
                  <Nav.Link onClick={handleToggleJson}>
                    {showJson ? (
                      <>
                        <EyeSlash size={18} className="mr-1" /> Hide JSON Files
                      </>
                    ) : (
                      <>
                        <Eye size={18} className="mr-1" /> Show JSON Files
                      </>
                    )}
                  </Nav.Link>
                </Nav.Item>
              </Nav>
              <Collapse in={showJson}>
                <div className="mt-2">
                  <Tabs defaultActiveKey="packageJson" id="json-tabs">
                    <Tab eventKey="packageJson" title="Package JSON">
                      <SyntaxHighlighter language="json" style={vs}>
                        {JSON.stringify(module.packageJson, null, 2)}
                      </SyntaxHighlighter>
                    </Tab>
                    <Tab eventKey="configJson" title="Config JSON">
                      <SyntaxHighlighter language="json" style={vs}>
                        {JSON.stringify(module.configJson, null, 2)}
                      </SyntaxHighlighter>
                    </Tab>
                  </Tabs>
                </div>
              </Collapse>
            </div>
          </Col>
        </Row>
      </Card.Body>
      <DeleteModal
        show={showDeleteModal}
        onHide={handleToggleDeleteModal}
        onConfirm={handleConfirmDelete}
      />
    </Card>
  );
}

export default ModuleCard;
