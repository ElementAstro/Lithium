import React, { useState, useEffect } from "react";
import { Modal, Button } from "react-bootstrap";
import ObjectManagement from "./object_manager";
import { XLg } from "react-bootstrap-icons";
import { GlobalStore } from "../../store/globalStore";

interface ObjectManagementDialogProps {
  open_dialog: number;
}

const ObjectManagementDialog: React.FC<ObjectManagementDialogProps> = (
  props
) => {
  const [open, set_open] = useState(false);
  const clear_all_checked =
    GlobalStore.actions.TargetListStore.clear_all_checked;

  useEffect(() => {
    if (props.open_dialog > 0) {
      set_open(true);
    }
  }, [props.open_dialog]);

  const handleClose = () => {
    clear_all_checked();
    set_open(false);
  };

  return (
    <>
      <Modal show={open} onHide={() => handleClose()} size="xl" fullscreen>
        <Modal.Header closeButton>
          <Modal.Title>待拍摄目标管理</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <ObjectManagement on_choice_maken={handleClose} />
        </Modal.Body>
        <Modal.Footer>
          <Button variant="danger" onClick={() => set_open(false)}>
            <XLg className="me-2" />
            关闭
          </Button>
        </Modal.Footer>
      </Modal>
    </>
  );
};

export default ObjectManagementDialog;
