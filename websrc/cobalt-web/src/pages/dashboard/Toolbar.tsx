// Toolbar.jsx
import React from "react";
import { Button } from "react-bootstrap";
import { SToolbar } from "./style";

const Toolbar = ({ visible, onToggle }) => {
  return (
    <SToolbar visible={visible}>
        <Button variant="secondary" onClick={onToggle}>
        {visible ? "隐藏工具栏" : "显示工具栏"}
      </Button>
      {/* Add more toolbar buttons */}
      <Button variant="primary">工具1</Button>
      <Button variant="primary">工具2</Button>
      {/* ... */}
    </SToolbar>
  );
};

export default Toolbar;
