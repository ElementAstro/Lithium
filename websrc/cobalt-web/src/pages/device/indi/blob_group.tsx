import React, { useState, useRef, useEffect } from "react";
import { Form, Button } from "react-bootstrap";
import { PencilSquare } from "react-bootstrap-icons";
import { useTranslation } from "react-i18next";

import DeviceControlLabelDescription from "../description/panel_lable";
import HelperSnackbar from "../description/helper_snackbar";

interface IndiBlobGroupProps {
  device_name: string;
  data: IndiPropertyBlobDataStruct;
  on_data_change: (
    device_name: string,
    property_name: string,
    new_data: IndiPropertyBlobDataStruct
  ) => void;
}

interface IndiBlobSingleCompProps {
  device_name: string;
  data: IndiBlobStruct;
  property_name: string;
  index: number;
  permission: "ReadOnly" | "WriteOnly" | "ReadWrite";
  on_data_change: (
    device_name: string,
    property_name: string,
    index: number,
    new_data: IndiBlobStruct
  ) => void;
}

const IndiBlobCompGroup: React.FC<IndiBlobGroupProps> = (props) => {
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
    new_data: IndiBlobStruct
  ) => {
    let this_property: IndiPropertyBlobDataStruct = JSON.parse(
      JSON.stringify(props.data)
    );
    this_property.property_info.data[index] = new_data;
    props.on_data_change(device_name, property_name, this_property);
  };
  return (
    <>
      <h5>{this_description.show_name}</h5>
      {props.data.property_info.data.map((one_indi_blob_struct, index) => {
        return (
          <IndiSingleBlobComp
            key={index}
            device_name={props.device_name}
            property_name={props.data.property_name}
            data={one_indi_blob_struct}
            index={index}
            permission={props.data.property_info.permission}
            on_data_change={on_data_change_in_group}
          />
        );
      })}
    </>
  );
};

export const IndiSingleBlobComp: React.FC<IndiBlobSingleCompProps> = (
  props
) => {
  const helper_Ref = useRef<HelperHandle>(null);
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
  return (
    <div className="mt-1">
      <Form className="d-flex align-items-center">
        <Form.Group className="mb-0" style={{ width: "90%" }}>
          <Form.Label>{this_description.show_name}</Form.Label>
          <Form.Control type="text" disabled value={props.data.size} />
        </Form.Group>
        <HelperSnackbar
          ref={helper_Ref}
          help_text={this_description.tooltips}
          close_signal={false}
        />
      </Form>
    </div>
  );
};

export default IndiBlobCompGroup;
