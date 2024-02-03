import React, { useState } from "react";
import styled from "styled-components";
import { Button, ButtonGroup } from "react-bootstrap";
import {
  PlusSquareFill,
  GearFill,
  PencilSquare,
  TrashFill,
} from "react-bootstrap-icons";

const FloatingUIContainer = styled.div`
  position: fixed;
  bottom: 20px;
  right: 20px;
  display: flex;
  flex-direction: column;
  align-items: flex-end;
`;

const FloatingButton = styled(Button)`
  margin-top: 10px;
  background-color: #1e90ff;
  color: #fff;
  border: none;
  box-shadow: 0px 2px 5px rgba(0, 0, 0, 0.2);

  &:hover {
    background-color: #187bcd;
  }
`;

const FloatingUI = () => {
  const [isOpen, setIsOpen] = useState(false);

  const handleActionClick = (name) => {
    // 处理按钮点击事件
    console.log("Clicked", name);
  };

  const toggleMenu = () => {
    setIsOpen(!isOpen);
  };

  return (
    <FloatingUIContainer>
      <Button variant="primary" onClick={toggleMenu}>
        {isOpen ? "Close" : "Open"}
      </Button>
      {isOpen && (
        <ButtonGroup vertical>
          <FloatingButton onClick={() => handleActionClick("main")}>
            <PlusSquareFill size={20} />
            <span style={{ marginLeft: "10px" }}>Add</span>
          </FloatingButton>
          <FloatingButton onClick={() => handleActionClick("settings")}>
            <GearFill size={20} />
            <span style={{ marginLeft: "10px" }}>Settings</span>
          </FloatingButton>
          <FloatingButton onClick={() => handleActionClick("edit")}>
            <PencilSquare size={20} />
            <span style={{ marginLeft: "10px" }}>Edit</span>
          </FloatingButton>
          <FloatingButton onClick={() => handleActionClick("delete")}>
            <TrashFill size={20} />
            <span style={{ marginLeft: "10px" }}>Delete</span>
          </FloatingButton>
        </ButtonGroup>
      )}
    </FloatingUIContainer>
  );
};

export default FloatingUI;
