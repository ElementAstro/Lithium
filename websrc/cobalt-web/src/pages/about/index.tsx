import React from "react";
import { useTranslation } from "react-i18next";
import { LinkContainer } from "react-router-bootstrap";
import Help from "./help";
import { Route, Routes } from "react-router-dom";
import {
  StyledContainer,
  TitleRow,
  StyledTitle,
  StyledBadge,
  ContentRow,
  ContentCol,
  FeaturesCard,
  RequirementsCard,
  ThanksCard,
  FAQCard,
  HelpList,
} from "./style";
import { Col, Image, Card, ListGroup } from "react-bootstrap";

function AboutPage() {
  const { t } = useTranslation();

  return (
    <StyledContainer>
      <TitleRow>
        <StyledTitle>
          {t("Lithium")}
          <StyledBadge bg="primary">v1.0.0</StyledBadge>
        </StyledTitle>
      </TitleRow>
      <ContentRow>
        <Col md={4} className="text-center">
          <Image src="atom.png" alt="Software Icon" fluid />
        </Col>
        <ContentCol md={8}>
          <p>
            {t("Lithium, a lively and lightweight astrophotography terminal.")}
          </p>
          <p>
            {t(
              "Based on INDI and ASCOM, it can be used to control and manage cameras, telescopes, and other equipment."
            )}
          </p>
          <p>{t("Released under the GPL3 license.")}</p>
        </ContentCol>
      </ContentRow>
      <FeaturesCard>
        <Card.Header as="h2">{t("Features")}</Card.Header>
        <ListGroup variant="flush"></ListGroup>
      </FeaturesCard>
      <RequirementsCard>
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
      </RequirementsCard>
      <ThanksCard>
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
      </ThanksCard>
      <FAQCard>
        <Card.Header as="h2">{t("FAQ")}</Card.Header>
        <HelpList variant="flush"></HelpList>
      </FAQCard>
    </StyledContainer>
  );
}

export default AboutPage;
