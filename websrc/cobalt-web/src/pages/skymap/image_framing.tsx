import React, { useState, useEffect } from "react";
import {
  Card,
  Button,
  Container,
  Row,
  Col,
  InputGroup,
  Form,
} from "react-bootstrap";
import { Search } from "react-bootstrap-icons";
import { useImmer } from "use-immer";

import AladinLiteView from "../../components/skymap/aladin_wrapper";
import * as AXIOSOF from "../../services/object_finding_api";
import FOVSettingDialog from "../../components/skymap/fov_dialog";
import ObjectManagementDialog from "./object_manager_dialog";
import ObjectSearchDialog from "./object_search_dialog";
import { GlobalStore } from "../../store/globalStore";

import { TransparentPaper } from "./style";

const ImageFraming = () => {
  const [target_ra, set_target_ra] = useState(0);
  const [target_dec, set_target_dec] = useState(0);
  const [screen_ra, set_screen_ra] = useState(0);
  const [screen_dec, set_screen_dec] = useState(0);
  const [camera_rotation, set_camera_rotation] = useState(0);
  const [fov_data, update_fov_data] = useImmer({
    x_pixels: 0,
    x_pixel_size: 0,
    y_pixels: 0,
    y_pixel_size: 0,
    focal_length: 0,
  });
  const [show_span, set_show_span] = useState(false);
  const [open_fov_dialog, set_open_fov_dialog] = useState(0);
  const [open_search_dialog, set_open_search_dialog] = useState(0);
  const [open_manage_dialog, set_open_manage_dialog] = useState(0);

  const [fov_points, set_fov_points] = useState([]);
  const [fov_x, set_fov_x] = useState(0.25);
  const [fov_y, set_fov_y] = useState(0.25);
  const [aladin_show_fov, set_aladin_show_fov] = useState(0.5);

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

  const on_new_ra_dec_input = (new_ra, new_dec) => {
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

  const post_for_one_single_fov_rect = async (ra, dec) => {
    let fov_request = {
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
    let to_add_object = {
      name: "-",
      ra: target_ra,
      dec: target_dec,
      rotation: camera_rotation,
      flag: "",
      tag: "",
      target_type: "",
      bmag: 0,
      vmag: 0,
      size: 0,
      checked: false,
    };
    set_focus_target_to_store(to_add_object);
  };

  const start_goto_and_focus_target = () => {
    // TODO, write this code later!
  };

  useEffect(() => {
    update_twilight_data();
    refresh_camera_telescope_data();
  }, []);

  useEffect(() => {
    let fov_x =
      ((57.3 / fov_data.focal_length) *
        fov_data.x_pixels *
        fov_data.x_pixel_size) /
      3600;
    let fov_y =
      ((57.3 / fov_data.focal_length) *
        fov_data.y_pixels *
        fov_data.y_pixel_size) /
      3600;
    set_fov_x(fov_x);
    set_fov_y(fov_y);
  }, [fov_data]);

  return (
    <Card className="main-paper">
      <Card.Title>Image Framing</Card.Title>

      <Card.Body>
        <Row lg={12}>
          <Col md={6}>
            <Container>
              <Row
                sx={{
                  width: 1,
                  height: 1,
                }}
              >
                <AladinLiteView
                  ra={target_ra}
                  dec={target_dec}
                  onCenterChange={on_new_ra_dec_input}
                  fovPoints={fov_points}
                  fovSize={aladin_show_fov}
                />
              </Row>
            </Container>
          </Col>
          <Col md={6}>
            <div className="framing-content">
              <div className="sidebar">
                <div className="center-control">
                  <Card.Title>Target Center</Card.Title>
                  <div className="ra-dec-input">
                    <InputGroup>
                      <InputGroup.Text>RA:</InputGroup.Text>
                      <Form.Control
                        type="number"
                        value={target_ra}
                        step="any"
                        onChange={(e) => set_target_ra(e.target.value)}
                      />
                    </InputGroup>
                    <InputGroup>
                      <InputGroup.Text>Dec:</InputGroup.Text>
                      <Form.Control
                        type="number"
                        value={target_dec}
                        step="any"
                        onChange={(e) => set_target_dec(e.target.value)}
                      />
                    </InputGroup>
                  </div>
                  <Button
                    variant="outline-dark"
                    className="reset-button"
                    onClick={on_click_reset_with_current_center}
                  >
                    Reset with Current Center
                  </Button>
                </div>
                <div className="fov-control">
                  <Card.Title>Field of View (FOV)</Card.Title>
                  <div className="fov-inputs">
                    <InputGroup>
                      <InputGroup.Text>X:</InputGroup.Text>
                      <Form.Control
                        type="number"
                        value={fov_x}
                        step="any"
                        onChange={(e) => set_fov_x(e.target.value)}
                      />
                    </InputGroup>
                    <InputGroup>
                      <InputGroup.Text>Y:</InputGroup.Text>
                      <Form.Control
                        type="number"
                        value={fov_y}
                        step="any"
                        onChange={(e) => set_fov_y(e.target.value)}
                      />
                    </InputGroup>
                  </div>
                  <div className="fov-buttons">
                    <Button
                      variant="outline-dark"
                      onClick={() => set_open_fov_dialog(true)}
                    >
                      Set FOV
                    </Button>
                    <Button
                      variant="outline-dark"
                      onClick={() => set_aladin_show_fov(0)}
                    >
                      Hide FOV
                    </Button>
                  </div>
                  {open_fov_dialog && (
                    <FOVSettingDialog
                      fov_data={fov_data}
                      rotation={camera_rotation}
                      open_dialog={open_fov_dialog}
                      on_fov_change={(new_fov_data) => {
                        update_fov_data((draft) => {
                          console.log(
                            "in framing, got fov change",
                            new_fov_data
                          );
                          draft.focal_length = new_fov_data.focal_length;
                          draft.x_pixels = new_fov_data.x_pixels;
                          draft.x_pixel_size = new_fov_data.x_pixel_size;
                          draft.y_pixels = new_fov_data.y_pixels;
                          draft.y_pixel_size = new_fov_data.y_pixel_size;
                        });
                      }}
                      on_rotation_change={function (rotation: number): void {
                        throw new Error("Function not implemented.");
                      }}
                    />
                  )}
                </div>
                <div className="object-control">
                  <Card.Title>Objects</Card.Title>
                  <Button
                    variant="outline-dark"
                    onClick={() => set_open_search_dialog(true)}
                  >
                    <Search className="icon" />
                    Search Objects
                  </Button>
                  <Button
                    variant="outline-dark"
                    onClick={() => set_open_manage_dialog(true)}
                  >
                    <Search className="icon" />
                    Manage Objects
                  </Button>
                  {open_search_dialog && (
                    <ObjectSearchDialog open_dialog={open_search_dialog} />
                  )}
                  {open_manage_dialog && (
                    <ObjectManagementDialog open_dialog={open_manage_dialog} />
                  )}
                </div>
              </div>
            </div>
          </Col>
        </Row>
      </Card.Body>
    </Card>
  );
};

export default ImageFraming;
