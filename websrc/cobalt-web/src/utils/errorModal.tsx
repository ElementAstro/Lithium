import React from 'react';
import { Modal, Button } from 'react-bootstrap';
import { ExclamationTriangle, Envelope } from 'react-bootstrap-icons';

const ErrorModal = ({ show, onHide, errorMessage }) => {
    return (
        <Modal show={show} onHide={onHide} centered>
            <Modal.Header closeButton>
                <Modal.Title>错误信息</Modal.Title>
            </Modal.Header>
            <Modal.Body>
                <div className="text-center">
                    <ExclamationTriangle className="fa-4x mb-3 animated bounceIn" />
                    <p className="note note-danger">
                        {errorMessage}
                    </p>
                    <p className="text-muted">
                        <small>如果你有任何问题，请将详细细节告知开发者</small>
                    </p>
                </div>
            </Modal.Body>
            <Modal.Footer>
                <a
                    className="btn btn-outline-danger modal-voy-btn"
                    href="mailto:astro_air@126.com,fran.dibiase@gmail.com"
                >
                    <Envelope className="mr-1" /> 联系我们
                </a>
                <Button variant="danger" className="modal-voy-btn" onClick={onHide}>
                    Ok
                </Button>
            </Modal.Footer>
        </Modal>
    );
};

export default ErrorModal;
