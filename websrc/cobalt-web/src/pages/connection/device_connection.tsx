import React, { useState } from "react";

import { Modal, Form, Button, Row, Col } from "react-bootstrap";
import { Trash, ArrowRight } from "react-bootstrap-icons";

import { useTranslation } from "react-i18next";

import { GlobalStore } from "../../store/globalStore";

const DeviceConnection = () => {
  const { t } = useTranslation();
  const {
    device_list,
    user_config_list,
    open_dialog,
    brand_type_cn,
    brand_type_en,
    brand_connection,
    device_selections,
  } = GlobalStore.useAppState((state) => state.connect);
  const [configName, setConfigName] = useState("");

  const handleNewSetting = () => {
    GlobalStore.actions.connect.loadNewConfig();
    handleCreateClose();
  };

  const handleConnectDevice = () => {
    GlobalStore.actions.connect.connectDevice();
  };

  const handleDeviceChange = (event, index) => {
    const value = event.target.value;
    const selectedConfig = device_list[brand_type_en[index]].find(
      (config) => config.device_name === value
    );
    const deviceType = brand_type_en[index];
    if (device_selections === null) {
      handleResetDeviceSelections();
    }
    GlobalStore.actions.connect.setSelectDevice({
      type: deviceType,
      device_infor: selectedConfig,
    });
  };

  const handleUserConfigLoad = (event) => {
    const value = event.target.value;
    const selectedConfig = user_config_list.find(
      (config) => config.name === value
    );
    if (selectedConfig !== undefined) {
      GlobalStore.actions.connect.setState({
        device_selections: selectedConfig,
      });
    }
  };

  const handleResetDeviceSelections = () => {
    GlobalStore.actions.connect.resetDeviceSelections();
  };

  const handleSettingChange = (event) => {
    setConfigName(event.target.value);
  };

  const handleCreateOpen = () => {
    GlobalStore.actions.connect.setState({
      open_dialog: true,
    });
  };

  const handleCreateClose = () => {
    GlobalStore.actions.connect.setState({
      open_dialog: false,
    });
  };

  const Sleep = (ms: number | undefined) => {
    return new Promise((resolve) => setTimeout(resolve, ms));
  };

  return (
    <>
      {brand_type_cn.map((item, type_index) => (
        <Row key={type_index}>
          <Col md={6}>
            <div style={{ display: "flex", alignItems: "center" }}>
              <Form.Select
                className="m-1"
                aria-label={`选择${item}配置`}
                value={
                  device_selections !== null
                    ? device_selections[brand_type_en[type_index]]
                        ?.device_name || ""
                    : ""
                }
                onChange={(e) => handleDeviceChange(e, type_index)}
              >
                <option value="">空</option>
                {device_list !== null
                  ? device_list[brand_type_en[type_index]].map(
                      (item, index) => (
                        <option key={index} value={item.device_name}>
                          {item.device_name}
                        </option>
                      )
                    )
                  : null}
              </Form.Select>
              {brand_connection[brand_type_en[type_index]] === 0 ? null : (
                <span>
                  {brand_connection[brand_type_en[type_index]] === 1 ? (
                    <Trash color="red" />
                  ) : (
                    <ArrowRight color="green" />
                  )}
                </span>
              )}
            </div>
          </Col>
        </Row>
      ))}
      <Row>
        <Col md={6}>
          <Form.Select
            className="m-1"
            aria-label="加载已有配置"
            value={device_selections !== null ? device_selections.name : ""}
            onChange={handleUserConfigLoad}
          >
            <option value="">空</option>
            {user_config_list !== null
              ? user_config_list.map((item, index) => (
                  <option key={index} value={item.name}>
                    {item.name}
                  </option>
                ))
              : null}
          </Form.Select>
          <Button
            variant="outline-danger"
            className="m-1"
            onClick={handleResetDeviceSelections}
          >
            重置
          </Button>
          <Button
            variant="outline-primary"
            className="m-1"
            onClick={handleCreateOpen}
          >
            建立一个新的配置
          </Button>
          <Button
            variant="primary"
            className="m-2"
            onClick={handleConnectDevice}
          >
            连接设备<ArrowRight/>
          </Button>
        </Col>
      </Row>
      <Modal show={open_dialog} onHide={handleCreateClose}>
        <Modal.Header closeButton>
          <Modal.Title>配置命名</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <p>给新配置取一个名字，使得之后可以使用</p>
          <Form.Control
            autoFocus
            id="name"
            placeholder="名称"
            onChange={handleSettingChange}
          />
        </Modal.Body>
        <Modal.Footer>
          <Button onClick={handleCreateClose}>取消</Button>
          <Button onClick={handleNewSetting}>提交</Button>
        </Modal.Footer>
      </Modal>
    </>
  );
};

export default DeviceConnection;
