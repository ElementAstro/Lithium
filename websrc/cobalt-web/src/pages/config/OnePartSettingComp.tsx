import React, { useEffect, useRef } from "react";
import Accordion from "react-bootstrap/Accordion";
import Card from "react-bootstrap/Card";
import Button from "react-bootstrap/Button";
import Container from "react-bootstrap/Container";
import { ChevronDown } from "react-bootstrap-icons";
import GlobalSettingSingleSettingEntry from "./OneSettingEntryComp";

interface SinglePartSettingProps {
  setting_part_name: string;
  setting_part_value: Partial<IGlobalParameters>;
  handleSettingChange: (
    setting_part_key: string,
    setting_key: string,
    set_value: boolean | number | string | object
  ) => void;
  parameter_explain: IGPParameterExplain;
}

const GlobalSettingOnePartSetting: React.FC<SinglePartSettingProps> = (
  props
) => {
  const parameter_explain = useRef<{
    [key: string]: { name: string; tooltips: string; range?: number[] };
  }>(props.parameter_explain);

  const render_single_part_data = (setting_key: string, set_value: any) => {
    const parameter_found = setting_key in parameter_explain.current;
    var this_name = "";
    var this_tooltips = "";

    if (parameter_found) {
      this_name = parameter_explain.current[setting_key].name;
      this_tooltips = parameter_explain.current[setting_key].tooltips;
    } else {
      this_name = "* " + setting_key;
      this_tooltips = "* " + "???";
    }

    return (
      <GlobalSettingSingleSettingEntry
        setting_part_key={props.setting_part_name}
        key={setting_key}
        setting_key={setting_key}
        setting_name={this_name}
        setting_data={set_value}
        tooltip={this_tooltips}
        handleSettingChange={props.handleSettingChange}
      />
    );
  };

  return (
    <Container fluid>
      {Object.entries(props.setting_part_value).map(
        ([setting_key, set_value]) => (
          <Accordion key={setting_key}>
            <Card>
              <Card.Header>
                <Accordion.Header
                  as={Button}
                  variant="link"
                  eventKey={setting_key}
                >
                  <span style={{ width: "33%", flexShrink: 0 }}>
                    {parameter_explain.current[setting_key]?.name}
                  </span>
                  <span style={{ color: "text.secondary" }}>
                    {parameter_explain.current[setting_key]?.tooltips}
                  </span>
                  <ChevronDown />
                </Accordion.Header>
              </Card.Header>
              <Accordion.Collapse eventKey={setting_key}>
                <Card.Body>
                  {render_single_part_data(setting_key, set_value)}
                </Card.Body>
              </Accordion.Collapse>
            </Card>
          </Accordion>
        )
      )}
    </Container>
  );
};

export default GlobalSettingOnePartSetting;
