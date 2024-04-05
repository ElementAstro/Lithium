import React, { ChangeEvent, useEffect, useState } from "react";
import { Form, Button, Row, Col, Container, Card } from "react-bootstrap";
import { Trash, ArrowRight, Plus, Send } from "react-bootstrap-icons";

import { useTranslation } from "react-i18next";

import { GlobalStore } from "../../store/globalStore";

import BrandDriverSelectDialog from "./brand_select";

const BrandConnection = () => {
  const { t } = useTranslation();
  const { brand_list, brand_type_cn, brand_type_en, brand_selections } =
    GlobalStore.useAppState((state) => state.connect);

  const [dialog_open, set_dialog_open] = useState(false);
  const [dialog_brand_index, set_dialog_brand_index] = useState(0);
  const [dialog_selections, set_dialog_selections] = useState<Array<any>>([]);

  const handle_dialog_close = () => {
    set_dialog_open(false);
  };

  const handle_dialog_select = (
    selection: IConnectBrandSelection,
    device_type_index: number
  ) => {
    // console.log(selection, device_type_index);
    const deviceType = brand_type_en[device_type_index];
    GlobalStore.actions.connect.setSelectBrand({
      type: deviceType,
      brand_name: selection,
    });
  };
  const on_select_clicked = (device_type_index: number) => {
    set_dialog_open(true);
    if (brand_selections !== null) {
      set_dialog_brand_index(device_type_index);
      set_dialog_selections(
        brand_list[
          brand_type_en[device_type_index] as keyof typeof brand_selections
        ]
      );
    } else {
      set_dialog_selections([]);
    }
  };

  useEffect(() => {
    GlobalStore.actions.connect.getBrandList();
  }, []);

  const handleConnectBrand = async () => {
    // 允许多次连接
    let count_time = 0;
    let connect_ready = false;

    // 启动驱动
    let deviceServerConnectReady =
      await GlobalStore.actions.connect.connectDeviceServer();

    if (!deviceServerConnectReady) {
      alert("当前未选择任何品牌,请重新选择");
      return;
    }
    const res = await GlobalStore.actions.connect.getDeviceList();
    if (res) {
      connect_ready = true;
      GlobalStore.actions.connect.setState({
        brand_selection_over: true,
        config_name: null,
      });
      return;
    }
    // 异常情况, 关闭服务器, 提示用户
    else {
      alert("未扫描得到任何设备, 请重新选择");
      await GlobalStore.actions.connect.closeDeviceServer();
      await Sleep(1000);
    }
  };

  const Sleep = (ms: number) => {
    return new Promise((resolve) => setTimeout(resolve, ms));
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
        camera: { zh: "", en: "", driver: "" },
        telescope: { zh: "", en: "", driver: "" },
        guider: { zh: "", en: "", driver: "" },
        focus: { zh: "", en: "", driver: "" },
        filter: { zh: "", en: "", driver: "" },
        polar: { zh: "", en: "", driver: "" },
      },
    });
  };

  return (
    <Container
      fluid
      style={{
        flexGrow: 1,
        height: "calc(100vh - 32px)",
        overflowY: "auto",
        overflowX: "hidden",
      }}
    >
      <Row>
        {brand_type_cn.map((item: string, type_index: number) => (
          <Col xs={6} key={type_index}>
            <Form.Group
              controlId={`brand-${type_index}`}
              style={{ margin: "1rem", width: "100%" }}
            >
              <Form.Control
                size="sm"
                type="text"
                placeholder={`选择${item}品牌`}
                defaultValue=""
                value={brand_selections[brand_type_en[type_index]]?.zh}
                onClick={() => {
                  on_select_clicked(type_index);
                }}
                readOnly
              />
            </Form.Group>
          </Col>
        ))}
      </Row>
      <BrandDriverSelectDialog
        device_type_index={dialog_brand_index}
        open={dialog_open}
        options={dialog_selections}
        handleClose={handle_dialog_close}
        handleSelect={handle_dialog_select}
      />
      <hr style={{ margin: "1rem 0" }} />
      <Row>
        <Col xs={6}>
          <Button
            variant="outline-secondary"
            style={{ margin: "1rem", width: "100%" }}
            onClick={handleResetBrandSelections}
          >
            <Trash size={20} /> 重置
          </Button>
        </Col>
        <Col xs={6}>
          <Button
            variant="primary"
            style={{ margin: "1rem", width: "100%" }}
            onClick={handleConnectBrand}
          >
            初始化配置 <Send size={20} />
          </Button>
        </Col>
      </Row>
    </Container>
  );
};

export default BrandConnection;
