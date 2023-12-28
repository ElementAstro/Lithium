import React from 'react';
import { Modal, Button } from 'react-bootstrap';
import { Question, Check } from 'react-bootstrap-icons';

const DoubleWarningModal = ({ show, onHide, onConfirm }) => {
    return (
        <Modal show={show} onHide={onHide} centered>
            <Modal.Header closeButton>
                <Modal.Title>确认操作?</Modal.Title>
            </Modal.Header>
            <Modal.Body>
                <div className="text-center">
                    <Question className="mb-3 text-danger" size={64} />
                    <p className="text-warning">你确定要执行这步操作吗?</p>
                </div>
            </Modal.Body>
            <Modal.Footer>
                <div className="row">
                    <div className="col-sm-6">
                        <Button variant="warning" block onClick={onConfirm}>
                            确认 <Check />
                        </Button>
                    </div>
                    <div className="col-sm-6">
                        <Button variant="outline-warning" block onClick={onHide}>
                            Nope
                        </Button>
                    </div>
                </div>
            </Modal.Footer>
        </Modal>
    );
};

export default DoubleWarningModal;
