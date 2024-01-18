import React from "react";
import { Accordion } from "react-bootstrap";
import { useTranslation } from "react-i18next";

const Help = () => {
  const { t, i18n } = useTranslation();
  return (
    <Accordion defaultActiveKey="0" flush>
      <Accordion.Item eventKey="0">
        <Accordion.Header>{t("1.About Lithium")}</Accordion.Header>
        <Accordion.Body>
          {t("Lithium, a lively and lightweight astrophotography terminal.\n")};
          {t(
            "The software is currently in an inner state and is not yet complete. In order to ensure your user experience and device safety, we have not joined it temporarily. In future updates, we will introduce the client."
          )}
          ;
        </Accordion.Body>
      </Accordion.Item>
      <Accordion.Item eventKey="1">
        <Accordion.Header>{t("2.Fetures")}</Accordion.Header>
        <Accordion.Body>
          {t(
            "Can be used as imaging software, device server, and system manager."
          )}
          ;
          {t(
            "Based on the latest C++20 standard, providing efficient functionality implementation (compatible with some C++17 features)."
          )}
          ;
          {t(
            "Supports open loading, allowing dynamic loading of C++ dynamic libraries for hot updates."
          )}
          ;
        </Accordion.Body>
      </Accordion.Item>
      <Accordion.Item eventKey="3">
        <Accordion.Header>{t("3.Usage")}</Accordion.Header>
        <Accordion.Body>
          {t(
            "After installing, you can use the software by opening the software folder and double-clicking the executable file."
          )}
          ;
          {t(
            "Then you can open the browser and enter the software address to use it."
          )}
          ;
        </Accordion.Body>
      </Accordion.Item>
      <Accordion.Item eventKey="4">
        <Accordion.Header>{t("4.License")}</Accordion.Header>
        <Accordion.Body>
          {t(
            "The software is under the GPL3 license, and the source code is open source."
          )}
          ;
        </Accordion.Body>
      </Accordion.Item>
    </Accordion>
  );
};

export default Help;
