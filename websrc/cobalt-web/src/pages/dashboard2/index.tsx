import * as React from "react";
import Offcanvas from "react-bootstrap/Offcanvas";
import { GlobalStore } from "../../store/globalStore";
import Registry from "./registry";
import Align from "./actions/align";
import Light from "./actions/light";
import Shoot from "./actions/shoot";
import { useEchoWebSocket } from "../../utils/websocketProvider";
import "./style.less";
import Autofocus from "./actions/autofocus";
import InfoUpdateComp from "./comps/ui/InfoUpdateComp";
import ConsoleSelectComp from "./comps/ui/ConsoleSelectComp";
import {
  Camera,
  Magic,
  ArrowsMove,
  Bullseye,
  Map,
  Star,
  PaintBucket,
  Rulers,
} from "react-bootstrap-icons";

import { ReactSVG } from "react-svg";

// devices
import DeviceCameraSimpleControlPanel from "./actions/controls/CameraControl";
import DeviceTelescopeSimpleControlPanel from "./actions/controls/TelescopeControl";
import DeviceFocuserSimpleControlPanel from "./actions/controls/FocuserControl";
import DeviceFilterSimpleControlPanel from "./actions/controls/FilterControl";
import DeviceGuiderSimpleControlPanel from "./actions/controls/GuiderControl";
import DevicePHD2SimpleControlPanel from "./actions/controls/Phd2Control";

Registry.addComponent("shoot", Shoot, Camera);
Registry.addComponent("star", Light, Magic);
Registry.addComponent("align", Align, Map);
Registry.addComponent("autofocus", Autofocus, Bullseye);

Registry.addComponent("camera", DeviceCameraSimpleControlPanel);
Registry.addComponent("telescope", DeviceTelescopeSimpleControlPanel, Star);
Registry.addComponent("focuser", DeviceFocuserSimpleControlPanel);
Registry.addComponent("filter", DeviceFilterSimpleControlPanel, PaintBucket);
Registry.addComponent("phd2", DevicePHD2SimpleControlPanel, Rulers);

const Console = () => {
  // state
  const state = GlobalStore.useAppState((state) => state.console);

  const graph = React.useRef(null);

  const container = React.useRef(null);

  // store
  const process_data = GlobalStore.useAppState(
    (state) => state.ProcessDataSaveStore
  );

  const handleMsg = React.useCallback((msg: any) => {
    // console.log(msg);
  }, []);

  const { removeListener } = useEchoWebSocket(handleMsg);

  React.useEffect(() => {
    return () => removeListener(handleMsg);
  }, []);

  const toggleDrawer = () => {
    GlobalStore.actions.console.setState({
      drawerVisible: false,
    });
  };

  const Handler = Registry._registry.get(state.menu) as React.FC;
  const ICONHandler = Registry._registry_icon.get(state.menu) as React.FC;

  return (
    <>
      {process_data.show_camera == 0 ? (
        <img
          className="newest_image"
          src={process_data.newest_camera_jpg_url}
          style={{ zIndex: 1 }}
        ></img>
      ) : (
        <img
          className="newest_image"
          src={process_data.newest_guider_jpg_url}
          style={{ zIndex: 1 }}
        ></img>
      )}
      <div className="app-console" ref={container}>
        <ConsoleSelectComp />
        <InfoUpdateComp />
        <Offcanvas
          show={state.drawerVisible}
          onHide={toggleDrawer}
          placement="end"
          className="console-drawer"
          backdrop={false}
          scroll={true}
          container={container.current}
          style={{ width: "320px" }}
        >
          <Offcanvas.Body>
            <div className="mx-auto">
              <Handler />
            </div>
          </Offcanvas.Body>
        </Offcanvas>
      </div>
    </>
  );
};

export default Console;
