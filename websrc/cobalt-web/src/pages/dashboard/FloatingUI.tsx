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
            <PlusSquareFill />
          </FloatingButton>
          <FloatingButton onClick={() => handleActionClick("settings")}>
            <GearFill />
          </FloatingButton>
          <FloatingButton onClick={() => handleActionClick("edit")}>
            <PencilSquare />
          </FloatingButton>
          <FloatingButton onClick={() => handleActionClick("delete")}>
            <TrashFill />
          </FloatingButton>
        </ButtonGroup>
      )}
    </FloatingUIContainer>
  );
};

export default FloatingUI;
