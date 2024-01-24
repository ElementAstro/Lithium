import styled, { keyframes } from "styled-components";

export const fadeIn = keyframes`
  from {
    opacity: 0;
  }
  to {
    opacity: 1;
  }
`;

export const fadeOut = keyframes`
  from {
    opacity: 1;
  }
  to {
    opacity: 0;
  }
`;

export const OpeningAnimation = styled.div<{ animationDone?: boolean }>`
  width: 100%;
  height: 100vh;
  display: flex;
  justify-content: center;
  align-items: center;
  opacity: ${({ animationDone }) => (animationDone ? 0 : 1)};
  background: linear-gradient(to bottom, #3f51b5, #2196f3);
  color: #fff;
  transition: opacity 1s ease-in-out;
  animation-name: ${({ animationDone }) => (animationDone ? fadeOut : fadeIn)};
  animation-duration: 2s;
`;

export const Title = styled.h1`
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  font-size: 40px;
  animation-name: ${fadeIn};
  animation-duration: 2s;
  opacity: ${({ animationDone }) => (animationDone ? 0 : 1)};
  transition: opacity 1s ease-in-out;
`;
