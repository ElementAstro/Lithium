import React from "react";
import { Modal, Button } from "react-bootstrap";
import { ExclamationTriangle, Envelope } from "react-bootstrap-icons";
import { useTranslation } from "react-i18next";

const InfoModal = ({ show, onHide, onConfirm, message }) => {
  const { t } = useTranslation();
  return (
    <Modal show={show} onHide={onHide} centered>
      <Modal.Header closeButton>
        <Modal.Title>信息</Modal.Title>
      </Modal.Header>
      <Modal.Body>
        <div className="text-center">
          <div className="spinner-border text-primary m-5" role="status">
            <span className="sr-only">常规信息</span>
          </div>
          <p className="note note-info">{message}</p>
        </div>
      </Modal.Body>
      <Modal.Footer>
        <Button variant="primary" className="modal-voy-btn" onClick={onHide}>
          Ok
        </Button>
      </Modal.Footer>
    </Modal>
  );
};

export default InfoModal;
