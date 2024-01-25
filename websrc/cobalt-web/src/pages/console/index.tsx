import React, { useState, useEffect } from "react";
import { Container, Row, Col } from "react-bootstrap";
import { useTranslation } from "react-i18next";

import { useEchoWebSocket } from "../../utils/websocketProvider";
import DebugWebsocketJsonDisplayer from "./json";
import { ScrollableGrid } from "./style";

const WebsocketDebugging = () => {
  const { t } = useTranslation();

  const [paa, setPaa] = useState([]);
  const [paa_update, setPaa_update] = useState(0);
  const [signal, setSignal] = useState([]);
  const [signal_update, setSignal_update] = useState(0);
  const [phd2_event, set_phd2_event] = useState([]);
  const [phd2_event_update, set_phd2_event_update] = useState(0);
  const [phd2_response, set_phd2_response] = useState([]);
  const [phd2_response_update, set_phd2_response_update] = useState(0);
  const [other, set_other] = useState([]);
  const [other_update, set_other_update] = useState(0);

  const process_ws_message = (msg) => {
    console.log(msg);
    if (msg.device_name === "PAA") {
      setPaa((prev) => [msg, ...prev]);
      setPaa_update((prev) => prev + 1);
    } else if (msg.device_name === "Signal") {
      setSignal((prev) => [msg, ...prev]);
      setSignal_update((prev) => prev + 1);
    } else if (msg.device_name === "PHD2_event") {
      set_phd2_event((prev) => [msg, ...prev]);
      set_phd2_event_update((prev) => prev + 1);
    } else if (msg.device_name === "PHD2_response") {
      set_phd2_response((prev) => [msg, ...prev]);
      set_phd2_response_update((prev) => prev + 1);
    } else {
      set_other((prev) => [msg, ...prev]);
      set_other_update((prev) => prev + 1);
    }
  };

  const { sendMessage, removeListener } = useEchoWebSocket(process_ws_message);

  useEffect(() => {
    return () => {
      removeListener(process_ws_message);
    };
  }, []);

  return (
    <Container>
      <Row>
        <Col xs={6}>
          <ScrollableGrid>
            <DebugWebsocketJsonDisplayer
              title={t("websocketDebugging.paa")}
              json_struct={paa}
              update={paa_update}
            />
          </ScrollableGrid>
        </Col>
        <Col xs={6}>
          <ScrollableGrid>
            <DebugWebsocketJsonDisplayer
              title={t("websocketDebugging.signal")}
              json_struct={signal}
              update={signal_update}
            />
          </ScrollableGrid>
        </Col>
      </Row>
      <Row>
        <Col xs={6}>
          <ScrollableGrid>
            <DebugWebsocketJsonDisplayer
              title={t("websocketDebugging.phd2Event")}
              json_struct={phd2_event}
              update={phd2_event_update}
            />
          </ScrollableGrid>
        </Col>
        <Col xs={6}>
          <ScrollableGrid>
            <DebugWebsocketJsonDisplayer
              title={t("websocketDebugging.phd2Response")}
              json_struct={phd2_response}
              update={phd2_response_update}
            />
          </ScrollableGrid>
        </Col>
      </Row>
      <Row>
        <Col xs={6}>
          <ScrollableGrid>
            <DebugWebsocketJsonDisplayer
              title={t("websocketDebugging.other")}
              json_struct={other}
              update={other_update}
            />
          </ScrollableGrid>
        </Col>
      </Row>
    </Container>
  );
};

export default WebsocketDebugging;
