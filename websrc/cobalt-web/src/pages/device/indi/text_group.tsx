import React, { useState, useRef, useEffect } from "react";
import { Form, Button } from "react-bootstrap";
import { PencilSquare } from "react-bootstrap-icons";
import { useTranslation } from "react-i18next";

import DeviceControlLabelDescription from "../description/panel_lable";
import HelperSnackbar from "../description/helper_snackbar";

interface IndiTextGroupProps {
  device_name: string;
  data: IndiPropertyTextDataStruct;
  on_data_change: (
    device_name: string,
    property_name: string,
    new_data: IndiPropertyTextDataStruct
  ) => void;
}

interface IndiTextSingleCompProps {
  device_name: string;
  data: IndiTextStruct;
  property_name: string;
  index: number;
  permission: "ReadOnly" | "WriteOnly" | "ReadWrite";
  on_data_change: (
    device_name: string,
    property_name: string,
    index: number,
    new_data: IndiTextStruct
  ) => void;
}

const IndiTextCompGroup: React.FC<IndiTextGroupProps> = (props) => {
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
    new_data: IndiTextStruct
  ) => {
    let this_property: IndiPropertyTextDataStruct = JSON.parse(
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
          <IndiSingleTextComp
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

export const IndiSingleTextComp: React.FC<IndiTextSingleCompProps> = (
  props
) => {
  const helper_Ref = useRef<HelperHandle>(null);
  const [to_show_value, set_to_show_value] = useState("");
  const { t } = useTranslation();
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
    set_to_show_value(props.data.text);
  }, [props.data.text]);
  return (
    <div className="mt-1">
      <Form className="d-flex align-items-center">
        <Form.Group className="mb-0">
          <Form.Label>{this_description.show_name}</Form.Label>
          <div className="position-relative">
            <Form.Control
              type="text"
              disabled={props.permission === "ReadOnly"}
              value={to_show_value}
              onChange={(event) => set_to_show_value(event.target.value)}
            />
            {props.permission !== "ReadOnly" && (
              <Button
                variant="outline-secondary"
                className="position-absolute top-50 end-0 translate-middle-y"
                onClick={() => {
                  let new_data: IndiTextStruct = JSON.parse(
                    JSON.stringify(props.data)
                  );
                  new_data.text = to_show_value;
                  props.on_data_change(
                    props.device_name,
                    props.property_name,
                    props.index,
                    new_data
                  );
                }}
              >
                <PencilSquare />
              </Button>
            )}
          </div>
        </Form.Group>
      </Form>
      <HelperSnackbar
        ref={helper_Ref}
        help_text={this_description.tooltips}
        close_signal={false}
      />
    </div>
  );
};

export default IndiTextCompGroup;
