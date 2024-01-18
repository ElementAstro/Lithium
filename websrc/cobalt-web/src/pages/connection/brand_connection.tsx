import React, { ChangeEvent, useEffect } from "react";
import { Form, Button, Row, Col, Container, Card } from "react-bootstrap";
import { Trash, ArrowRight, Plus } from "react-bootstrap-icons";

import { useTranslation } from "react-i18next";

import { GlobalStore } from "../../store/globalStore";

const BrandConnection = () => {
  const { t } = useTranslation();
  const { brand_list, brand_type_cn, brand_type_en, brand_selections } =
    GlobalStore.useAppState((state) => state.connect);

  useEffect(() => {
    GlobalStore.actions.connect.getBrandList();
  }, []);

  const handleConnectBrand = async () => {
    let deviceServerConnectReady =
      GlobalStore.actions.connect.connectDeviceServer();

    if (deviceServerConnectReady) {
      GlobalStore.actions.connect.getDeviceList();
      GlobalStore.actions.connect.setState({
        brand_selection_over: true,
      });
    } else {
      console.error(t("品牌连接失败"));
    }
  };

  const handleBrandChange = (
    event: ChangeEvent<HTMLSelectElement>,
    index: number
  ) => {
    if (brand_selections === null) {
      handleResetBrandSelections();
    }
    const value = event.target.value;
    const deviceType = brand_type_en[index];
    GlobalStore.actions.connect.setSelectBrand({
      type: deviceType,
      brand_name: value,
    });
  };

  const handleResetBrandSelections = () => {
    GlobalStore.actions.connect.setState({
      brand_selections: {
        camera: "",
        telescope: "",
        focus: "",
        filter: "",
        guider: "",
        polar: "",
      },
    });
  };

  return (
    <Container>
      <Row>
        <Col md={6}>
          <Card>
            <Card.Body>
              <Form.Group controlId="formBrandName">
                <Form.Label>{t("设备品牌")}</Form.Label>
                <Row>
                  {brand_type_cn.map((item: string, type_index: number) => (
                    <Col xs={6} key={type_index}>
                      <Form.Group>
                        <Form.Label>{item}{t("品牌")}</Form.Label>
                        <Form.Control
                          as="select"
                          value={
                            brand_selections !== null
                              ? brand_selections[
                                  brand_type_en[
                                    type_index
                                  ] as keyof typeof brand_selections
                                ]
                              : ""
                          }
                          onChange={(e) => handleBrandChange(e, type_index)}
                        >
                          <option value="">
                            <em>{t("空")}</em>
                          </option>
                          {brand_list !== null
                            ? brand_list[
                                brand_type_en[
                                  type_index
                                ] as keyof typeof brand_selections
                              ].map((item: any, index: number) => (
                                <option key={index} value={item}>
                                  {item.zh}
                                </option>
                              ))
                            : null}
                        </Form.Control>
                      </Form.Group>
                    </Col>
                  ))}
                </Row>
              </Form.Group>
            </Card.Body>
          </Card>
        </Col>
        <Col md={6}>
          <Card>
            <Card.Body>
              <Button
                variant="outline-primary"
                onClick={handleResetBrandSelections}
              >
                <Trash />
                {t("重置")}
              </Button>
              <Button variant="primary" onClick={handleConnectBrand}>
                <ArrowRight />
                {t("初始化配置")}
              </Button>
            </Card.Body>
          </Card>
        </Col>
      </Row>
    </Container>
  );
};

export default BrandConnection;
