import {
  Container,
  Row,
  Col,
  Card,
  ListGroup,
  Image,
  Badge,
  Accordion,
} from "react-bootstrap";
import React from "react";
import { useTranslation } from "react-i18next";
import { LinkContainer } from "react-router-bootstrap";
import Help from "./help";
import { Route, Routes } from "react-router-dom";

function AboutPage() {
  const { t } = useTranslation();

  return (
    <Container>
      <Row className="mt-5">
        <Col md={4} className="text-center">
          <Image src="atom.png" alt="Software Icon" fluid />
        </Col>
        <Col md={8}>
          <Row className="text-center">
            <h1>
              {t("Lithium")}
              <Badge bg="primary">v1.0.0</Badge>
            </h1>
          </Row>
          <Row className="text-center">
            <Col>
              <p>
                {t(
                  "Lithium, a lively and lightweight astrophotography terminal."
                )}
              </p>
              <p>
                {t(
                  "Based on INDI and ASCOM, it can be used to control and manage cameras, telescopes, and other equipment."
                )}
              </p>
              <p>{t("Released under the GPL3 license.")}</p>
            </Col>
          </Row>
        </Col>
      </Row>
      <Row className="mt-5">
        <Col md={12}>
          <Card>
            <Card.Header as="h2">{t("Features")}</Card.Header>
            <ListGroup variant="flush"></ListGroup>
          </Card>
        </Col>
      </Row>
      <Row className="mt-5">
        <Col md={12}>
          <Card>
            <Card.Header as="h2">{t("System Requirements")}</Card.Header>
            <Card.Body>
              <p>
                {t(
                  "Lithium is a cross-platform software that can be installed on Windows, Linux, and Mac."
                )}
              </p>
              <p>
                {t(
                  "For Windows, you can download the executable file from the official website and install it directly."
                )}
              </p>
              <p>
                {t(
                  "For Linux, we only provide the source code due to the many different versions of Linux."
                )}
              </p>
            </Card.Body>
          </Card>
        </Col>
      </Row>
      <Row className="mt-5">
        <Col md={12}>
          <Card>
            <Card.Header as="h2">{t("Thanks")}</Card.Header>
            <Card.Body>
              <p>{t("Lithium includes many dependencies, including:")}</p>
              <ul>
                <li>React</li>
                <li>Bootstrap</li>
                <li>Electron</li>
                <li>Node.js</li>
                <li>INDI</li>
                <li>ASCOM</li>
                <li>OpenCV</li>
                <li>Oatpp</li>
              </ul>
              <p>
                {t(
                  "We are very grateful to these open source projects for supporting our software."
                )}
              </p>
              <p>
                {t(
                  "This software is also built by the open source community, and we are very grateful to them."
                )}
              </p>
              <p>
                {t(
                  "If you have any questions, please visit our official website or contact us directly."
                )}
              </p>
            </Card.Body>
          </Card>
        </Col>
      </Row>
      <Row className="mt-5">
        <Col md={12}>
          <Card>
            <Card.Header as="h2">{t("FAQ")}</Card.Header>
            <Help />
          </Card>
        </Col>
      </Row>
    </Container>
  );
}

export default AboutPage;
