import React, { useState, useEffect } from "react";
import { Modal, Button } from "react-bootstrap";
import ObjectManagement from "./object_manager";
import { GlobalStore } from "../../store/globalStore";
import { XCircle } from "react-bootstrap-icons";

const ObjectManagementDialog = (props) => {
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
        <Modal.Body>
          <ObjectManagement on_choice_maken={handleClose} />
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={() => handleClose()}>
            <XCircle /> 关闭
          </Button>
        </Modal.Footer>
      </Modal>
    </>
  );
};

export default ObjectManagementDialog;
