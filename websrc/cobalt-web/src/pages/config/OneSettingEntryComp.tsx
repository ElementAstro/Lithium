import React, { useState, useEffect } from "react";
import Form from "react-bootstrap/Form";
import FloatingLabel from "react-bootstrap/FloatingLabel";
import Switch from "react-bootstrap/Switch";
import FormControl from "react-bootstrap/FormControl";
import {
  PersonCircle,
  GearFill,
  MoonStarsFill,
  ArrowUpCircleFill,
} from "react-bootstrap-icons";
import { Row, Col } from "react-bootstrap";
import Tooltip from "react-bootstrap/Tooltip";
import OverlayTrigger from "react-bootstrap/OverlayTrigger";
import Dropdown from "react-bootstrap/Dropdown";

import GPRedDotComp from "./ClickQuestionTooltips";

interface SingleSettingPartProps {
  setting_part_key: string;
  setting_key: string; // key in the object
  setting_name: string; // display name
  setting_data:
    | string
    | boolean
    | number
    | "astap"
    | "astronomy"
    | ISingleFilterInfo[];
  show_unit?: string;
  tooltip?: string;
  range?: Array<number> | null;
  handleSettingChange: (
    setting_part_key: string,
    setting_key: string,
    set_value: boolean | number | string | object
  ) => void;
}

const GlobalSettingSingleSettingEntry: React.FC<SingleSettingPartProps> = (
  props
) => {
  const [error, setError] = useState(false);
  const [helperText, setHelperText] = useState(props.tooltip);
  const [show_changing, set_changing_flag] = useState(false);
  const [show_setting_value, set_show_setting_value] = useState(
    props.setting_data
  );

  useEffect(() => {
    console.log("entry found props changed", props.setting_key);
    set_show_setting_value(props.setting_data);
    set_changing_flag(false);
  }, [props.setting_data]);

  const handleInputChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    set_changing_flag(true);
    set_show_setting_value(event.target.value);
  };

  const handleSwitchChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    props.handleSettingChange(
      props.setting_part_key,
      props.setting_key,
      event.target.checked
    );
  };

  const handleDropdownChange = (eventKey: string) => {
    props.handleSettingChange(
      props.setting_part_key,
      props.setting_key,
      eventKey
    );
  };

  const renderTooltip = (tooltip: string) => (
    <Tooltip id="button-tooltip">{tooltip}</Tooltip>
  );

  if (props.setting_key === "use") {
    return (
      <Row>
        <Col sm={10}>
          <FloatingLabel label="解析使用引擎">
            <Form.Select
              value={show_setting_value}
              onChange={(event) =>
                props.handleSettingChange(
                  props.setting_part_key,
                  props.setting_key,
                  event.target.value
                )
              }
            >
              <option value="astap">astap</option>
              <option value="astronomy">astronomy</option>
            </Form.Select>
          </FloatingLabel>
        </Col>
        <Col sm={2}>
          <GPRedDotComp show_changing={show_changing} tooltip={props.tooltip} />
        </Col>
      </Row>
    );
  } else if (props.setting_key === "use_filter") {
    const all_filter_full_name = GlobalStore.useAppState(
      (state) => state.GlobalParameterStore.get_filter_names_full
    );
    const default_empty_selector: IGPFilterSelection[] = [
      { label: "当前滤镜", value: "Current" },
    ];
    const filter_selector = default_empty_selector.concat(all_filter_full_name);
    return (
      <Row>
        <Col sm={10}>
          <h6>{helperText}</h6>
          <Form.Select
            defaultValue={["Current"]}
            onChange={(event) => {
              let selected_value = event.target.value;
              props.handleSettingChange(
                props.setting_part_key,
                props.setting_key,
                selected_value
              );
            }}
          >
            {filter_selector.map((filter) => (
              <option key={filter.value} value={filter.value}>
                {filter.label}
              </option>
            ))}
          </Form.Select>
        </Col>
        <Col sm={2}>
          <GPRedDotComp show_changing={show_changing} tooltip={props.tooltip} />
        </Col>
      </Row>
    );
  } else if (typeof show_setting_value === "boolean") {
    return (
      <Row>
        <Col sm={10}>
          <Form.Check
            type="switch"
            id={props.setting_key}
            label={props.setting_name}
            checked={show_setting_value}
            onChange={handleSwitchChange}
          />
        </Col>
        <Col sm={2}>
          <GPRedDotComp show_changing={show_changing} tooltip={props.tooltip} />
        </Col>
      </Row>
    );
  } else if (typeof show_setting_value === "number") {
    return (
      <Row>
        <Col sm={10}>
          <FloatingLabel label={props.setting_name}>
            <FormControl
              type="number"
              value={show_setting_value}
              min={props.range?.[0] ?? 0}
              max={props.range?.[1] ?? 100}
              step={1}
              onBlur={() => {
                props.handleSettingChange(
                  props.setting_part_key,
                  props.setting_key,
                  show_setting_value
                );
                set_changing_flag(false);
              }}
              onChange={handleInputChange}
              isInvalid={error}
            />
          </FloatingLabel>
        </Col>
        <Col sm={2}>
          <GPRedDotComp show_changing={show_changing} tooltip={props.tooltip} />
        </Col>
      </Row>
    );
  } else if (typeof show_setting_value === "string") {
    return (
      <Row>
        <Col sm={10}>
          <FloatingLabel label={props.setting_name}>
            <FormControl
              type="text"
              value={show_setting_value}
              onBlur={() => {
                props.handleSettingChange(
                  props.setting_part_key,
                  props.setting_key,
                  show_setting_value
                );
                set_changing_flag(false);
              }}
              onChange={handleInputChange}
              isInvalid={error}
            />
          </FloatingLabel>
        </Col>
        <Col sm={2}>
          <GPRedDotComp show_changing={show_changing} tooltip={props.tooltip} />
        </Col>
      </Row>
    );
  } else {
    return <div>奇怪了！</div>;
  }
};

export default GlobalSettingSingleSettingEntry;
