// CaptureButton.jsx
import React from "react";
import { Button } from "react-bootstrap";
import styled from "styled-components";

export const CaptureButton = styled(Button)`
  width: 60px;
  height: 60px;
  margin: 10px auto; // 水平居中
  border-radius: 50%;
  border: 2px solid white;
  background-color: transparent;
  color: white;
  font-size: 16px;
  z-index: 21;

  &:hover {
    background-color: #555; // 鼠标悬停时变色
  }

  &:active {
    background-color: #ccc;
  }

  animation: pulse 2s infinite;

  @keyframes pulse {
    0% {
      transform: scale(1);
    }
    50% {
      transform: scale(1.1);
    }
    100% {
      transform: scale(1);
    }
  }
`;

export const SCaptureButton = ({ onCapture, Icon }) => {
  return (
    <CaptureButton variant="primary" onClick={onCapture}>
      {Icon}
    </CaptureButton>
  );
};
