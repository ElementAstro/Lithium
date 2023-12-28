import { useCallback } from "react";
import { useNavigate } from "react-router-dom";
import { Nav,Navbar } from "react-bootstrap";
import { routesConfig } from "../../routes/root";
import { GlobalStore } from "../../store/globalStore";
import "./style.less";
import React from "react";

const Navigation = () => {
  const { drawerVisible } = GlobalStore.useAppState((state) => state.visible);

  const toggleNavigation = () => {
    GlobalStore.actions.visible.setState({
      drawerVisible: false,
    });
  };

  const navigate = useNavigate();

  const push = (key: string) => {
    navigate(key);
    toggleNavigation();
  };

  return (
    
    <Nav defaultActiveKey="/" className="flex-column">
      {routesConfig.map((item) => (
        <Nav.Item key={item.path} onClick={() => push(item.path as string)}>
          <Nav.Link>{item.title}</Nav.Link>
        </Nav.Item>
      ))}
    </Nav>
  );
};

export default Navigation;
