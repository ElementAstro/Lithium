import React from "react";
import styled from "styled-components";

const StyledSquareButton = styled.button`
  width: 50px; // 定义按钮的宽度
  height: 50px; // 定义按钮的高度
  display: flex;
  justify-content: center;
  align-items: center;
  border: none;
  cursor: pointer;
  background-color: #007bff; // 示例背景色
  color: white; // 文字颜色
  &:hover {
    background-color: #0056b3;
  }
`;

const SquareButton = ({ text, onClick }) => {
  return <StyledSquareButton onClick={onClick}>{text}</StyledSquareButton>;
};

export default SquareButton;
