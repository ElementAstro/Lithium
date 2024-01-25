import React from "react";
import { Modal, Button } from "react-bootstrap";
import ObjectSearch from "./object_search";
import { XCircle } from "react-bootstrap-icons";

interface ObjectSearchDialogProps {
  open_dialog: number;
}

const ObjectSearchDialog: React.FC<ObjectSearchDialogProps> = (props) => {
  const [open, set_open] = React.useState(false);

  React.useEffect(() => {
    if (props.open_dialog > 0) {
      set_open(true);
    }
  }, [props.open_dialog]);

  const handleClose = () => {
    set_open(false);
  };

  return (
    <>
      <Modal show={open} onHide={handleClose} fullscreen>
        <Modal.Header closeButton>
          <Modal.Title>标题</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <ObjectSearch on_choice_maken={handleClose} />
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={handleClose}>
            <XCircle />
            关闭
          </Button>
        </Modal.Footer>
      </Modal>
    </>
  );
};

export default ObjectSearchDialog;
