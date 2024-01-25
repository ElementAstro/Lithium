import React, { useState, useEffect, useRef } from "react";
import { Form, Button, Row, Col, InputGroup } from "react-bootstrap";
import { InfoCircleFill } from "react-bootstrap-icons";
import DeviceControlLabelDescription from "../description/panel_lable";
import HelperSnackbar from "../description/helper_snackbar";
import { useTranslation } from "react-i18next";

interface IndiNumberGroupProps {
  device_name: string;
  data: IndiPropertyNumberDataStruct;
  on_data_change: (
    device_name: string,
    property_name: string,
    new_data: IndiPropertyNumberDataStruct
  ) => void;
}

interface IndiNumberSingleCompProps {
  device_name: string;
  data: IndiNumberStruct;
  property_name: string;
  index: number;
  permission: "ReadOnly" | "WriteOnly" | "ReadWrite";
  on_data_change: (
    device_name: string,
    property_name: string,
    index: number,
    new_data: IndiNumberStruct
  ) => void;
}

const IndiNumberCompGroup: React.FC<IndiNumberGroupProps> = (props) => {
  const { t } = useTranslation();
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
    new_data: IndiNumberStruct
  ) => {
    let this_property: IndiPropertyNumberDataStruct = JSON.parse(
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
          <IndiSingleNumberComp
            key={index}
            device_name={props.device_name}
            data={one_indi_number_struct}
            property_name={props.data.property_name}
            index={index}
            permission={props.data.property_info.permission}
            on_data_change={on_data_change_in_group}
          />
        );
      })}
    </>
  );
};

export const IndiSingleNumberComp: React.FC<IndiNumberSingleCompProps> = (
  props
) => {
  const { t } = useTranslation();
  const helper_Ref = useRef<HelperHandle>(null);
  const [to_show_value, set_to_show_value] = useState("0");
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
  useEffect(() => {
    set_to_show_value(String(props.data.value));
  }, [props.data.value]);
  return (
    <Row className="mt-1">
      <Col xs={11}>
        <InputGroup>
          <Form.Label>
            {this_description.show_name}
            {this_description.tooltips && (
              <InfoCircleFill
                className="ms-1"
                size={16}
                color="gray"
                onClick={() => helper_Ref.current?.toggle()}
              />
            )}
          </Form.Label>
          <Form.Control
            disabled={props.permission == "ReadOnly" ? true : false}
            type="number"
            value={to_show_value}
            onChange={(event) => {
              set_to_show_value(event.target.value);
            }}
          />
          <Button
            variant="primary"
            disabled={props.permission == "ReadOnly" ? true : false}
            onClick={() => {
              let new_data: IndiNumberStruct = JSON.parse(
                JSON.stringify(props.data)
              );
              new_data.value = parseFloat(to_show_value);
              if (new_data.value > new_data.max) {
                new_data.value = new_data.max;
              } else if (new_data.value < new_data.min) {
                new_data.value = new_data.min;
              }
              props.on_data_change(
                props.device_name,
                props.property_name,
                props.index,
                new_data
              );
            }}
          >
            {t("Modify")}
          </Button>
        </InputGroup>
      </Col>
      <HelperSnackbar
        ref={helper_Ref}
        help_text={this_description.tooltips}
        close_signal={false}
      />
    </Row>
  );
};

export default IndiNumberCompGroup;
