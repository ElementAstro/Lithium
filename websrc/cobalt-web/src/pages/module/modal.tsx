import React, { useState } from "react";
import {
  Button,
  Card,
  Collapse,
  Form,
  Nav,
  Tab,
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
} from "react-bootstrap-icons";
import FAQAccordion from "./faq";

function DeleteModal(props) {
  const { show, onHide, onConfirm } = props;
  const [showFAQ, setShowFAQ] = useState(false);

  const handleToggleFAQ = () => {
    setShowFAQ(!showFAQ);
  };

  return (
    <Modal show={show} onHide={onHide}>
      <Modal.Header closeButton>
        <Modal.Title>Confirm Deletion</Modal.Title>
      </Modal.Header>
      <Modal.Body>
        Are you sure you want to delete this module?
        <Accordion className="mt-3" activeKey={showFAQ ? "0" : ""}>
          <Card>
            <Accordion as={Card.Header} eventKey="0">
              Frequently Asked Questions
            </Accordion>
            <Accordion.Collapse eventKey="0">
              <Card.Body>
                <FAQAccordion />
              </Card.Body>
            </Accordion.Collapse>
          </Card>
        </Accordion>
      </Modal.Body>
      <Modal.Footer>
        <Button variant="secondary" onClick={onHide}>
          Cancel
        </Button>
        <Button variant="info" onClick={handleToggleFAQ}>
          FAQ
        </Button>
        <Button variant="danger" onClick={onConfirm}>
          Delete
        </Button>
      </Modal.Footer>
    </Modal>
  );
}

export default DeleteModal;
