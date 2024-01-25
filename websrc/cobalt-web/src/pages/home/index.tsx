import React from "react";
import { Button, Container } from "react-bootstrap";
import { useTranslation } from "react-i18next";
import { LinkContainer } from "react-router-bootstrap";
import { House, People, Book } from "react-bootstrap-icons";

function Home() {
  const { t } = useTranslation();
  return (
    <Container
      fluid
      className="d-flex flex-column justify-content-center align-items-center"
      style={{
        height: "100vh",
        textAlign: "center",
        backgroundColor: "#E6E6E6",
        backgroundSize: "cover",
        backgroundPosition: "center",
      }}
    >
      <h1 style={{ fontSize: "48px", color: "white" }}>{t("home.title")}</h1>
      <div
        style={{
          display: "flex",
          justifyContent: "center",
          alignItems: "center",
          marginTop: "20px",
        }}
      >
        <LinkContainer to="/dashboard" style={{ marginRight: "10px" }}>
          <Button variant="primary">
            <House size={20} style={{ marginRight: "5px" }} />{" "}
            {t("home.deviceControl")}
          </Button>
        </LinkContainer>
        <LinkContainer to="/community" style={{ marginRight: "10px" }}>
          <Button variant="outline-primary" style={{ marginRight: "10px" }}>
            <People size={20} style={{ marginRight: "5px" }} />{" "}
            {t("home.community")}
          </Button>
        </LinkContainer>
        <LinkContainer to="/about" style={{ marginRight: "10px" }}>
          <Button variant="outline-primary">
            <Book size={20} style={{ marginRight: "5px" }} />{" "}
            {t("home.knowledgeBase")}
          </Button>
        </LinkContainer>
      </div>
    </Container>
  );
}

export default Home;
