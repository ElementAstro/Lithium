import styled from "styled-components";

// styles for the toolbar
export const SToolbar = styled.div`
  position: fixed;
  top: 25%;
  left: ${(props) => (props.visible ? "0" : "-50px")};
  width: 75px;
  height: 50%;
  background-color: #fff;
  z-index: 2;
  transition: all 0.3s ease;

  /* 添加更多样式 */
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  padding: 20px;
  box-shadow: 0px 0px 5px rgba(0, 0, 0, 0.2);
  border-radius: 10px;
  @media (max-width: 768px) {
    top: 15%;
  }
  transparent: ${(props) => (props.visible ? "1" : "0")};
`;
