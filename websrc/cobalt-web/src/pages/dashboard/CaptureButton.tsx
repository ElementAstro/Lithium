// CaptureButton.jsx
import React from "react";
import { Button } from "react-bootstrap";

const CaptureButton = ({ onCapture }) => {
  return (
    <Button variant="primary" onClick={onCapture}>
      拍摄
    </Button>
  );
};

export default CaptureButton;
