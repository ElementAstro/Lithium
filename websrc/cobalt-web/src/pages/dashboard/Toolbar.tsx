// Toolbar.jsx
import React from "react";
import { Button } from "react-bootstrap";
import { SToolbar } from "./style";
import {
  Boxes,
  ChevronLeft,
  ChevronRight,
  Gear,
  GraphDown,
  MapFill,
  Tools,
  XCircle,
} from "react-bootstrap-icons";

const Toolbar = ({ visible, onToggle }) => {
  return (
    <SToolbar visible={visible}>
      <Button variant="light" className="mt-1">
        <MapFill size={20} />
        <span style={{ marginLeft: "10px" }}>Guide</span>
      </Button>
      <Button variant="light">
        <Boxes size={20} className="mt-1"/>
        <span style={{ marginLeft: "10px" }}>Solve</span>
      </Button>
      <Button variant="light" className="mt-1">
        <GraphDown size={20} className="mt-1"/>
        <span style={{ marginLeft: "10px" }}>Hist</span>
      </Button>
      <Button variant="light" className="mt-1">
        <Tools size={20} className="mt-1"/>
        <span style={{ marginLeft: "10px" }}>Tools</span>
      </Button>
      <Button
        variant="light"
        onClick={onToggle}
        style={{ alignSelf: "flex-start" }}
        className="mt-1 mr-1 mb-1"
      >
        {visible ? <ChevronLeft size={20} /> : <ChevronRight size={20} />}
      </Button>
    </SToolbar>
  );
};

export default Toolbar;
