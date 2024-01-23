import React from "react";
import { Modal, Button } from "react-bootstrap";

const DeviceAdvancedControlModal = (props: { handleCloseModal: () => void; handleOkModal: () => void; showModal: boolean | undefined; }) => {
  const handleClose = () => {
    props.handleCloseModal();
  };

  const handleOk = () => {
    props.handleOkModal();
  };

  return (
    <Modal show={props.showModal} onHide={handleClose}>
      <Modal.Header closeButton>
        <Modal.Title>警告</Modal.Title>
      </Modal.Header>
      <Modal.Body>
        <p>
          详细配置是专业模式，除非必要不建议用户修改！修改会导致不可预料的故障！是否确认进入？
        </p>
      </Modal.Body>
      <Modal.Footer>
        <Button variant="secondary" onClick={handleClose}>
          取消
        </Button>
        <Button variant="warning" onClick={handleOk}>
          确认
        </Button>
      </Modal.Footer>
    </Modal>
  );
};

export default DeviceAdvancedControlModal;
