import React, { useState } from "react";
import Container from "react-bootstrap/Container";
import Nav from "react-bootstrap/Nav";
import Navbar from "react-bootstrap/Navbar";
import Offcanvas from "react-bootstrap/Offcanvas";
import { LinkContainer } from "react-router-bootstrap";
import {
  Info,
  House,
  Map,
  Gear,
  Webcam,
  Box,
  Bookmark
} from "react-bootstrap-icons";
import { useTranslation } from "react-i18next";

function Navigation() {
  const { t } = useTranslation();

  const [language, setLanguage] = useState("en");

  const navLinks = [
    { to: "/dashboard", text: "Dashboard", icon: House },
    { to: "/server", text: "Server Finder", icon: Info },
    { to: "/device_connect", text: "Device Connector", icon: Webcam },
    { to: "/device", text: "Device Control", icon: Box },
    { to: "/skymap", text: "Skymap & Object", icon: Map },
    { to: "/settings", text: "Settings", icon: Gear },
    { to: "/about", text: "About", icon: Bookmark },
  ];

  const changeLanguage = (lng) => {
    setLanguage(lng);
  };

  return (
    <>
      <Navbar key="xxl" expand="xxl" className="bg-body-tertiary mb-3">
        <Container fluid>
          <Navbar.Brand href="#home">
            <img
              alt=""
              src="atom.png"
              width="30"
              height="30"
              className="d-inline-block align-top"
            />{" "}
            {t("title")}
          </Navbar.Brand>
          <Navbar.Toggle aria-controls={`offcanvasNavbar-routes`} />
          <Navbar.Offcanvas
            id={`offcanvasNavbar-routes`}
            aria-labelledby={`offcanvasNavbarLabel-routes`}
            placement="end"
          >
            <Offcanvas.Header closeButton>
              <Offcanvas.Title id={`offcanvasNavbarLabel-routes`}>
                {t("title")}
              </Offcanvas.Title>
            </Offcanvas.Header>
            <Offcanvas.Body>
              <Nav className="justify-content-end flex-grow-1 pe-3">
                {navLinks.map((link, index) => (
                  <LinkContainer key={index} to={link.to}>
                    <Nav.Link>
                      {React.createElement(link.icon, { size: 24 })}
                      {link.text}
                    </Nav.Link>
                  </LinkContainer>
                ))}
              </Nav>
            </Offcanvas.Body>
          </Navbar.Offcanvas>
        </Container>
      </Navbar>
    </>
  );
}

export default Navigation;
