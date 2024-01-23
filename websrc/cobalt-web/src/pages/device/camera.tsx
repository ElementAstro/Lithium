import React, { useState, useEffect } from "react";
import {
  Container,
  Row,
  Col,
  Card,
  Button,
  Form,
  InputGroup,
} from "react-bootstrap";
import {
  ArrowDown,
  ArrowLeft,
  Grid3x3,
  Hourglass,
  Snow,
} from "react-bootstrap-icons";
import { useImmer } from "use-immer";
import { DragDropContext, Draggable, Droppable } from "react-beautiful-dnd";

import HelperSnackbar from "./description/helper_snackbar";
import { useEchoWebSocket } from "../../utils/websocketProvider";
import DeviceCustomSettingComp from "./custom/settings";

const DeviceCameraGeneralControlPanel: React.FC = () => {
  // static status
  const [device_name, set_device_name] = React.useState("XXXX");
  const [camera_info, set_camera_info] = React.useState({
    x_pixels: 0,
    y_pixels: 0,
    x_pixel_size: 0,
    y_pixel_size: 0,
  });
  const [exposure_status, set_exposure_status] = React.useState(false);
  const [cool_status, set_cool_status] = React.useState(false);
  // control status
  const [control_cool_down, set_control_cool_down] = React.useState(false);
  const [camera_temperature, set_camera_temperature] = React.useState(0);
  // const [target_temperature, set_target_temperature] = React.useState(0);
  const [binning, set_binning] = React.useState(1);
  const [gain, set_gain] = React.useState(100);
  const [offset, set_offset] = React.useState(0);
  const [extra_setting, set_extra_setting] = React.useState<Array<any>>([]);
  const [setting_show, update_setting_show] = useImmer({
    has_cool: true,
    // has_fan: true,
    // has_heater: true,
    has_binning: true,
    // has_high_fullwell: false,
    // has_low_noise: false,
    // has_conversion_gain: false,
  });
  // input data
  const [input_target_temperature, set_input_target_temperature] =
    React.useState<string>("-10");
  const [input_binning, set_input_binning] = React.useState<string>("1");
  const [input_gain, set_input_gain] = React.useState<string>("");
  const [input_offset, set_input_offset] = React.useState<string>("");

  const process_ws_message = (msg: any): void => {
    // console.log('from camera device', msg);
    if (msg.type == "error") {
      return;
    }
    if (msg.device_name == "camera") {
      switch (msg.instruction) {
        case "get_ccd_name": {
          set_device_name(msg.data.device_name);
          break;
        }
        case "get_static_info": {
          let new_camera_info = {
            x_pixels: 0,
            y_pixels: 0,
            x_pixel_size: 0,
            y_pixel_size: 0,
          };
          for (let i = 0; i < msg.data.data.length; i++) {
            if (msg.data.data[i].name == "CCD_MAX_X") {
              new_camera_info.x_pixels = msg.data.data[i].value;
            } else if (msg.data.data[i].name == "CCD_MAX_Y") {
              new_camera_info.y_pixels = msg.data.data[i].value;
            } else if (msg.data.data[i].name == "CCD_PIXEL_SIZE_X") {
              new_camera_info.x_pixel_size = msg.data.data[i].value;
            } else if (msg.data.data[i].name == "CCD_PIXEL_SIZE_Y") {
              new_camera_info.y_pixel_size = msg.data.data[i].value;
            }
          }
          set_camera_info(new_camera_info);
          break;
        }
        case "set_cool_target_temperature": {
          break;
        }
        case "get_set_params": {
          set_gain(msg.data.gain);
          set_input_gain(String(msg.data.gain));
          set_offset(msg.data.offset);
          set_input_offset(String(msg.data.offset));
          // binning
          if (msg.data.binning !== null) {
            set_binning(msg.data.binning);
            update_setting_show((draft) => {
              draft.has_binning = true;
            });
          } else {
            update_setting_show((draft) => {
              draft.has_binning = false;
            });
          }
          // cooler
          if (msg.data.cooler !== null) {
            set_cool_status(msg.data.cooler);
            update_setting_show((draft) => {
              draft.has_cool = true;
            });
          } else {
            update_setting_show((draft) => {
              draft.has_cool = false;
            });
          }
          // // fan
          // if (msg.data.fan !== null){
          //   set_extra_setting(draft => {
          //     draft.fan_control = msg.data.fan;
          //   })
          //   update_setting_show(draft => {
          //     draft.has_fan = true;
          //   })
          // }else{
          //   update_setting_show(draft => {
          //     draft.has_fan = false;
          //   })
          // }
          // // heater
          // if (msg.data.heater !== null){
          //   set_extra_setting(draft => {
          //     draft.glass_heat = msg.data.heater;
          //   })
          //   update_setting_show(draft => {
          //     draft.has_heater = true;
          //   })
          // }else{
          //   update_setting_show(draft => {
          //     draft.has_heater = false;
          //   })
          // }
          // hcg
          // if (msg.data.hcg !== null){
          //   set_extra_setting(draft => {
          //     draft.high_fullwell = msg.data.hcg;
          //   })
          //   update_setting_show(draft => {
          //     draft.has_high_fullwell = true;
          //   })
          // }else{
          //   update_setting_show(draft => {
          //     draft.has_high_fullwell = false;
          //   })
          // }
          // // low_noise_mode
          // if (msg.data.low_noise_mode !== null){
          //   set_extra_setting(draft => {
          //     draft.low_noise = msg.data.low_noise_mode;
          //   })
          //   update_setting_show(draft => {
          //     draft.has_low_noise = true;
          //   })
          // }else{
          //   update_setting_show(draft => {
          //     draft.has_low_noise = false;
          //   })
          // }
          break;
        }
        case "get_real_time_info": {
          set_camera_temperature(msg.data.temperature);
          set_exposure_status(msg.data.in_exposure);
          set_cool_status(msg.data.in_cooling);
          break;
        }
        case "start_cool_camera": {
          break;
        }
        case "stop_cool_camera": {
          break;
        }
        case "start_fan": {
          break;
        }
        case "stop_fan": {
          break;
        }
        case "start_tc_heat": {
          break;
        }
        case "stop_tc_heat": {
          break;
        }
        case "start_single_exposure": {
          break;
        }
        case "abort_exposure": {
          break;
        }
        case "set_number_parameters": {
          // call helper
          // resend get set params
          sendMessage(
            JSON.stringify({
              device_name: "camera",
              instruction: "get_set_params",
              params: [],
            })
          );
          break;
        }
        case "get_custom_settings": {
          set_extra_setting(msg.data);
          break;
        }
        case "set_custom_setting": {
          sendMessage(
            JSON.stringify({
              device_name: "camera",
              instruction: "get_custom_settings",
              params: [],
            })
          );
          break;
        }
        default: {
        }
      }
    }
  };
  const { sendMessage, removeListener } = useEchoWebSocket(process_ws_message);

  const MINUTE_MS = 1000;
  React.useEffect(() => {
    // This will fire only on mount.
    // first get static info
    sendMessage(
      JSON.stringify({
        device_name: "camera",
        instruction: "get_ccd_name",
        params: [],
      })
    );
    sendMessage(
      JSON.stringify({
        device_name: "camera",
        instruction: "get_static_info",
        params: [],
      })
    );
    sendMessage(
      JSON.stringify({
        device_name: "camera",
        instruction: "get_set_params",
        params: [],
      })
    );
    sendMessage(
      JSON.stringify({
        device_name: "camera",
        instruction: "get_custom_settings",
        params: [],
      })
    );
    const interval = setInterval(() => {
      // console.log('Logs every second');
      set_control_cool_down(false);
      sendMessage(
        JSON.stringify({
          device_name: "camera",
          instruction: "get_real_time_info",
          params: [],
        })
      );
    }, MINUTE_MS);

    return () => {
      // console.log('clear interval');
      clearInterval(interval);
      removeListener(process_ws_message);
    }; // This represents the unmount function, in which you need to clear your interval to prevent memory leaks.
  }, []);

  return (
    <Container>
      <Row>
        <Col xs={12} md={6}>
          <Card>
            <Card.Title className="m-3">主相机信息</Card.Title>
            <Card.Body>
              <h6>设备型号: {device_name}</h6>
              <h6>相机物理参数:</h6>
              <p>x像素数量: {camera_info.x_pixels}</p>
              <p>y像素数量: {camera_info.y_pixels}</p>
              <p>x像元大小: {camera_info.x_pixel_size.toFixed(2)} um</p>
              <p>y像元大小: {camera_info.y_pixel_size.toFixed(2)} um</p>
              <div className="mt-4">
                {exposure_status ? (
                  <Button variant="outline-danger">
                    <Hourglass />
                    正在曝光
                  </Button>
                ) : (
                  <Button variant="outline-primary">待机</Button>
                )}
              </div>
              <div className="mt-2">
                {cool_status ? (
                  <Button variant="outline-danger">
                    <Hourglass />
                    制冷中
                  </Button>
                ) : (
                  <Button variant="outline-primary">未开启制冷</Button>
                )}
              </div>
            </Card.Body>
          </Card>
        </Col>
        <Col xs={12} md={6}>
          <Card>
            <Card.Title className="m-3">设备控制</Card.Title>
            <Card.Body>
              <Form.Group>
                <Form.Label>开关冷冻</Form.Label>
                <Form.Check
                  type="switch"
                  id="cool-switch"
                  checked={cool_status}
                  onChange={(event) => {
                    if (!control_cool_down) {
                      set_cool_status(event.target.checked);
                      if (event.target.checked) {
                        sendMessage(
                          JSON.stringify({
                            device_name: "camera",
                            instruction: "start_cool_camera",
                            params: [],
                          })
                        );
                      } else {
                        sendMessage(
                          JSON.stringify({
                            device_name: "camera",
                            instruction: "stop_cool_camera",
                            params: [],
                          })
                        );
                      }
                    } else {
                      // todo call helper
                    }
                  }}
                />
              </Form.Group>
              <div className="mt-3">
                <span>相机温度:</span>
                {camera_temperature < 0 ? (
                  <span className="border border-success rounded px-1">
                    {camera_temperature} ℃
                  </span>
                ) : (
                  <span className="border border-danger rounded px-1">
                    {camera_temperature} ℃
                  </span>
                )}
              </div>
              <div className="mt-3">
                <InputGroup>
                  <InputGroup.Text>
                    <Snow />
                  </InputGroup.Text>
                  <Form.Control
                    type="number"
                    placeholder="修改冷冻目标温度 ℃"
                    value={input_target_temperature}
                    onChange={(event) =>
                      set_input_target_temperature(event.target.value)
                    }
                  />
                  <Button
                    variant="primary"
                    onClick={() => {
                      if (control_cool_down) {
                        // call helper
                      } else {
                        if (input_target_temperature !== null) {
                          sendMessage(
                            JSON.stringify({
                              device_name: "camera",
                              instruction: "set_cool_target_temperature",
                              params: [parseInt(input_target_temperature)],
                            })
                          );
                        } else {
                          // call helper
                        }
                      }
                    }}
                  >
                    执行
                  </Button>
                </InputGroup>
              </div>
              <div className="mt-3">
                <span>当前增益gain</span>
                <span className="border border-success rounded px-1">
                  {gain}
                </span>
              </div>
              <div className="mt-3">
                <InputGroup>
                  <InputGroup.Text>
                    <ArrowDown />
                  </InputGroup.Text>
                  <Form.Control
                    type="number"
                    placeholder="修改增益"
                    value={input_gain}
                    onChange={(event) => set_input_gain(event.target.value)}
                  />
                  <Button
                    variant="primary"
                    onClick={() => {
                      if (control_cool_down) {
                        // call helper
                      } else {
                        if (input_gain !== null) {
                          sendMessage(
                            JSON.stringify({
                              device_name: "camera",
                              instruction: "set_number_parameters",
                              params: ["gain", parseInt(input_gain)],
                            })
                          );
                        } else {
                          // call helper
                        }
                      }
                    }}
                  >
                    执行
                  </Button>
                </InputGroup>
              </div>
              <div className="mt-3">
                <span>当前偏置值offset</span>
                <span className="border border-success rounded px-1">
                  {offset}
                </span>
              </div>
              <div className="mt-3">
                <InputGroup>
                  <InputGroup.Text>
                    <ArrowLeft />
                  </InputGroup.Text>

                  <Form.Control
                    type="number"
                    placeholder="修改偏置值"
                    value={input_offset}
                    onChange={(event) => set_input_offset(event.target.value)}
                  />

                  <Button
                    variant="primary"
                    onClick={() => {
                      if (control_cool_down) {
                        // call helper
                      } else {
                        if (input_offset !== null) {
                          sendMessage(
                            JSON.stringify({
                              device_name: "camera",
                              instruction: "set_number_parameters",
                              params: ["offset", parseInt(input_offset)],
                            })
                          );
                        } else {
                        }
                      }
                    }}
                  >
                    执行
                  </Button>
                </InputGroup>
              </div>
              {setting_show.has_binning && (
                <>
                  <div className="mt-3">
                    <span>当前binning {binning}</span>
                  </div>
                  <div className="mt-3">
                    <InputGroup>
                      <InputGroup.Text>
                        <Grid3x3 />
                      </InputGroup.Text>

                      <Form.Control
                        type="number"
                        placeholder="修改binning值"
                        value={input_binning}
                        onChange={(event) => {
                          let real_binning = parseInt(event.target.value);
                          if (real_binning > 4) {
                            real_binning = 4;
                          }
                          if (real_binning == 3) {
                            real_binning = 4;
                          }
                          if (real_binning < 1) {
                            real_binning = 1;
                          }
                          set_input_binning(String(real_binning));
                        }}
                      />

                      <Button
                        variant="primary"
                        onClick={() => {
                          if (control_cool_down) {
                            // call helper
                          } else {
                            if (input_binning !== null) {
                              sendMessage(
                                JSON.stringify({
                                  device_name: "camera",
                                  instruction: "set_number_parameters",
                                  params: ["binning", parseInt(input_binning)],
                                })
                              );
                            } else {
                            }
                          }
                        }}
                      >
                        执行
                      </Button>
                    </InputGroup>
                  </div>
                </>
              )}
              {/*
                {setting_show.has_heater && (
                <div className="mt-3">
                  <Form.Check
                    type="switch"
                    id="glass-heat-switch"
                    label="相机玻璃加热"
                    checked={extra_setting.glass_heat}
                    onChange={(event) => {
                      set_extra_setting((draft) => {
                        draft.glass_heat = event.target.checked;
                      });
                    }}
                  />
                </div>
              )}
              {setting_show.has_fan && (
                <div className="mt-3">
                  <Form.Check
                    type="switch"
                    id="fan-control-switch"
                    label="散热风扇控制"
                    checked={extra_setting.fan_control}
                    onChange={(event) => {
                      set_extra_setting((draft) => {
                        draft.fan_control = event.target.checked;
                      });
                    }}
                  />
                </div>
              )}
                */}
              <DeviceCustomSettingComp
                setting_values={extra_setting}
                on_value_change={function (
                  custom_name: string,
                  value: string | boolean
                ): void {
                  sendMessage(
                    JSON.stringify({
                      device_name: "camera",
                      instruction: "set_custom_setting",
                      params: [custom_name, value],
                    })
                  );
                }}
              />
            </Card.Body>
          </Card>
        </Col>
      </Row>
    </Container>
  );
};

export default DeviceCameraGeneralControlPanel;
