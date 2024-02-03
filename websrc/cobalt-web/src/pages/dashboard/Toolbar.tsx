// Toolbar.jsx
import React from "react";
import { Button } from "react-bootstrap";
import { SToolbar } from "./style";
import { ChevronLeft, Gear, Tools } from "react-bootstrap-icons";

const Toolbar = ({ visible, onToggle }) => {
  return (
    <SToolbar visible={visible}>
      <Button variant="secondary" onClick={onToggle}>
        {visible ? "隐藏工具栏" : "显示工具栏"}
      </Button>
      <Button variant="primary">
        <Gear />
        工具1
      </Button>
      <Button variant="primary">
        <Tools />
        工具2
      </Button>
      <Button variant="light" onClick={onToggle} style={{ alignSelf: "flex-start" }}>
        <ChevronLeft size={20} />
      </Button>
      {/* ... Add more toolbar buttons with icons and names */}
    </SToolbar>
  );
};

export default Toolbar;
