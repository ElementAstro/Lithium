import React, { useState } from "react";

import { Modal, Form, Button, Row, Col, Container } from "react-bootstrap";
import {
  Trash,
  ArrowRight,
  CheckCircle,
  Send,
  Plus,
  CircleFill,
  XCircle,
  Archive,
  CodeSquare,
} from "react-bootstrap-icons";

import { useTranslation } from "react-i18next";

import { GlobalStore } from "../../store/globalStore";
import Phd2ConfigEditor from "./phd2_connection";

const DeviceConnection = () => {
  const { t } = useTranslation();
  const {
    device_list,
    user_config_list,
    open_dialog,
    config_name,
    already_connect,
    brand_type_cn,
    brand_type_en,
    brand_connection,
    device_selections,
    phd2_connect_ready,
  } = GlobalStore.useAppState((state) => state.connect);

  const [isDialogOpen, setDialogOpen] = React.useState(false);
  const [isAlertOpen, setAlertOpen] = React.useState(false);

  const handleNewSetting = () => {
    if (config_name !== null) {
      GlobalStore.actions.connect.setProfile(config_name);
      // 关闭Dialog
      handleCreateClose();
    }
  };

  // 用户完成配置时，调用connectDevice正式启动连接
  const handleConnectDevice = () => {
    if (already_connect) {
      // 提示用户
      setAlertOpen(true);
    } else {
      connectToDevice();
    }
  };

  const connectToDevice = () => {
    GlobalStore.actions.connect.connectDevice();
    handleAlertClose();
  };

  const handleDeviceChange = (event: SelectChangeEvent, index: number) => {
    const value = event.target.value;
    const selectedConfig = device_list[brand_type_en[index]].find(
      (config: { device_name: string }) => config.device_name === value
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

  // 切换一个新的配置
  const handleUserConfigLoad = (event: SelectChangeEvent) => {
    const value = event.target.value;
    const selectedConfig = value;
    GlobalStore.actions.connect.setState({
      config_name: value,
    });
    GlobalStore.actions.connect.setProfile(selectedConfig);
  };
  // 重置
  const handleResetDeviceSelections = () => {
    GlobalStore.actions.connect.resetDeviceSelections();
  };

  const handleSettingChange = (event: any) => {
    GlobalStore.actions.connect.setState({
      config_name: event.target.value,
    });
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

  const handleStartPHD2 = async () => {
    let connectReady = await GlobalStore.actions.connect.startPhd2();
    if (connectReady) {
      // todo modify later.
      alert("PHD2 start OK");
    } else {
      alert("PHD2 Error");
    }
  };
  //   休眠函数
  const Sleep = (ms: number) => {
    return new Promise((resolve) => setTimeout(resolve, ms));
  };

  const handleOpenDialog = () => {
    setDialogOpen(true);
  };

  const handleCloseDialog = () => {
    setDialogOpen(false);
  };

  const handleDeleteConfig = (name: string) => {
    GlobalStore.actions.connect.deleteProfile(name);
  };

  const handleAlertClose = () => {
    setAlertOpen(false);
  };

  return (
    <Container
      fluid
      style={{ flexGrow: 1, height: "calc(100vh - 32px)", overflow: "auto" }}
    >
      <Row>
        <Col xs={6}>
          <div style={{ overflow: "auto" }}>
            {brand_type_cn.map((item: string, type_index: number) => (
              <React.Fragment key={type_index}>
                <div style={{ display: "flex", alignItems: "center" }}>
                  <Form.Group
                    controlId={`device-select-${type_index}`}
                    style={{ margin: "1rem", width: "90%" }}
                  >
                    <Form.Label>选择{item}配置</Form.Label>
                    <Form.Select
                      size="sm"
                      value={
                        device_selections !== null
                          ? device_selections[
                              brand_type_en[
                                type_index
                              ] as keyof typeof device_selections
                            ]?.device_name || ""
                          : ""
                      }
                      onChange={(e) => handleDeviceChange(e, type_index)}
                    >
                      <option value="">空</option>
                      {device_list !== null
                        ? device_list[brand_type_en[type_index]].map(
                            (item: any, index: number) => (
                              <option key={index} value={item.device_name}>
                                {item.device_name}
                              </option>
                            )
                          )
                        : null}
                    </Form.Select>
                  </Form.Group>
                  {brand_connection[brand_type_en[type_index]] === 0 ? null : (
                    <div className="ms-2">
                      {brand_connection[brand_type_en[type_index]] === 1 ? (
                        <CircleFill size={20} />
                      ) : (
                        <CheckCircle size={20} />
                      )}
                    </div>
                  )}
                </div>
              </React.Fragment>
            ))}
          </div>
        </Col>
        <Col xs={6}>
          <div className="d-grid gap-2">
            <Form.Group controlId="config-select" style={{ margin: "1rem" }}>
              <Form.Label>加载已有配置</Form.Label>
              <Form.Select
                onChange={handleUserConfigLoad}
                className="m-1"
                aria-label="加载已有配置"
                value={device_selections !== null ? device_selections.name : ""}
              >
                <option value="">空</option>
                {user_config_list !== null
                  ? user_config_list.map((item: any, index: number) => (
                      <option key={index} value={item}>
                        {item}
                      </option>
                    ))
                  : null}
              </Form.Select>
            </Form.Group>
            <Button
              variant="outline-secondary"
              style={{ margin: "1rem" }}
              onClick={handleResetDeviceSelections}
            >
              <Trash size={20} /> 重置
            </Button>
            <Button
              variant="outline-primary"
              style={{ margin: "1rem" }}
              onClick={handleCreateOpen}
            >
              <Plus size={20} /> 建立新配置
            </Button>
            <Button
              variant="primary"
              style={{ margin: "2rem" }}
              onClick={handleConnectDevice}
            >
              <ArrowRight size={20} />
              连接设备
            </Button>
            {phd2_connect_ready && (
              <Button
                variant="primary"
                style={{ margin: "2rem" }}
                onClick={handleStartPHD2}
              >
                <Send size={20} />
                启动并连接PHD2
              </Button>
            )}
          </div>
          <Modal show={open_dialog} onHide={handleCreateClose}>
            <Modal.Header closeButton>
              <Modal.Title>配置命名</Modal.Title>
            </Modal.Header>
            <Modal.Body>
              <Form.Group controlId="config-name" style={{ margin: "1rem" }}>
                <Form.Label>配置名称</Form.Label>
                <Form.Control
                  type="text"
                  autoFocus
                  id="name"
                  title="名称"
                  onChange={handleSettingChange}
                />
              </Form.Group>
            </Modal.Body>
            <Modal.Footer>
              <Button onClick={handleCreateClose}>
                <XCircle size={20} /> 取消
              </Button>
              <Button onClick={handleNewSetting}>
                <Archive size={20} /> 提交
              </Button>
            </Modal.Footer>
          </Modal>
          <Button onClick={handleOpenDialog}>
            <CodeSquare size={20} /> 配置Phd2相关内容
          </Button>
          <Phd2ConfigEditor open={isDialogOpen} onClose={handleCloseDialog} />
        </Col>
      </Row>
    </Container>
  );
};

export default DeviceConnection;
