import React, { useState } from "react";
import { Modal, Button, Form, ListGroup } from "react-bootstrap";
import { XCircle } from "react-bootstrap-icons";

const BrandDriverSelectDialog = (props) => {
  const [filter, setFilter] = useState("");

  const handleInputChange = (event) => {
    setFilter(event.target.value);
  };

  const handleListItemClick = (option) => {
    props.handleSelect(option, props.device_type_index);
    props.handleClose();
  };

  return (
    <Modal show={props.open} onHide={props.handleClose} size="lg" centered>
      <Modal.Header closeButton>
        <Modal.Title>搜索驱动</Modal.Title>
      </Modal.Header>
      <Modal.Body>
        <Form.Control
          type="text"
          placeholder="搜索驱动"
          value={filter}
          onChange={handleInputChange}
        />
        <ListGroup>
          {props.options
            .filter((option) => {
              const { zh, en, driver } = option;
              const lowerCaseFilter = filter.toLowerCase();
              return (
                zh.toLowerCase().includes(lowerCaseFilter) ||
                en.toLowerCase().includes(lowerCaseFilter) ||
                driver.toLowerCase().includes(lowerCaseFilter)
              );
            })
            .map((option) => (
              <ListGroup.Item
                key={option.en}
                action
                onClick={() => handleListItemClick(option)}
              >
                {option.zh}
              </ListGroup.Item>
            ))}
        </ListGroup>
      </Modal.Body>
      <Modal.Footer>
        <Button variant="secondary" onClick={props.handleClose}>
          <XCircle size={20} />
          取消
        </Button>
      </Modal.Footer>
    </Modal>
  );
};

export default BrandDriverSelectDialog;
