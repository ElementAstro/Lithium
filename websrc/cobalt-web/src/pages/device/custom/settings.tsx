import React, { useState, useEffect } from "react";
import { Collapse, ListGroup, Button, Form, Row, Col } from "react-bootstrap";
import { ArrowDown, ArrowUp, BoundingBox } from "react-bootstrap-icons";
import { useTranslation } from "react-i18next";

interface DeviceCustomSwitchValue {
  data_type: string;
  data: boolean;
  custom_name: string;
  display_label: string;
}

interface DeviceCustomSelectValue {
  data_type: string;
  data: {
    selections: string[];
    selected: string;
  };
  custom_name: string;
  display_label: string;
}

interface DeviceCustomSettingCompProps {
  setting_values: Array<DeviceCustomSwitchValue | DeviceCustomSelectValue>;
  on_value_change: (custom_name: string, value: string | boolean) => void;
}

interface DeviceCustomSingleDisplayCompProps {
  single_setting_value: DeviceCustomSwitchValue | DeviceCustomSelectValue;
  on_value_change: (custom_name: string, value: string | boolean) => void;
}

const DeviceCuostomSingleDisplayComp: React.FC<
  DeviceCustomSingleDisplayCompProps
> = (props) => {
  const [showSwitch, setShowSwitch] = useState(false);
  const [selections, setSelections] = useState<string[]>([]);
  const [selected, setSelected] = useState("");

  useEffect(() => {
    if (props.single_setting_value.data_type === "SWITCH") {
      setShowSwitch(props.single_setting_value.data);
    } else if (props.single_setting_value.data_type === "SELECT") {
      setSelections(props.single_setting_value.data.selections);
      setSelected(props.single_setting_value.data.selected);
    }
  }, []);

  useEffect(() => {
    if (props.single_setting_value.data_type === "SWITCH") {
      setShowSwitch(props.single_setting_value.data);
    } else if (props.single_setting_value.data_type === "SELECT") {
      setSelected(props.single_setting_value.data.selected);
    }
  }, [props.single_setting_value.data]);

  if (props.single_setting_value.data_type === "SWITCH") {
    return (
      <Form.Check
        type="switch"
        id={props.single_setting_value.custom_name}
        label={props.single_setting_value.display_label}
        checked={showSwitch}
        onChange={(event) => {
          setShowSwitch(event.target.checked);
          props.on_value_change(
            props.single_setting_value.custom_name,
            event.target.checked
          );
        }}
      />
    );
  } else if (props.single_setting_value.data_type === "SELECT") {
    return (
      <Form>
        <Form.Group as={Row} controlId={props.single_setting_value.custom_name}>
          <Form.Label column sm="auto">
            {props.single_setting_value.display_label}
          </Form.Label>
          <Col sm="5">
            <Form.Select
              value={selected}
              onChange={(event) => {
                setSelected(event.target.value as string);
                props.on_value_change(
                  props.single_setting_value.custom_name,
                  event.target.value as string
                );
              }}
            >
              {selections.length > 0 &&
                selections.map((one_selection, index) => (
                  <option value={one_selection} key={index}>
                    {one_selection}
                  </option>
                ))}
            </Form.Select>
          </Col>
        </Form.Group>
      </Form>
    );
  } else {
    return <></>;
  }
};

const DeviceCustomSettingComp: React.FC<DeviceCustomSettingCompProps> = (
  props
) => {
  const [openCollapse, setOpenCollapse] = useState(false);

  const handleClick = () => {
    setOpenCollapse(!openCollapse);
  };

  return (
    <ListGroup>
      <ListGroup.Item action onClick={handleClick}>
        <BoundingBox size={18} />
        <span style={{ marginLeft: "5px" }}>设备拓展配置参数</span>
        {openCollapse ? <ArrowUp size={18} /> : <ArrowDown size={18} />}
      </ListGroup.Item>
      <Collapse in={openCollapse}>
        <ListGroup variant="flush">
          {props.setting_values.length > 0 &&
            props.setting_values.map((one_setting_value, index) => (
              <ListGroup.Item key={index}>
                <DeviceCuostomSingleDisplayComp
                  single_setting_value={one_setting_value}
                  on_value_change={props.on_value_change}
                />
              </ListGroup.Item>
            ))}
        </ListGroup>
      </Collapse>
    </ListGroup>
  );
};

export default DeviceCustomSettingComp;
