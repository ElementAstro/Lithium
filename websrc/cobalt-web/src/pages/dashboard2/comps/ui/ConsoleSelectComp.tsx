import * as React from "react";
import { Button, Alert } from "react-bootstrap";
import Modal from "react-bootstrap/Modal";
import Card from "react-bootstrap/Card";
import Form from "react-bootstrap/Form";
import { Offcanvas } from "react-bootstrap";
import { GlobalStore } from "../../../../store/globalStore";
import Registry from "../../registry";

const ConsoleSelectComp: React.FC = () => {
  const state = GlobalStore.useAppState((state) => state.console);

  const ICONHandler = Registry._registry_icon.get(state.menu) as React.FC;

  return (
    <>
      <Button
        variant="primary"
        size="sm"
        onClick={() => {
          GlobalStore.actions.console.setState({
            selec_console_drawer_open: true,
          });
        }}
        style={{
          position: "absolute",
          bottom: "8px",
          left: "20px",
          width: "150px",
        }}
      >
        <ICONHandler />
        {Registry.getComponent(state.menu)?.displayName}
      </Button>

      {/* drawer of the functions */}
      <Offcanvas
        size="lg"
        show={state.selec_console_drawer_open}
        onHide={() => {
          GlobalStore.actions.console.setState({
            selec_console_drawer_open: false,
          });
        }}
        placement="start"
        style={{
          backgroundColor: "transparent",
          padding: "24px",
          boxShadow: "none",
        }}
      >
        <Offcanvas.Header closeButton>
          <Offcanvas.Title>中控台</Offcanvas.Title>
        </Offcanvas.Header>
        <Offcanvas.Body
          style={{
            borderRadius: "8px",
            padding: "16px",
            display: "flex",
            flexDirection: "column",
            gap: "16px",
            height: "100%",
            overflow: "auto",
          }}
        >
          <Form>
            <Form.Label style={{ fontSize: "20px", fontWeight: "bold" }}>
              功能选择
            </Form.Label>
            <div
              style={{
                display: "grid",
                gridTemplateColumns: "repeat(auto-fill, minmax(140px, 1fr))",
                gap: "12px",
              }}
            >
              {Registry.getMenus().map((item) => (
                <Card
                  key={item.id}
                  style={{
                    boxShadow: "none",
                    cursor: "pointer",
                  }}
                  className="outline-primary"
                  onClick={() => {
                    GlobalStore.actions.console.setState({
                      menu: item.id as any,
                      drawerVisible: true,
                      selec_console_drawer_open: false,
                    });
                    if (item.id == "phd2") {
                      GlobalStore.actions.ProcessDataSaveStore.switch_camera_display(
                        1
                      );
                    } else {
                      GlobalStore.actions.ProcessDataSaveStore.switch_camera_display(
                        0
                      );
                    }
                  }}
                >
                  <Card.Body>
                    <item.icon />
                    <Card.Title>{item.displayName}</Card.Title>
                  </Card.Body>
                  <Form.Check
                    type="radio"
                    id={item.id}
                    checked={state.menu === item.id}
                    value={item.id}
                    style={{ marginTop: "-8px" }}
                    size={24}
                  />
                </Card>
              ))}
            </div>
          </Form>
        </Offcanvas.Body>
      </Offcanvas>
      {/* end of the drawer */}
    </>
  );
};

export default ConsoleSelectComp;
