import React from "react";
import { FooterContainer, FooterText } from "./style/Footer";

export const Footer = ({ systemInfo }) => {
  return (
    <FooterContainer>
      <FooterText>{systemInfo}</FooterText>
    </FooterContainer>
  );
};
