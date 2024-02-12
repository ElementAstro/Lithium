import React from "react";
import { Modal, Button, Form } from "react-bootstrap";
import { XCircleFill, SaveFill } from "react-bootstrap-icons";

const ParameterModal = ({ show, onHide, parameters, onSave }) => {
  // 创建一个本地状态来处理表单输入
  const [localParameters, setLocalParameters] = React.useState(parameters);

  // 处理输入变化
  const handleInputChange = (key, value) => {
    setLocalParameters((prev) => ({ ...prev, [key]: value }));
  };

  // 处理保存操作
  const handleSave = () => {
    onSave(localParameters);
    onHide(); // 关闭模态框
  };

  return (
    <Modal
      show={show}
      onHide={onHide}
      size="lg"
      aria-labelledby="contained-modal-title-vcenter"
      centered
    >
      <Modal.Header closeButton>
        <Modal.Title id="contained-modal-title-vcenter">设置参数</Modal.Title>
      </Modal.Header>
      <Modal.Body>
        {/* 根据参数生成对应的表单元素 */}
        {Object.entries(localParameters).map(([key, value], index) => (
          <Form.Group key={index} controlId={`form-${key}`}>
            <Form.Label>{key}</Form.Label>
            <Form.Control
              type="text"
              defaultValue={value}
              onChange={(e) => handleInputChange(key, e.target.value)}
            />
          </Form.Group>
        ))}
      </Modal.Body>
      <Modal.Footer>
        <Button variant="secondary" onClick={onHide}>
          <XCircleFill /> 取消
        </Button>
        <Button variant="primary" onClick={handleSave}>
          <SaveFill /> 保存
        </Button>
      </Modal.Footer>
    </Modal>
  );
};

export default ParameterModal;
