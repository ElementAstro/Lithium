import React from "react";
import { Container, Row, Col, Button } from "react-bootstrap";
import {
  Check2Circle,
  Arrow90degLeft,
  InfoCircle,
} from "react-bootstrap-icons";
import { useTranslation } from "react-i18next";

import DeviceControlLabelDescription from "../description/panel_lable";
import HelperSnackbar from "../description/helper_snackbar";

interface IndiLightGroupProps {
  device_name: string;
  data: IndiPropertyLightDataStruct;
  on_data_change: (
    device_name: string,
    property_name: string,
    new_data: IndiPropertyLightDataStruct
  ) => void;
}

interface IndiLightSingleCompProps {
  device_name: string;
  data: IndiLightStruct;
  property_name: string;
  index: number;
  permission: "ReadOnly" | "WriteOnly" | "ReadWrite";
  on_data_change: (
    device_name: string,
    property_name: string,
    index: number,
    new_data: IndiLightStruct
  ) => void;
}

const IndiLightCompGroup: React.FC<IndiLightGroupProps> = (props) => {
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
    new_data: IndiLightStruct
  ) => {
    let this_property: IndiPropertyLightDataStruct = JSON.parse(
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
          <IndiSingleLightComp
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

export const IndiSingleLightComp: React.FC<IndiLightSingleCompProps> = (
  props
) => {
  const helper_Ref = React.useRef<HelperHandle>(null);
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
  const parse_status = (status: string) => {
    switch (status) {
      case "Idle": {
        return (
          <Button variant="outline-success">
            <Check2Circle />
            {t("standby")}
          </Button>
        );
      }
      case "Ok": {
        return (
          <Button variant="outline-primary">
            <Check2Circle />
            {t("standby")}
          </Button>
        );
      }
      case "Busy": {
        return (
          <Button variant="outline-warning">
            <Arrow90degLeft />
            {t("running")}
          </Button>
        );
      }
      case "Alert": {
        return (
          <Button variant="outline-danger">
            <InfoCircle />
            {t("alarm")}
          </Button>
        );
      }
      default: {
        return (
          <Button variant="outline-danger">
            <InfoCircle />
            {t("unknown_status")}
          </Button>
        );
      }
    }
  };
  return (
    <Container fluid>
      <Row className="mt-1">
        <Col xs={11}>
          {props.data.label}
          {parse_status(props.data.status)}
        </Col>
        <Col xs={1}></Col>
        <HelperSnackbar
          ref={helper_Ref}
          help_text={this_description.tooltips}
          close_signal={false}
        />
      </Row>
    </Container>
  );
};

export default IndiLightCompGroup;
