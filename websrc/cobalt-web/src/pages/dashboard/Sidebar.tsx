// Sidebar.jsx
import React from "react";
import { Form, InputGroup, FormControl } from "react-bootstrap";
import {SSidebar} from "./style"

const Sidebar = ({ visible, brightness, contrast, saturation, onBrightnessChange, onContrastChange, onSaturationChange }) => {
  return (
    <SSidebar visible={visible}>
      <Form>
        <Form.Group controlId="brightness">
          <Form.Label>亮度</Form.Label>
          <InputGroup>
            <FormControl
              type="range"
              min="0"
              max="100"
              value={brightness}
              onChange={onBrightnessChange}
            />
            <FormControl value={brightness} readOnly />
          </InputGroup>
        </Form.Group>
        <Form.Group controlId="contrast">
          <Form.Label>对比度</Form.Label>
          <InputGroup>
            <FormControl
              type="range"
              min="0"
              max="100"
              value={contrast}
              onChange={onContrastChange}
            />
            <FormControl value={contrast} readOnly />
          </InputGroup>
        </Form.Group>
        <Form.Group controlId="saturation">
          <Form.Label>饱和度</Form.Label>
          <InputGroup>
            <FormControl
              type="range"
              min="0"
              max="100"
              value={saturation}
              onChange={onSaturationChange}
            />
            <FormControl value={saturation} readOnly />
          </InputGroup>
        </Form.Group>
      </Form>
    </SSidebar>
  );
};

export default Sidebar;
