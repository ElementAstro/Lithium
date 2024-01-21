import React from "react";
import { Form, Row, Col } from "react-bootstrap";
import { QuestionCircleFill } from "react-bootstrap-icons";
import HelperSnackbar from "../description/helper_snackbar";
import DeviceControlLabelDescription from "../description/panel_lable";

interface IndiSwitchGroupProps {
  device_name: string;
  data: IndiPropertySwitchDataStruct;
  on_data_change: (
    device_name: string,
    property_name: string,
    new_data: IndiPropertySwitchDataStruct
  ) => void;
}

interface IndiSwitchSingleCompProps {
  device_name: string;
  data: IndiSwitchStruct;
  property_name: string;
  index: number;
  permission: "ReadOnly" | "WriteOnly" | "ReadWrite";
  on_data_change: (
    device_name: string,
    property_name: string,
    index: number,
    new_data: IndiSwitchStruct
  ) => void;
}

const IndiSwitchCompGroup: React.FC<IndiSwitchGroupProps> = (props) => {
  let this_description: IndiLabelDescription;
  if (props.data.property_name in DeviceControlLabelDescription) {
    this_description = DeviceControlLabelDescription[props.data.property_name];
  } else {
    this_description = {
      name: props.data.property_name,
      show_name: props.data.property_name,
      tooltips: "",
    };
  }
  const on_data_change_in_group = (
    device_name: string,
    property_name: string,
    index: number,
    new_data: IndiSwitchStruct
  ) => {
    let this_property: IndiPropertySwitchDataStruct = JSON.parse(
      JSON.stringify(props.data)
    );
    this_property.property_info.data[index] = new_data;
    props.on_data_change(device_name, property_name, this_property);
  };
  return (
    <>
      <h5>{this_description.show_name}</h5>
      {props.data.property_info.data.map((one_indi_number_struct, index) => {
        return (
          <IndiSingleSwitchComp
            key={index}
            device_name={props.device_name}
            property_name={props.data.property_name}
            data={one_indi_number_struct}
            index={index}
            permission={props.data.property_info.permission}
            on_data_change={on_data_change_in_group}
          />
        );
      })}
    </>
  );
};

export const IndiSingleSwitchComp: React.FC<IndiSwitchSingleCompProps> = (
  props
) => {
  const helper_Ref = React.useRef<HelperHandle>(null);
  const [show_switch, set_show_switch] = React.useState(props.data.switch);
  let this_description: IndiLabelDescription;
  if (props.data.label in DeviceControlLabelDescription) {
    this_description = DeviceControlLabelDescription[props.data.label];
  } else {
    this_description = {
      name: props.data.label,
      show_name: props.data.label,
      tooltips: "",
    };
  }
  React.useEffect(() => {
    set_show_switch(props.data.switch);
  }, [props.data.switch]);
  return (
    <>
      <Row className="mt-1">
        <Col xs={11}>
          <Form.Check
            type="switch"
            id={`switch-${props.device_name}-${props.property_name}-${props.index}`}
            label={this_description.show_name}
            checked={show_switch}
            onChange={(event) => {
              set_show_switch(event.target.checked);
              let new_data: IndiSwitchStruct = JSON.parse(
                JSON.stringify(props.data)
              );
              new_data.switch = event.target.checked;
              props.on_data_change(
                props.device_name,
                props.property_name,
                props.index,
                new_data
              );
            }}
            disabled={props.permission === "ReadOnly"}
          />
        </Col>
        <Col xs={1}>
          <QuestionCircleFill
            onClick={() => {
              if (helper_Ref.current !== null) {
                helper_Ref.current.open_snackbar();
              }
            }}
          />
        </Col>
      </Row>
      <HelperSnackbar
        ref={helper_Ref}
        help_text={this_description.tooltips}
        close_signal={false}
      />
    </>
  );
};

export default IndiSwitchCompGroup;
