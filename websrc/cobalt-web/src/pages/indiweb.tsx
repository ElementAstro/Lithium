import React, { useState, useEffect } from "react";
import {
  Button,
  Form,
  Modal,
  Alert,
  ToggleButtonGroup,
  ToggleButton,
} from "react-bootstrap";
import axios from "axios";
import $ from "jquery";

function INDIWebComponent() {
  const [selectedProfile, setSelectedProfile] = useState("");
  const [remoteDrivers, setRemoteDrivers] = useState("");
  const [profileInfo, setProfileInfo] = useState({
    port: "",
    autostart: false,
    autoconnect: false,
  });
  const [profiles, setProfiles] = useState([]);
  const [driversList, setDriversList] = useState([]);
  const [remoteDriversValue, setRemoteDriversValue] = useState("");

  const [showModal, setShowModal] = useState(false);
  const [errorMessage, setErrorMessage] = useState("");
  const [notifyMessage, setNotifyMessage] = useState("");

  useEffect(() => {
    $('[data-toggle="tooltip"]').tooltip();
    loadProfiles();
    getStatus();
  }, []);

  const loadProfiles = async () => {
    try {
      const response = await axios.get("/indiweb/api/profiles");
      setProfiles(response.data.profiles);
    } catch (error) {
      setErrorMessage("加载配置文件出错");
      setShowModal(true);
    }
  };

  const loadCurrentProfileDrivers = async () => {
    const name = selectedProfile.trim();
    const url = `/indiweb/api/profiles/${name}/labels`;

    try {
      const response = await axios.get(url);
      const drivers = response.data;
      setDriversList(drivers);

      $("#drivers_list").selectpicker(
        "val",
        drivers.map((driver) => driver.label)
      );
    } catch (error) {
      setErrorMessage("加载设备列表出错");
      setShowModal(true);
    }

    const remoteUrl = encodeURI(`/indiweb/api/profiles/${name}/remote`);
    try {
      const response = await axios.get(remoteUrl);
      const data = response.data;
      if (data && data.drivers !== undefined) {
        setRemoteDriversValue(data.drivers);
      } else {
        setRemoteDriversValue("");
      }
    } catch (error) {
      setErrorMessage("加载远程设备出错");
      setShowModal(true);
    }

    loadProfileData();
  };

  const loadProfileData = async () => {
    const name = selectedProfile.trim();
    const url = encodeURI(`/indiweb/api/profiles/${name}`);

    try {
      const response = await axios.get(url);
      const info = response.data;
      setProfileInfo({
        port: info.port,
        autostart: info.autostart === 1,
        autoconnect: info.autoconnect === 1,
      });
    } catch (error) {
      setErrorMessage("加载配置信息出错");
      setShowModal(true);
    }
  };

  const saveProfile = async () => {
    const name = selectedProfile.trim();
    const url = `/indiweb/api/profiles/${name}`;

    try {
      await axios.post(url);
      saveProfileDrivers(name);
    } catch (error) {
      setErrorMessage("添加配置文件出错");
      setShowModal(true);
    }
  };

  const saveProfileInfo = async () => {
    const name = selectedProfile.trim();
    const { port, autostart, autoconnect } = profileInfo;
    const url = `/indiweb/api/profiles/${name}`;

    const profileData = {
      port,
      autostart: autostart ? 1 : 0,
      autoconnect: autoconnect ? 1 : 0,
    };

    try {
      await axios.put(url, profileData);
    } catch (error) {
      setErrorMessage("更新配置出错");
      setShowModal(true);
    }
  };

  const saveProfileDrivers = async (profile) => {
    const url = `/indiweb/api/profiles/${profile}/drivers`;
    const selectedDrivers = driversList.filter((driver) => driver.selected);
    const remote = remoteDriversValue ? { remote: remoteDriversValue } : null;
    const drivers = [...selectedDrivers, remote];

    try {
      await axios.post(url, drivers);
      setNotifyMessage(`配置 ${profile} 已保存`);
    } catch (error) {
      setErrorMessage("无法添加设备");
      setShowModal(true);
    }
  };

  const addNewProfile = () => {
    const profileName = $("#new_profile_name").val();
    if (profileName) {
      setProfiles((prevProfiles) => [...prevProfiles, profileName]);
      setSelectedProfile(profileName);
      clearDriverSelection();
      setNotifyMessage(`配置 ${profileName} 已创建，请选择设备并保存`);
    }
  };

  const removeProfile = async () => {
    const name = selectedProfile.trim();
    const url = `/indiweb/api/profiles/${name}`;

    if (profiles.length === 1) {
      setNotifyMessage("无法删除默认配置");
      return;
    }

    try {
      await axios.delete(url);
      setProfiles((prevProfiles) =>
        prevProfiles.filter((profile) => profile !== name)
      );
      setSelectedProfile(profiles[0]);
      loadCurrentProfileDrivers();
      setNotifyMessage(`配置 ${name} 已删除`);
    } catch (error) {
      setErrorMessage("无法删除配置");
      setShowModal(true);
    }
  };

  const toggleServer = async () => {
    const status = $.trim($("#server_command").text());

    if (status === "启动") {
      const profile = $("#profiles option:selected").text();
      const url = `/indiweb/api/server/start/${profile}`;

      try {
        await axios.post(url);
        getStatus();
      } catch (error) {
        setErrorMessage("无法启动INDI服务器");
        setShowModal(true);
      }
    } else {
      try {
        await axios.post("/indiweb/api/server/stop");
        getStatus();
      } catch (error) {
        setErrorMessage("无法停止INDI服务器");
        setShowModal(true);
      }
    }
  };

  const getStatus = async () => {
    try {
      const response = await axios.get("/indiweb/api/server/status");
      const status = response.data[0].status;

      if (status === "True") {
        getActiveDrivers();
      } else {
        $("#server_command").html(
          "<i class='fas fa-light-switch-on'></i> 启动"
        );
        $("#server_notify").html(
          "<p className='alert alert-warning'>服务器抽风了！</p>"
        );
      }
    } catch (error) {
      setErrorMessage("获取服务器状态出错");
      setShowModal(true);
    }
  };

  const getActiveDrivers = async () => {
    try {
      const response = await axios.get("/indiweb/api/server/drivers");
      const drivers = response.data;

      if (
        drivers.length < driversList.filter((driver) => driver.selected).length
      ) {
        setNotifyMessage("不是所有设备都在正常工作，请检查电力连接");
      }

      const msg = `
        <p className='alert alert-info'>服务器在线啦！<ul className="list-unstyled">
          ${drivers
            .map(
              (driver) => `
            <li className="border border-primary mb-1 stupid-text">
              <button className="btn btn-xs" onClick={() => restartDriver(driver.label)} data-toggle="tooltip" title="重启设备">
                <i className='fas fa-redo'></i>
              </button>
              ${driver.label}
            </li>
          `
            )
            .join("")}
        </ul></p>
      `;

      $("#server_command").html("<i className='fas fa-close'></i> 停止");
      $("#server_notify").html(msg);
    } catch (error) {
      setErrorMessage("获取设备列表出错");
      setShowModal(true);
    }
  };

  const restartDriver = async (label) => {
    const url = `/indiweb/api/drivers/restart/${label}`;

    try {
      await axios.post(url);
      getStatus();
      setNotifyMessage(`设备 "${label}" 重启成功`);
    } catch (error) {
      setErrorMessage("重启设备失败");
      setShowModal(true);
    }
  };

  const clearDriverSelection = () => {
    $("#drivers_list").selectpicker("val", []);
    $("#drivers_list option").prop("selected", false);
    $("#drivers_list").selectpicker("refresh");
    // Uncheck Auto Start
    $("#profile_auto").prop("checked", false);
    $("#profile_port").val("7624");
  };
  return (
    <div className="container">
      <h1>INDI Web 控制面板</h1>
      <hr />

      {notifyMessage && <Alert variant="success">{notifyMessage}</Alert>}
      {errorMessage && <Alert variant="danger">{errorMessage}</Alert>}

      <h3>配置文件</h3>
      <div className="row">
        <div className="col-md-4">
          <Form.Group controlId="profiles">
            <Form.Label>选择配置文件</Form.Label>
            <Form.Control
              as="select"
              value={selectedProfile}
              onChange={(e) => setSelectedProfile(e.target.value)}
            >
              {profiles.map((profile, index) => (
                <option key={index}>{profile}</option>
              ))}
            </Form.Control>
          </Form.Group>

          <Button
            className="mb-3"
            variant="primary"
            onClick={() => loadCurrentProfileDrivers()}
          >
            加载配置文件
          </Button>

          <Form.Group controlId="new_profile_name">
            <Form.Label>创建新配置文件</Form.Label>
            <Form.Control type="text" placeholder="配置文件名称" />
          </Form.Group>

          <Button variant="success" onClick={() => addNewProfile()}>
            创建配置文件
          </Button>

          <Button
            className="ml-2"
            variant="danger"
            onClick={() => removeProfile()}
          >
            删除配置文件
          </Button>
        </div>

        <div className="col-md-8">
          <h5>配置信息</h5>
          <Form.Group>
            <Form.Label>端口号</Form.Label>
            <Form.Control
              type="text"
              value={profileInfo.port}
              onChange={(e) =>
                setProfileInfo({ ...profileInfo, port: e.target.value })
              }
            />
          </Form.Group>

          <Form.Group>
            <Form.Check
              type="checkbox"
              label="启动时自动运行"
              checked={profileInfo.autostart}
              onChange={(e) =>
                setProfileInfo({ ...profileInfo, autostart: e.target.checked })
              }
            />
          </Form.Group>

          <Form.Group>
            <Form.Check
              type="checkbox"
              label="自动连接设备"
              checked={profileInfo.autoconnect}
              onChange={(e) =>
                setProfileInfo({
                  ...profileInfo,
                  autoconnect: e.target.checked,
                })
              }
            />
          </Form.Group>

          <Button variant="success" onClick={() => saveProfileInfo()}>
            保存配置信息
          </Button>
        </div>
      </div>

      <hr />

      <h3>设备列表</h3>
      <ToggleButtonGroup type="checkbox">
        {driversList.map((driver, index) => (
          <ToggleButton
            key={index}
            type="checkbox"
            variant={driver.selected ? "primary" : "secondary"}
            value={driver.label}
            onChange={() => toggleDriverSelection(driver.label)}
          >
            {driver.label}
          </ToggleButton>
        ))}
      </ToggleButtonGroup>

      <br />
      <br />

      <h5>远程设备</h5>
      <Form.Group controlId="remote_drivers">
        <Form.Control
          type="text"
          placeholder="远程设备URL，以逗号分隔"
          value={remoteDriversValue}
          onChange={(e) => setRemoteDriversValue(e.target.value)}
        />
      </Form.Group>

      <Button
        variant="success"
        onClick={() => saveProfileDrivers(selectedProfile)}
      >
        保存设备列表
      </Button>

      <hr />

      <h3>INDI 服务器</h3>
      <Button
        id="server_command"
        className="mb-3"
        variant="primary"
        onClick={() => toggleServer()}
      >
        启动
      </Button>

      <div id="server_notify"></div>
    </div>
  );
}

export default INDIWebComponent;
