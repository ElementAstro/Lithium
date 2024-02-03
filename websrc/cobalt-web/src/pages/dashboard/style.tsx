import styled from "styled-components";

// styles for the sidebar
export const SSidebar = styled.div`
  position: absolute;
  top: 0;
  left: ${(props) => (props.visible ? "0" : "-300px")};
  width: 300px;
  height: 100%;
  background-color: #fff;
  z-index: 1;
  transition: all 0.3s ease;

  /* 添加更多样式 */
  display: flex;
  flex-direction: column;
  justify-content: space-between;
  align-items: center;
  padding: 20px;

  /* 添加阴影效果 */
  box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.2);
`;

// styles for the toolbar
export const SToolbar = styled.div`
  position: fixed;
  top: 25%;
  left: ${(props) => (props.visible ? "0" : "-50px")};
  width: 50px;
  height: 50%;
  background-color: #fff;
  z-index: 2;
  transition: all 0.3s ease;

  /* 添加更多样式 */
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  padding: 10px;
  box-shadow: 0px 0px 5px rgba(0, 0, 0, 0.2);
`;

// ... 其他样式
