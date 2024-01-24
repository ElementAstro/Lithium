import { useEffect, useState } from "react";
import {
  Container,
  Row,
  Col,
  ListGroup,
  Button,
  Accordion,
  Form,
  Tab,
  Nav,
  Modal,
  TabContent,
  TabPane,
  Tabs,
} from "react-bootstrap";
import {
  Bullseye as DetailIcon,
  Trash as DeleteIcon,
  Save2 as SaveIcon,
  ArrowClockwise as UpdateIcon,
  Thermometer as TempIcon,
  ArrowRepeat as RepeatIcon,
  Pencil,
  ArrowRight,
} from "react-bootstrap-icons";
import { GlobalStore } from "../../store/globalStore";
import React from "react";

interface ScriptProps {
  file_name: string;
  modified_time: string;
}

function DarkTimeScriptItem() {
  const [scriptName, setScriptName] = useState("");
  const [scriptSettingDetails, setScriptSettingDetails] = useState({
    cool_temperature: 20,
    dark_time: [120, 300],
    repeat: 5,
    warm_camera: true,
    bias: false,
  });

  const [openDialog, setOpenDialog] = useState(false);

  const handleTemperatureChange = (value: string) => {
    setScriptSettingDetails((prev) => ({
      ...prev,
      cool_temperature: Number(value),
    }));
  };

  const handleTimeChange = (index: number, value: string) => {
    const newEntries = [...scriptSettingDetails.dark_time];
    newEntries[index] = parseInt(value, 10) || 0;
    setScriptSettingDetails((prev) => ({
      ...prev,
      dark_time: newEntries,
    }));
  };

  const handleRepeatChange = (value: string) => {
    setScriptSettingDetails((prev) => ({
      ...prev,
      repeat: Number(value),
    }));
  };

  const handleWarmCameraChange = () => {
    setScriptSettingDetails((prev) => ({
      ...prev,
      warm_camera: !prev.warm_camera,
    }));
  };

  const handleBiasChange = () => {
    setScriptSettingDetails((prev) => ({
      ...prev,
      bias: !prev.bias,
    }));
  };

  const addTimeEntry = () => {
    setScriptSettingDetails((prev) => ({
      ...prev,
      dark_time: [...prev.dark_time, 0],
    }));
  };

  const removeTimeEntry = (index: number) => {
    setScriptSettingDetails((prev) => ({
      ...prev,
      dark_time: [
        ...prev.dark_time.slice(0, index),
        ...prev.dark_time.slice(index + 1),
      ],
    }));
  };

  const savePAAScript = () => {
    setOpenDialog(true);
  };

  // 生成一个新的脚本流程
  const handleNewSetting = async () => {
    await GlobalStore.actions.PAA.post_generate_script(scriptSettingDetails);

    // 脚本建立
    await GlobalStore.actions.PAA.post_generate_script(scriptSettingDetails);
    // 脚本命名
    await GlobalStore.actions.PAA.post_save_script(scriptName);
    // 建立成功
    setOpenDialog(false);
    GlobalStore.actions.PAA.get_saved_scripts();
    GlobalStore.actions.PAA.get_saved_scripts();
  };

  return (
    <Container style={{ position: "relative", height: "60vh" }}>
      <Row>
        <Col xs={6}>
          {/* 温度控制 */}
          <Form.Control
            type="number"
            value={scriptSettingDetails.cool_temperature}
            onChange={(e) => handleTemperatureChange(e.target.value)}
            placeholder="冷冻温度(℃)"
          />
          {/* 拍摄张数选择 */}
          <Form.Control
            type="number"
            value={scriptSettingDetails.repeat}
            onChange={(e) => handleRepeatChange(e.target.value)}
            placeholder="拍摄张数"
          />
          {/* 关闭制冷 */}
          <Form.Check
            type="switch"
            id="warm-camera-switch"
            label={scriptSettingDetails.warm_camera ? "ON" : "OFF"}
            checked={scriptSettingDetails.warm_camera}
            onChange={handleWarmCameraChange}
            style={{
              display: "flex",
              alignItems: "center",
              marginBottom: "8px",
            }}
          >
            结束后关闭制冷
          </Form.Check>
          <Form.Check
            type="switch"
            id="bias-switch"
            label={scriptSettingDetails.bias ? "ON" : "OFF"}
            checked={scriptSettingDetails.bias}
            onChange={handleBiasChange}
            style={{ display: "flex", alignItems: "center" }}
          >
            拍摄偏置
          </Form.Check>
        </Col>
        <Col xs={6}>
          <div style={{ padding: "16px" }}>
            <ListGroup title="拍摄时间选择">
              {scriptSettingDetails.dark_time.map((entry, index) => (
                <ListGroup.Item
                  key={index}
                  style={{
                    display: "flex",
                    alignItems: "center",
                    marginBottom: "8px",
                  }}
                >
                  <Form.Control
                    type="number"
                    value={entry}
                    onChange={(e) => handleTimeChange(index, e.target.value)}
                    placeholder={`Time ${index + 1} (s)`}
                    style={{ flex: 1 }}
                  />
                  <Button
                    variant="danger"
                    onClick={() => removeTimeEntry(index)}
                    style={{ marginLeft: "8px" }}
                  >
                    删除
                  </Button>
                </ListGroup.Item>
              ))}
            </ListGroup>
            <Button onClick={addTimeEntry} style={{ marginTop: "8px" }}>
              添加时间选项
            </Button>
          </div>
        </Col>
      </Row>

      {/* 按钮区 */}
      <Row style={{ position: "absolute", bottom: 0 }}>
        <Col xs={3}></Col>
        <Col xs={6}>
          <Button
            variant="outline-primary"
            style={{ marginTop: "16px", width: "100%" }}
            onClick={savePAAScript}
          >
            <SaveIcon />
            保存当前脚本
          </Button>
        </Col>
        <Col xs={3}></Col>
      </Row>

      <Modal show={openDialog} onHide={() => setOpenDialog(false)}>
        <Modal.Header>
          <Modal.Title>脚本命名</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <p>给新脚本取一个名字，方便之后使用</p>
          <Form.Control
            autoFocus
            onChange={(e) => setScriptName(e.target.value)}
            placeholder="名称"
          />
        </Modal.Body>
        <Modal.Footer>
          <Button onClick={handleNewSetting}>提交</Button>
        </Modal.Footer>
      </Modal>
    </Container>
  );
}

export default function SimpleScriptEditor() {
  const { all_script_info } = GlobalStore.useAppState((state) => state.PAA);

  const [openMenu, setOpenMenu] = useState<boolean>(false);
  const [value, setValue] = useState(0);
  const [selectedIndex, setSelectedIndex] = useState(1);

  const [showMenu, setShowMenu] = useState(false);

  const handleOpenMenu = () => {
    setShowMenu(true);
  };

  const handleCloseMenu = () => {
    setShowMenu(false);
  };

  const handleChange = (event: any) => {
    setValue(event.target.value);
  };

  useEffect(() => {
    // 生成所有脚本
    GlobalStore.actions.PAA.get_saved_scripts();
  }, []);

  const toggleDrawer =
    (open: boolean) => (event: React.KeyboardEvent | React.MouseEvent) => {
      if (
        event.type === "keydown" &&
        ((event as React.KeyboardEvent).key === "Tab" ||
          (event as React.KeyboardEvent).key === "Shift")
      ) {
        return;
      }
      setOpenMenu(false);
    };

  // 表单详情
  const infoPage = () => {
    console.log(1);
  };

  const deleteObject = (task: ScriptProps) => {
    const file_name = task.file_name;
    GlobalStore.actions.PAA.post_delete_script(file_name);
    // 表单更新
    GlobalStore.actions.PAA.get_saved_scripts();
    GlobalStore.actions.PAA.get_saved_scripts();
  };

  const handleListItemClick = (
    event: React.MouseEvent<HTMLDivElement, MouseEvent>,
    index: number
  ) => {
    setSelectedIndex(index);
  };

  const updatePAAScript = async (script_index: number) => {
    // 更新当前要执行的部分
    let script_name = all_script_info[script_index].file_name;
    let res = await GlobalStore.actions.PAA.post_load_script(script_name);
    await GlobalStore.actions.PAA.post_update_script({
      data: res.data,
      file_name: script_name,
    });
  };

  return (
    <Container fluid>
      <Row>
        <Col xs={9}>
          <div className="m-4">
            <h4 className="mb-3">当前脚本参数编辑</h4>
            <Tabs
              id="controlled-tab-example"
              onSelect={(k: any) => setValue(k)}
              className="mb-3"
            >
              <Tab eventKey="dark" title="Dark">
                <DarkTimeScriptItem />
              </Tab>
              <Tab eventKey="camera_color" title="Color Camera">
                Item Two
              </Tab>
              <Tab eventKey="camera_mono" title="Mono Camera">
                Item Three
              </Tab>
              <Tab eventKey="auto_flat" title="Auto Flat">
                Item Four
              </Tab>
            </Tabs>
          </div>
        </Col>

        <Col xs={3}>
          <div className="m-4 position-relative">
            <h4 className="mb-3">拍摄脚本选择</h4>
            <ListGroup
              className="mb-3"
              style={{ maxHeight: "60vh", overflowY: "auto" }}
            >
              {all_script_info.map((task, index) => (
                <ListGroup.Item
                  key={index}
                  active={selectedIndex === index}
                  onClick={(event) => handleListItemClick(event, index)}
                >
                  <div className="d-flex justify-content-between">
                    <div>
                      <h5 className="mb-1">
                        {task.file_name.replace(".json", "")}
                      </h5>
                      <small className="text-muted">
                        Modify Time: {task.modified_time}
                      </small>
                    </div>
                    <div>
                      <Pencil className="me-3" onClick={infoPage} />
                      <Trash onClick={(e) => deleteObject(task)} />
                    </div>
                  </div>
                </ListGroup.Item>
              ))}
            </ListGroup>

            {/* 按钮区 */}
            <div className="d-flex justify-content-between position-absolute bottom-0 w-100 mb-3">
              <div className="w-25"></div>
              <div className="w-50">
                <Button
                  variant="primary"
                  className="w-100"
                  onClick={() => {
                    updatePAAScript(selectedIndex);
                  }}
                >
                  <ArrowRight className="me-2" />
                  运行选中脚本
                </Button>
              </div>
              <div className="w-25"></div>
            </div>
          </div>
        </Col>
      </Row>

      <Button onClick={handleOpenMenu}>打开菜单</Button>

      <Modal show={showMenu} onHide={handleCloseMenu} size="sm">
        <Modal.Header closeButton>
          <Modal.Title>脚本模板选择</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Tabs activeKey={value} onSelect={handleChange}>
            <Tab eventKey="dark" title="Dark">
              {/* Dark模板内容 */}
            </Tab>
            <Tab eventKey="camera_color" title="Color Camera">
              {/* Color Camera模板内容 */}
            </Tab>
            <Tab eventKey="camera_mono" title="Mono Camera">
              {/* Mono Camera模板内容 */}
            </Tab>
            <Tab eventKey="auto_flat" title="Auto Flat">
              {/* Auto Flat模板内容 */}
            </Tab>
          </Tabs>
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={handleCloseMenu}>
            关闭
          </Button>
        </Modal.Footer>
      </Modal>
    </Container>
  );
}
