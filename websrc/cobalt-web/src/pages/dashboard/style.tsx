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
`;

// styles for the toolbar
export const SToolbar = styled.div`
  position: fixed;
  bottom: 0;
  left: 0;
  width: 100%;
  height: ${(props) => (props.visible ? "50px" : "0px")};
  background-color: #fff;
  z-index: 2;
  transition: all 0.3s ease;
`;