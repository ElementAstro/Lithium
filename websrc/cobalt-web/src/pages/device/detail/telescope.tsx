import React, { useState, useEffect } from "react";
import { useEchoWebSocket } from "../../../utils/websocketProvider";
import IndiRenderOnePropertyGroup from "../indi/group_render";
import { Container, Row, Col } from "react-bootstrap";
import { GearFill } from "react-bootstrap-icons";
import { StyledContainer } from "./style";

const DeviceTelescopeDetailedControlPanel = () => {
  const [property_list, set_property_list] = useState([]);

  const process_ws_message = (msg) => {
    // console.log('in telescope detail', msg);
    if (
      msg.device_name === "telescope" &&
      msg.instruction === "get_parameter"
    ) {
      set_property_list(msg.data);
    }
    if (
      msg.device_name === "telescope" &&
      msg.instruction === "set_any_one_parameter"
    ) {
      // after set, refresh data
      if (msg.data) {
        // flag is true
      } else {
        // flag is false, call helper
      }
      sendMessage(
        JSON.stringify({
          device_name: "telescope",
          instruction: "get_parameter",
          params: [],
        })
      );
    }
  };

  const set_one_parameter = (device_name, property_name, new_data) => {
    sendMessage(
      JSON.stringify({
        device_name: "telescope",
        instruction: "set_any_one_parameter",
        params: [property_name, new_data.property_info],
      })
    );
  };

  const { sendMessage, removeListener } = useEchoWebSocket(process_ws_message);
  const MINUTE_MS = 2000;

  useEffect(() => {
    // This will fire only on mount.
    // first get static info
    sendMessage(
      JSON.stringify({
        device_name: "telescope",
        instruction: "get_parameter",
        params: [],
      })
    );

    const interval = setInterval(() => {
      // console.log('Logs every second');
      sendMessage(
        JSON.stringify({
          device_name: "telescope",
          instruction: "get_parameter",
          params: [],
        })
      );
    }, MINUTE_MS);

    return () => {
      // console.log('clear interval');
      clearInterval(interval);
      removeListener(process_ws_message);
    }; // This represents the unmount function, in which you need to clear your interval to prevent memory leaks.
  }, []);

  return (
    <StyledContainer>
      <Container fluid>
        <Row>
          {property_list.map((one_property_group, index) => (
            <Col key={index}>
              <IndiRenderOnePropertyGroup
                device_name="telescope"
                property_data={one_property_group}
                on_data_change={set_one_parameter}
              />
            </Col>
          ))}
        </Row>
      </Container>
    </StyledContainer>
  );
};

export default DeviceTelescopeDetailedControlPanel;
