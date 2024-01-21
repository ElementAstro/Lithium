import styled, { keyframes } from "styled-components";
import { DetailedHTMLProps, HTMLAttributes } from "react";
import { FastOmit } from "ts-essentials";

const fadeIn = keyframes`
  from {
    opacity: 0;
  }
  to {
    opacity: 1;
  }
`;

const fadeOut = keyframes`
  from {
    opacity: 1;
  }
  to {
    opacity: 0;
  }
`;

type OpeningAnimationProps = FastOmit<DetailedHTMLProps<HTMLAttributes<HTMLDivElement>, HTMLDivElement>, "fadeOutAnimation"> & {
  fadeOutAnimation?: boolean;
};

const OpeningAnimation = styled.div<OpeningAnimationProps>`
  width: 100%;
  height: 100vh;
  display: flex;
  justify-content: center;
  align-items: center;
  opacity: 0;
  background: linear-gradient(to bottom, #3f51b5, #2196f3);
  color: #fff;
  transition: opacity 1s ease-in-out;
  animation-name: ${({ fadeOutAnimation }) => (fadeOutAnimation ? fadeOut : fadeIn)};
  animation-duration: 2s;
`;

