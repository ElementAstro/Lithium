import styled from "styled-components";
import Container from "react-bootstrap/Container";
import Button from "react-bootstrap/Button";

export const CameraContainer = styled(Container)`
  height: 100vh;
  justify-content: center;
  display: flex;
  align-items: center;
  position: relative;
  background-color: black;
`;

export const CaptureButton = styled(Button)`
  width: 60px;
  height: 60px;
  border-radius: 50%;
  border: 2px solid white;
  background-color: transparent;
  position: absolute;
  bottom: 20px;
`;
