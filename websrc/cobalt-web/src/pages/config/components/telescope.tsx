import React from "react";
import { Card, Form, Row, Col, InputGroup, Button } from "react-bootstrap";
import { InfoCircle } from "react-bootstrap-icons";
import { GlobalStore } from "../../../store/globalStore";

const GPTelescopeEasyUseComp: React.FC = () => {
  const global_parameter = GlobalStore.useAppState(
    (state) => state.GlobalParameterStore
  );

  const [main_focal_length, set_main_focal_length] = React.useState("0");
  const [main_apeture, set_main_apeture] = React.useState("0");
  const [guider_focal_length, set_guider_focal_length] = React.useState("0");
  const [guider_apeture, set_guider_apeture] = React.useState("0");

  React.useEffect(() => {
    if (global_parameter.global_parameter.telescope_info !== null) {
      set_main_apeture(
        String(global_parameter.global_parameter.telescope_info?.aperture)
      );
      set_main_focal_length(
        String(global_parameter.global_parameter.telescope_info?.focal_length)
      );
      set_guider_apeture(
        String(
          global_parameter.global_parameter.telescope_info?.guider_aperture
        )
      );
      set_guider_focal_length(
        String(
          global_parameter.global_parameter.telescope_info?.guider_focal_length
        )
      );
    }
  }, [
    global_parameter.global_parameter.telescope_info?.focal_length,
    global_parameter.global_parameter.telescope_info?.aperture,
    global_parameter.global_parameter.telescope_info?.guider_aperture,
    global_parameter.global_parameter.telescope_info?.guider_focal_length,
  ]);

  const on_telescope_modify_blur = (data_type: string) => {
    let to_set_value: number;
    switch (data_type) {
      case "aperture":
        try {
          to_set_value = Number(main_apeture);
          GlobalStore.actions.GlobalParameterStore.set_parameter({
            parameter_name: "telescope_info",
            to_set_parameter: {
              aperture: to_set_value,
            },
          });
        } catch (error) {}
        break;
      case "focal_length":
        try {
          to_set_value = Number(main_focal_length);
          GlobalStore.actions.GlobalParameterStore.set_parameter({
            parameter_name: "telescope_info",
            to_set_parameter: {
              focal_length: to_set_value,
            },
          });
        } catch (error) {}
        break;
      case "guider_aperture":
        try {
          to_set_value = Number(guider_apeture);
          GlobalStore.actions.GlobalParameterStore.set_parameter({
            parameter_name: "telescope_info",
            to_set_parameter: {
              guider_aperture: to_set_value,
            },
          });
        } catch (error) {}
        break;
      case "guider_focal_length":
        try {
          to_set_value = Number(guider_focal_length);
          GlobalStore.actions.GlobalParameterStore.set_parameter({
            parameter_name: "telescope_info",
            to_set_parameter: {
              guider_focal_length: to_set_value,
            },
          });
        } catch (error) {}
        break;
      default:
        break;
    }
  };
  return (
    <>
      <Card>
        <Card.Header>
          <Card.Title>
            <h2>
              <InfoCircle className="me-2" />
              主镜参数
            </h2>
          </Card.Title>
        </Card.Header>
        <Card.Body>
          <Form.Group as={Row} controlId="mainFocalLength">
            <Form.Label column sm={4}>
              主镜焦距
            </Form.Label>
            <Col sm={8}>
              <InputGroup>
                <Form.Control
                  type="text"
                  placeholder="输入焦距"
                  value={main_focal_length}
                  onChange={(e) => set_main_focal_length(e.target.value)}
                  onBlur={() => on_telescope_modify_blur("focal_length")}
                />
                <InputGroup.Text id="basic-addon2">mm</InputGroup.Text>
              </InputGroup>
            </Col>
          </Form.Group>

          <Form.Group as={Row} controlId="mainAperture">
            <Form.Label column sm={4}>
              主镜口径
            </Form.Label>
            <Col sm={8}>
              <InputGroup>
                <Form.Control
                  type="text"
                  placeholder="输入口径"
                  value={main_apeture}
                  onChange={(e) => set_main_apeture(e.target.value)}
                  onBlur={() => on_telescope_modify_blur("aperture")}
                />
                <InputGroup.Text id="basic-addon2">mm</InputGroup.Text>
              </InputGroup>
            </Col>
          </Form.Group>
        </Card.Body>
      </Card>

      <Card>
        <Card.Header>
          <Card.Title>
            <h2>
              <InfoCircle className="me-2" />
              导星镜参数
            </h2>
          </Card.Title>
        </Card.Header>
        <Card.Body>
          <Form.Group as={Row} controlId="guiderFocalLength">
            <Form.Label column sm={4}>
              导星焦距
            </Form.Label>
            <Col sm={8}>
              <InputGroup>
                <Form.Control
                  type="text"
                  placeholder="输入焦距"
                  value={guider_focal_length}
                  onChange={(e) => set_guider_focal_length(e.target.value)}
                  onBlur={() => on_telescope_modify_blur("guider_focal_length")}
                />
                <InputGroup.Text id="basic-addon2">mm</InputGroup.Text>
              </InputGroup>
            </Col>
          </Form.Group>

          <Form.Group as={Row} controlId="guiderAperture">
            <Form.Label column sm={4}>
              导星口径
            </Form.Label>
            <Col sm={8}>
              <InputGroup>
                <Form.Control
                  type="text"
                  placeholder="输入口径"
                  value={guider_apeture}
                  onChange={(e) => set_guider_apeture(e.target.value)}
                  onBlur={() => on_telescope_modify_blur("guider_aperture")}
                />
                <InputGroup.Text id="basic-addon2">mm</InputGroup.Text>
              </InputGroup>
            </Col>
          </Form.Group>
        </Card.Body>
      </Card>
    </>
  );
};

export default GPTelescopeEasyUseComp;
