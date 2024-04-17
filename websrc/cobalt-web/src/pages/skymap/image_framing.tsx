import * as React from "react";
import { Alert, Button, Stack } from "react-bootstrap";
import { Card } from "react-bootstrap";
import "./framing.css";
import { GlobalStore } from "../../store/globalStore";
import { Search, Gear } from "react-bootstrap-icons";
import AladinLiteView from "../../components/skymap/aladin";
import { useImmer } from "use-immer";
import * as AXIOSOF from "../..//services/object_finding_api";

import FOVSettingDialog from "../../components/skymap/fov_dialog";
import ObjectManagementDialog from "./object_manager";
import ObjectSearchDialog from "./object_search_dialog";

const ImageFraming: React.FC = () => {
  // const theme = useTheme();

  // comp data
  const [target_ra, set_target_ra] = React.useState(0);
  const [target_dec, set_target_dec] = React.useState(0);
  const [screen_ra, set_screen_ra] = React.useState(0);
  const [screen_dec, set_screen_dec] = React.useState(0);
  const [camera_rotation, set_camera_rotation] = React.useState(0);
  const [fov_data, update_fov_data] = useImmer({
    x_pixels: 0,
    x_pixel_size: 0,
    y_pixels: 0,
    y_pixel_size: 0,
    focal_length: 0,
  });
  const [show_span, set_show_span] = React.useState(false);
  const [open_fov_dialog, set_open_fov_dialog] = React.useState(0);
  const [open_search_dialog, set_open_search_dialog] = React.useState(0);
  const [open_manage_dialog, set_open_manage_dialog] = React.useState(0);

  // other data
  const [fov_points, set_fov_points] = React.useState<
    Array<
      [[number, number], [number, number], [number, number], [number, number]]
    >
  >([]);
  const [fov_x, set_fov_x] = React.useState(0.25);
  const [fov_y, set_fov_y] = React.useState(0.25);
  const [aladin_show_fov, set_aladin_show_fov] = React.useState<number>(0.5);

  // store related
  const target_store = GlobalStore.useAppState(
    (state) => state.TargetListStore
  );
  const global_parameter = GlobalStore.useAppState(
    (state) => state.GlobalParameterStore
  );
  const set_focus_target_to_store =
    GlobalStore.actions.TargetListStore.change_focus_target;
  const update_twilight_data =
    GlobalStore.actions.TargetListStore.fetch_twilight_data;
  // todo actions need to be initialed if necessary

  const on_new_ra_dec_input = (new_ra: number, new_dec: number) => {
    // console.log('got new radec', new_ra, new_dec);
    set_screen_ra(new_ra);
    set_screen_dec(new_dec);
  };
  const refresh_camera_telescope_data = () => {
    let camera_info = global_parameter.global_parameter.camera_info;
    let telescope_info = global_parameter.global_parameter.telescope_info;
    update_fov_data((draft) => {
      if (camera_info?.CCD_MAX_X !== undefined) {
        draft.x_pixels = camera_info?.CCD_MAX_X;
      }
      if (camera_info?.CCD_MAX_Y !== undefined) {
        draft.y_pixels = camera_info?.CCD_MAX_Y;
      }
      if (camera_info?.CCD_PIXEL_SIZE !== undefined) {
        draft.x_pixel_size = camera_info.CCD_PIXEL_SIZE;
        draft.y_pixel_size = camera_info.CCD_PIXEL_SIZE;
      }
      if (telescope_info?.focal_length !== undefined) {
        draft.focal_length = telescope_info.focal_length;
      }
    });
  };
  const on_click_reset_with_current_center = () => {
    set_target_ra(screen_ra);
    set_target_dec(screen_dec);
    calculate_fov_points();
  };
  const post_for_one_single_fov_rect = async (ra: number, dec: number) => {
    let fov_request: IOFRequestFOVpoints = {
      x_pixels: fov_data.x_pixels,
      x_pixel_size: fov_data.x_pixel_size,
      y_pixels: fov_data.y_pixels,
      y_pixel_size: fov_data.y_pixel_size,
      focal_length: fov_data.focal_length,
      camera_rotation: camera_rotation,
      target_ra: ra,
      target_dec: dec,
    };
    try {
      const fov_response = await AXIOSOF.getFovPointsOfRect(fov_request);
      if (fov_response.success) {
        // console.log('successfully got fov points', fov_response.data);
        set_fov_points([fov_response.data]);
      } else {
        console.log(fov_response.message);
        return null;
      }
    } catch (err) {
      return null;
    }
  };
  const calculate_tile_fov_points = () => {};
  const calculate_fov_points = () => {
    set_fov_points([]);
    if (fov_data.focal_length == 0) {
      return;
    }
    if (show_span) {
      // 瓦片叠加的绘图在这里
    } else {
      post_for_one_single_fov_rect(target_ra, target_dec);
    }
  };
  const update_target_center_points = () => {
    if (target_store.current_focus_index != null) {
      target_store.all_saved_targets[target_store.current_focus_index].ra =
        target_ra;
      target_store.all_saved_targets[target_store.current_focus_index].dec =
        target_dec;
      target_store.all_saved_targets[
        target_store.current_focus_index
      ].rotation = camera_rotation;
    }
  };
  const add_current_as_new_target = () => {
    let to_add_object: IDSOFramingObjectInfo = {
      name: "-",
      ra: target_ra,
      dec: target_dec,
      rotation: camera_rotation,
      flag: "",
      tag: "",
      target_type: "",
      size: 0,
      checked: false,
    };
    set_focus_target_to_store(to_add_object);
  };
  const start_goto_and_focus_target = () => {
    // TODO, write this code later!
  };

  // on mount
  React.useEffect(() => {
    // read x pixel and y pixel data from global parameter
    // console.log('image framing got ', props.tab_value);
    update_twilight_data();
    refresh_camera_telescope_data();
  }, []);
  // camera and telescope info update to string
  React.useEffect(() => {
    let fov_x =
      ((57.3 / fov_data.focal_length) *
        fov_data.x_pixels *
        fov_data.x_pixel_size) /
      1000;
    let fov_y =
      ((57.3 / fov_data.focal_length) *
        fov_data.y_pixels *
        fov_data.y_pixel_size) /
      1000;
    set_fov_x(fov_x);
    set_fov_y(fov_y);
    if (fov_x > fov_y) {
      if (2 * fov_x < 4) set_aladin_show_fov(4);
      else set_aladin_show_fov(2 * fov_x);
    } else {
      if (2 * fov_y < 4) set_aladin_show_fov(4);
      else set_aladin_show_fov(2 * fov_y);
    }
    calculate_fov_points();
  }, [fov_data]);
  // on target ra dec change
  React.useEffect(() => {
    calculate_fov_points();
  }, [target_ra, target_dec, camera_rotation]);

  React.useEffect(() => {
    // console.log('need focus change', target_store.need_focus);
    if (target_store.need_focus) {
      set_target_ra(target_store.current_focus_target.ra);
      set_target_dec(target_store.current_focus_target.dec);
      set_screen_ra(target_store.current_focus_target.ra);
      set_screen_dec(target_store.current_focus_target.dec);
    }
    target_store.need_focus = false;
  }, [target_store.need_focus]);

  return (
    <div className="framing-root">
      <div
        style={{
          width: "100%",
          height: `calc(100vh - 2px)`,
          position: "fixed",
          left: 0,
          top: 0,
        }}
      >
        <AladinLiteView
          ra={target_ra}
          dec={target_dec}
          onCenterChange={on_new_ra_dec_input}
          fov_points={fov_points}
          fov_size={aladin_show_fov}
        />
      </div>
      <div
        style={{
          zIndex: 4,
          position: "fixed",
          left: 2,
          top: "50vh",
          width: 200,
          transform: "translateY(-50%)",
          backgroundColor: "transparent",
        }}
        className="left_display"
      >
        <Card style={{ backgroundColor: "transparent" }}>
          <Card.Body>
            <Card.Subtitle
              style={{
                color: "white",
                textShadow: "2px 2px 4px rgba(0, 0, 0, 0.5)",
              }}
            >
              当前目标: {target_store.current_focus_target.name}
            </Card.Subtitle>
            <Card.Text
              style={{
                color: "white",
                textShadow: "2px 2px 4px rgba(0, 0, 0, 0.5)",
              }}
            >
              Ra: {target_ra.toFixed(7)}
            </Card.Text>
            <Card.Text
              style={{
                color: "white",
                textShadow: "2px 2px 4px rgba(0, 0, 0, 0.5)",
              }}
            >
              Dec: {target_dec.toFixed(7)}
            </Card.Text>
          </Card.Body>
          <Card.Footer>
            <Button
              variant="primary"
              onClick={() => {
                set_open_fov_dialog(open_fov_dialog + 1);
              }}
            >
              修改视场参数
            </Button>
          </Card.Footer>
        </Card>
      </div>
      <div
        style={{
          zIndex: 4,
          position: "fixed",
          right: 2,
          top: "50vh",
          transform: "translateY(-50%)",
          backgroundColor: "transparent",
        }}
      >
        <Stack
          direction="vertical"
          gap={1}
          style={{ margin: "1rem", backgroundColor: "transparent" }}
        >
          <Button
            variant="primary"
            size="sm"
            onClick={() => {
              set_open_search_dialog(open_search_dialog + 1);
            }}
          >
            <Search /> 深空目标搜索
          </Button>
          <Button
            variant="primary"
            size="sm"
            onClick={() => {
              set_open_manage_dialog(open_manage_dialog + 1);
            }}
          >
            <Gear /> 待拍目标选择
          </Button>
          <Button
            variant="secondary"
            size="sm"
            onClick={on_click_reset_with_current_center}
          >
            更新视场中心
          </Button>
          <Button
            variant="secondary"
            size="sm"
            disabled={target_store.current_focus_index != null}
            onClick={update_target_center_points}
          >
            更新目标坐标
          </Button>
          <Button
            variant="secondary"
            size="sm"
            onClick={add_current_as_new_target}
          >
            新建目标
          </Button>
          <br></br>
          <Button
            variant="success"
            size="sm"
            onClick={() => {
              start_goto_and_focus_target();
            }}
          >
            移动赤道仪并居中
          </Button>
        </Stack>
      </div>

      <Alert
        variant="success"
        style={{
          width: "61vw",
          position: "fixed",
          bottom: 0,
          right: 100,
          zIndex: 90,
          borderRadius: "8px",
        }}
      >
        视场中心点坐标: Ra: {screen_ra.toFixed(7)};Dec: {screen_dec.toFixed(7)}
      </Alert>
      {/* 修正左上角aladin关不掉的中心点坐标 */}
      <div
        style={{
          zIndex: 4,
          position: "fixed",
          left: 1,
          top: 1,
          width: 220,
          height: 25,
          backgroundColor: "black",
        }}
      ></div>
      <FOVSettingDialog
        fov_data={fov_data}
        rotation={camera_rotation}
        open_dialog={open_fov_dialog}
        on_fov_change={(new_fov_data) => {
          update_fov_data((draft) => {
            console.log("in framing, got fov change", new_fov_data);
            draft.focal_length = new_fov_data.focal_length;
            draft.x_pixels = new_fov_data.x_pixels;
            draft.x_pixel_size = new_fov_data.x_pixel_size;
            draft.y_pixels = new_fov_data.y_pixels;
            draft.y_pixel_size = new_fov_data.y_pixel_size;
          });
        }}
        on_rotation_change={(new_rotation) => {
          set_camera_rotation(new_rotation);
        }}
      />
      <ObjectSearchDialog open_dialog={open_search_dialog} />
      <ObjectManagementDialog open_dialog={open_manage_dialog} />
    </div>
  );
};

export default ImageFraming;
