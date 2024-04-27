import React, { useState, useEffect } from "react";
import { Modal, Button, Form, Col, Row } from "react-bootstrap";
import { GlobalStore } from "../../../../store/globalStore";

const AutoFocusSettingDialog = (props) => {
  // ui control
  const [open, setOpen] = useState(false);

  // input data
  const [startSide, setStartSide] = useState(false);

  // filter selections
  const initialFilterNames =
    GlobalStore.useAppState(
      (state) => state.GlobalParameterStore.get_filter_names_full
    ) || [];
  const [allFilters, setAllFilters] = useState(initialFilterNames);
  const [selectedFilter, setSelectedFilter] = useState(1);

  const { brand_connection } = GlobalStore.useAppState(
    (state) => state.connect
  );

  // useEffect
  // on mount
  useEffect(() => {
    setOpen(props.open_dialog > 0);
    setStartSide(props.start_side);
  }, []);

  // open dialog
  useEffect(() => {
    setOpen(props.open_dialog > 0);
  }, [props.open_dialog]);

  // functions
  const handleClose = () => {
    props.on_update_data(selectedFilter, startSide);
    setOpen(false);
  };

  return (
    <Modal show={open} onHide={handleClose}>
      <Modal.Header closeButton>
        <Modal.Title>自动对焦初始化设置</Modal.Title>
      </Modal.Header>
      <Modal.Body>
        <p>单独执行自动对焦流程的初始化配置。</p>
        <p>
          一部分参数在全局变量的自动对焦参数里进行修改！全局变量里的自动对焦参数在所有运行的自动对焦流程都有效，包括PAA的流程。
        </p>
        <p>
          <strong>
            开始自动对焦前，请务必保证
            <span className="text-danger">望远镜已经在焦点附近！</span>
            否则将无法进行自动对焦。请在拍摄界面确认已经可以看到比较细的星点的情况下再进行本流程！
          </strong>
        </p>
        <Form>
          <Form.Group as={Row} controlId="start-side">
            <Form.Label column sm="6">
              {startSide ? "初始向外移动" : "初始向内移动"}
            </Form.Label>
            <Col sm="6">
              <Form.Check
                type="switch"
                checked={startSide}
                onChange={() => setStartSide(!startSide)}
              />
            </Col>
          </Form.Group>

          {/* 滤镜选择 */}
          {brand_connection["filter"] == 2 ? (
            <Form.Group controlId="filter-selector">
              <Form.Label>滤镜选择</Form.Label>
              <Form.Control
                as="select"
                value={selectedFilter}
                onChange={(e) => setSelectedFilter(e.target.value)}
              >
                {allFilters.map((filter, index) => (
                  <option key={index} value={filter}>
                    {filter}
                  </option>
                ))}
              </Form.Control>
            </Form.Group>
          ) : (
            <p className="text-danger">滤镜轮未连接</p>
          )}
        </Form>
      </Modal.Body>
      <Modal.Footer>
        <Button variant="secondary" onClick={handleClose}>
          关闭
        </Button>
      </Modal.Footer>
    </Modal>
  );
};

export default AutoFocusSettingDialog;
