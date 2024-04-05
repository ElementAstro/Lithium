import React, { useEffect } from "react";

import { Tab, Tabs, Container } from "react-bootstrap";
import { BoxArrowInRight } from "react-bootstrap-icons";

import { useTranslation } from "react-i18next";

import { GlobalStore } from "../../store/globalStore";

import BrandConnection from "./brand_connection";
import DeviceConnection from "./device_connection";
import ProfileConnection from "./profile";

const Connect = () => {
  const { t } = useTranslation();
  const { setting_mode, brand_selection_over } = GlobalStore.useAppState(
    (state) => state.connect
  );

  useEffect(() => {
    async function fetchData() {
      await GlobalStore.actions.connect.getProfileList();
      await GlobalStore.actions.GlobalParameterStore.get_all_paramters();
      await GlobalStore.actions.connect.getProfileLDevice();
    }
    fetchData();
  }, []);

  const handleModeChange = (newMode) => {
    GlobalStore.actions.connect.setState({
      setting_mode: newMode,
    });
  };

  return (
    <Container>
      <Tabs activeKey={setting_mode} onSelect={handleModeChange}>
        <Tab eventKey={0} title={t("连接配置")}>
          <ProfileConnection />
        </Tab>
        <Tab eventKey={1} title={t("初次连接设置")}>
          <BrandConnection />
        </Tab>
        <Tab
          eventKey={2}
          title={t("按照配置连接选项")}
          //disabled={!brand_selection_over}
        >
          <DeviceConnection />
        </Tab>
      </Tabs>
    </Container>
  );
};

export default Connect;
