import Container from "react-bootstrap/esm/Container";
import Row from "react-bootstrap/esm/Row";
import styled from "styled-components";

export const RightSidebar = styled.div`
  position: fixed;
  top: 0;
  right: 0;
  height: 100vh;
  width: 80px; // 你可以根据需要调整宽度
  background-color: #333; // 暗色背景
  display: flex;
  flex-direction: column;
  justify-content: space-between;
`;

// 自定义Row，使其在容器中等间距分布
export const StyledRow = styled(Row)`
  display: flex;
  justify-content: center; // 水平居中
  align-items: center; // 垂直居中
  flex: 1; // 使所有Row等高
`;

// 调整Container样式以填满高度
export const FullHeightContainer = styled(Container)`
  height: 100vh; // 使容器高度充满视口
  display: flex;
  flex-direction: column; // 竖直排列子元素
`;