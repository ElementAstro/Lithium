import React from "react";
import { Accordion, Card } from "react-bootstrap";
import { useTranslation } from "react-i18next";

function FAQAccordion() {
  const { t } = useTranslation();

  return (
    <Accordion defaultActiveKey="0">
      <Card>
        <Accordion as={Card.Header} eventKey="0">
          {t("faq.question1")}
        </Accordion>
        <Accordion.Collapse eventKey="0">
          <Card.Body>
            {t("faq.answer1")}
            <ol>
              <li>{t("faq.step1")}</li>
              <li>{t("faq.step2")}</li>
              <li>{t("faq.step3")}</li>
            </ol>
          </Card.Body>
        </Accordion.Collapse>
      </Card>

      <Card>
        <Accordion as={Card.Header} eventKey="1">
          {t("faq.question2")}
        </Accordion>
        <Accordion.Collapse eventKey="1">
          <Card.Body>{t("faq.answer2")}</Card.Body>
        </Accordion.Collapse>
      </Card>

      <Card>
        <Accordion as={Card.Header} eventKey="2">
          {t("faq.question3")}
        </Accordion>
        <Accordion.Collapse eventKey="2">
          <Card.Body>
            {t("faq.answer3")}
            <ol>
              <li>{t("faq.step1")}</li>
              <li>{t("faq.step2")}</li>
              <li>{t("faq.step3")}</li>
            </ol>
          </Card.Body>
        </Accordion.Collapse>
      </Card>
    </Accordion>
  );
}

export default FAQAccordion;
