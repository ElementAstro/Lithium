import React from "react";
import { Container, Row, Col } from "react-bootstrap";
import { useTranslation } from "react-i18next";
import IndiNumberCompGroup from "./number_group";
import IndiTextCompGroup from "./text_group";
import IndiLightCompGroup from "./light_group";
import IndiSwitchCompGroup from "./swtich_group";
import IndiBlobCompGroup from "./blob_group";
import { WaterfallElement } from "./style";

interface RenderPropertyProps {
  device_name: string;
  property_data: IndiPropertyGroupStruct;
  on_data_change: (
    device_name: string,
    property_name: string,
    new_data:
      | IndiPropertyNumberDataStruct
      | IndiPropertyTextDataStruct
      | IndiPropertySwitchDataStruct
      | IndiPropertyBlobDataStruct
      | IndiPropertyLightDataStruct
  ) => void;
}

const IndiRenderOnePropertyGroup: React.FC<RenderPropertyProps> = (props) => {
  const { t } = useTranslation();

  const renderPropertyGroup = () => {
    if (props.property_data.property_info.type === "NUMBER") {
      return (
        <IndiNumberCompGroup
          device_name={props.device_name}
          data={props.property_data as IndiPropertyNumberDataStruct}
          on_data_change={props.on_data_change}
        />
      );
    } else if (props.property_data.property_info.type === "TEXT") {
      return (
        <IndiTextCompGroup
          device_name={props.device_name}
          data={props.property_data as IndiPropertyTextDataStruct}
          on_data_change={props.on_data_change}
        />
      );
    } else if (props.property_data.property_info.type === "SWITCH") {
      return (
        <IndiSwitchCompGroup
          device_name={props.device_name}
          data={props.property_data as IndiPropertySwitchDataStruct}
          on_data_change={props.on_data_change}
        />
      );
    } else if (props.property_data.property_info.type === "BLOB") {
      return (
        <IndiBlobCompGroup
          device_name={props.device_name}
          data={props.property_data as IndiPropertyBlobDataStruct}
          on_data_change={props.on_data_change}
        />
      );
    } else if (props.property_data.property_info.type === "LIGHT") {
      return (
        <IndiLightCompGroup
          device_name={props.device_name}
          data={props.property_data as IndiPropertyLightDataStruct}
          on_data_change={props.on_data_change}
        />
      );
    } else {
      return null;
    }
  };

  return (
    <Container fluid>
      <WaterfallElement>
        <Row>
          <Col>{renderPropertyGroup()}</Col>
        </Row>
      </WaterfallElement>
    </Container>
  );
};

export default IndiRenderOnePropertyGroup;
