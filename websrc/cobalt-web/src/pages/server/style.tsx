import styled from "styled-components";
import {
  Container,
  Row,
  Col,
  Form,
  ButtonToolbar,
  Button,
  Modal,
} from "react-bootstrap";

export const StyledContainer = styled(Container)`
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  height: 100vh;
`;

export const StyledForm = styled(Form)`
  width: 400px;
  padding: 20px;
  border-radius: 5px;
  box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
`;

export const StyledButtonToolbar = styled(ButtonToolbar)`
  display: flex;
  justify-content: center;
  margin-top: 20px;
`;

export const StyledErrorMessage = styled.div`
  margin-top: 20px;
  color: red;
  display: flex;
  align-items: center;
`;

export const StyledModalBody = styled(Modal.Body)`
  display: flex;
  align-items: center;
`;

export const StyledModalFooter = styled(Modal.Footer)`
  display: flex;
  justify-content: space-between;
`;
