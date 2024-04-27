import React, { useState, useEffect, useRef } from "react";
import { Modal, Button } from "react-bootstrap";
import ObjectSearch from "./object_search";
import { XCircle } from "react-bootstrap-icons";
import { ArrowUpCircle } from "react-bootstrap-icons";

interface ObjectSearchDialogProps {
  open_dialog: number;
}

const ObjectSearchDialog: React.FC<ObjectSearchDialogProps> = (props) => {
  const [open, setOpen] = useState(false);
  const dialogRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (props.open_dialog > 0) {
      setOpen(true);
    }
  }, [props.open_dialog]);

  const handleClose = () => {
    setOpen(false);
  };

  return (
    <>
      <Modal show={open} onHide={() => handleClose()} fullscreen>
        <Modal.Header closeButton>
          <Modal.Title>目标搜索</Modal.Title>
        </Modal.Header>
        <Modal.Body ref={dialogRef}>
          <ObjectSearch on_choice_maken={handleClose} />
        </Modal.Body>
        <Modal.Footer>
          <Button size="sm" variant="danger" onClick={() => setOpen(false)}>
            <XCircle size={20} />
            关闭
          </Button>
        </Modal.Footer>
      </Modal>
      {open && (
        <ArrowUpCircle
          color="primary"
          size={40}
          style={{
            position: "fixed",
            bottom: "20px",
            left: "20px",
            zIndex: 1400,
          }}
          onClick={() => {
            if (dialogRef.current) dialogRef.current.scrollTop = 0;
          }}
        />
      )}
    </>
  );
};

export default ObjectSearchDialog;
