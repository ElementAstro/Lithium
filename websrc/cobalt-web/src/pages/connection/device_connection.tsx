import React, { useState } from "react";

import { Modal, Form, Button, Row, Col } from "react-bootstrap";
import { Trash, ArrowRight, CheckCircle } from "react-bootstrap-icons";

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
    if (value !== "") {
      GlobalStore.actions.connect.setProfile(selectedConfig);
    }
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
            连接设备
            <ArrowRight />
          </Button>
        </Col>
      </Row>
      {phd2_connect_ready ? (
        <Button
          variant="contained"
          onClick={async () => {
            handleStartPHD2();
          }}
        >
          <CheckCircle />
          启动并连接PHD2
        </Button>
      ) : null}
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
      <Button onClick={handleOpenDialog}>配置Phd2相关内容</Button>
      <Phd2ConfigEditor open={isDialogOpen} onClose={handleCloseDialog} />
    </>
  );
};

export default DeviceConnection;
