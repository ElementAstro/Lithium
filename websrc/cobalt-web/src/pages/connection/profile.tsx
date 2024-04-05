import React, { useEffect, useState } from "react";
import {
  Accordion,
  Button,
  Card,
  Col,
  Container,
  Form,
  Row,
} from "react-bootstrap";
import { Prism as SyntaxHighlighter } from "react-syntax-highlighter";
import { dark } from "react-syntax-highlighter/dist/esm/styles/prism";

import { useTranslation } from "react-i18next";

const ProfileConnection = () => {
  const [configOptions, setConfigOptions] = useState([]);
  const [selectedConfig, setSelectedConfig] = useState("");
  const [configContent, setConfigContent] = useState("");

  const { t } = useTranslation();

  useEffect(() => {
    async function fetchConfigOptions() {
      try {
        const response = await fetch("/api/config/options");
        if (response.ok) {
          const data = await response.json();
          setConfigOptions(data.options);

          const selectedOption = getCookie("selectedConfig");
          if (selectedOption && data.options.includes(selectedOption)) {
            setSelectedConfig(selectedOption);
            const content = getCookie(`configContent_${selectedOption}`);
            if (content) {
              setConfigContent(content);
            }
          }
        }
      } catch (error) {
        console.error(t("Failed to fetch config options:"), error);
      }
    }

    fetchConfigOptions();
  }, []);

  useEffect(() => {
    if (selectedConfig) {
      setCookie("selectedConfig", selectedConfig);
      setCookie(`configContent_${selectedConfig}`, configContent);
    }
  }, [selectedConfig, configContent]);

  const handleConfigSelect = async (event) => {
    const selectedOption = event.target.value;
    setSelectedConfig(selectedOption);

    if (selectedOption) {
      try {
        const response = await fetch(`/api/config/${selectedOption}`);
        if (response.ok) {
          const data = await response.json();
          setConfigContent(data.content);
        } else {
          setConfigContent("");
        }
      } catch (error) {
        console.error("Failed to fetch config content:", error);
        setConfigContent("");
      }
    } else {
      setConfigContent("");
    }
  };

  const handleCreateConfig = () => {
    window.location.href = "/create-config";
  };

  return (
    <Container>
      <Row className="justify-content-center">
        <Col xs={12} md={8}>
          <h2 className="text-center mb-4">{t("配置选择")}</h2>
          <Form>
            <Form.Group controlId="已有配置选择">
              <Form.Label>{t("选择配置")}:</Form.Label>
              <Form.Select value={selectedConfig} onChange={handleConfigSelect}>
                <option value="">{t("选中")}...</option>
                {configOptions.map((option) => (
                  <option key={option} value={option}>
                    {option}
                  </option>
                ))}
              </Form.Select>
            </Form.Group>
          </Form>

          <Accordion className="mt-4">
            <Accordion.Item eventKey="0">
              <Accordion.Header>{t("配置详细信息")}</Accordion.Header>
              <Accordion.Body>
                <Card>
                  <Card.Body>
                    {configContent ? (
                      <SyntaxHighlighter language="json" style={dark}>
                        {configContent}
                      </SyntaxHighlighter>
                    ) : (
                      <p className="text-center">{t("未选择任何配置")}</p>
                    )}
                  </Card.Body>
                </Card>
              </Accordion.Body>
            </Accordion.Item>
          </Accordion>

          <div className="d-grid gap-2 mt-4">
            <Button variant="primary" size="lg" onClick={handleCreateConfig}>
              {t("创建新的配置")}
            </Button>
          </div>
        </Col>
      </Row>
    </Container>
  );
};

export default ProfileConnection;

/**
 * 从 Cookie 中获取指定名称的值。
 * @param {string} name Cookie 名称。
 * @returns {string|null} Cookie 值，如果不存在则返回 null。
 */
function getCookie(name) {
  const value = `; ${document.cookie}`;
  const parts = value.split(`; ${name}=`);
  if (parts.length === 2) {
    const lastPart = parts.pop();
    if (lastPart) {
      return lastPart.split(";").shift();
    }
  }

  return null;
}

/**
 * 将指定名称和值的 Cookie 写入浏览器。
 * @param {string} name Cookie 名称。
 * @param {string} value Cookie 值。
 * @param {number} [days=30] Cookie 过期时间（天数）。默认为 30 天。
 */
function setCookie(name, value, days = 30) {
  const date = new Date();
  date.setTime(date.getTime() + days * 24 * 60 * 60 * 1000);
  const expires = `expires=${date.toUTCString()}`;
  document.cookie = `${name}=${value};${expires};path=/`;
}
