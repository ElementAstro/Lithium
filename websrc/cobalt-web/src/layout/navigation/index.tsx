import React, { useState } from "react";
import Container from "react-bootstrap/Container";
import Nav from "react-bootstrap/Nav";
import Navbar from "react-bootstrap/Navbar";
import NavDropdown from "react-bootstrap/NavDropdown";
import Offcanvas from "react-bootstrap/Offcanvas";
import { LinkContainer } from "react-router-bootstrap";
import {
  Info,
  People,
  House,
  Cpu,
  Map,
  Gear,
  WindowDash,
  Webcam,
  Box,
  Bookmark,
  Camera,
} from "react-bootstrap-icons";
import { useTranslation } from "react-i18next";
import { Button } from "react-bootstrap";

function Navigation() {
  const { t } = useTranslation();

  const [language, setLanguage] = useState("en");

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
                <LinkContainer to="/dashboard">
                  <Nav.Link>
                    <House size={24} />
                    Dashboard
                  </Nav.Link>
                </LinkContainer>
                <LinkContainer to="/drivers">
                  <Nav.Link>
                    <Cpu size={24} />
                    Drivers
                  </Nav.Link>
                </LinkContainer>
                <LinkContainer to="/skymap">
                  <Nav.Link>
                    <Map size={24} />
                    Skymap
                  </Nav.Link>
                </LinkContainer>
                <LinkContainer to="/settings">
                  <Nav.Link>
                    <Gear size={24} />
                    Settings
                  </Nav.Link>
                </LinkContainer>
                <NavDropdown
                  title="Utils"
                  id={`offcanvasNavbarDropdown-expand-xxl`}
                >
                  <NavDropdown.Item href="#action4">
                    <WindowDash size={24} />
                    NoVNC
                  </NavDropdown.Item>
                  <NavDropdown.Divider />
                  <NavDropdown.Item href="#action5">
                    <Webcam size={24} />
                    INDIWeb
                  </NavDropdown.Item>
                </NavDropdown>
                <LinkContainer to="/about">
                  <Nav.Link href="#action6">
                    <People size={24} />
                    About
                  </Nav.Link>
                </LinkContainer>
                <LinkContainer to="/help">
                  <Nav.Link href="#action6">
                    <Info size={24} />
                    Help
                  </Nav.Link>
                </LinkContainer>
              </Nav>
            </Offcanvas.Body>
          </Navbar.Offcanvas>
        </Container>
      </Navbar>
    </>
  );
}

export default Navigation;
