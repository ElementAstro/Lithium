import * as React from "react";
import Button from "react-bootstrap/Button";
import Modal from "react-bootstrap/Modal";

interface ConfirmDialogProps {
  open: number;
  show_text: string;
  show_title: string;
  on_confirm_clicked: () => void;
  on_cancel_clicked: () => void;
}

const ConfirmDialog: React.FC<ConfirmDialogProps> = (props) => {
  const [show, setShow] = React.useState(false);

  const handleClose = (confirm_flag: boolean) => {
    setShow(false);
    if (confirm_flag) {
      props.on_confirm_clicked();
    } else {
      props.on_cancel_clicked();
    }
  };

  React.useEffect(() => {
    if (props.open > 0) {
      setShow(true);
    }
  }, [props.open]);

  return (
    <Modal show={show} onHide={() => handleClose(false)}>
      <Modal.Header closeButton>
        <Modal.Title>{props.show_title}</Modal.Title>
      </Modal.Header>
      <Modal.Body>{props.show_text}</Modal.Body>
      <Modal.Footer>
        <Button variant="primary" onClick={() => handleClose(true)}>
          Confirm
        </Button>
        <Button variant="secondary" onClick={() => handleClose(false)}>
          Cancel
        </Button>
      </Modal.Footer>
    </Modal>
  );
};

export default ConfirmDialog;
