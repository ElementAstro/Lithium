import React, { useState, useEffect } from 'react';
import { Typography, Paper, Button, FormControl, InputGroup } from 'react-bootstrap';
import { AcUnit, KeyboardDoubleArrowUp, KeyboardDoubleArrowRight, Grid4x4, HourglassEmpty, Done } from 'react-bootstrap-icons';
import HelperSnackbar from './description/helper_snackbar';
import { useEchoWebSocket } from '@/utils/websocketProvider';

const DeviceGuiderGeneralControlPanel = () => {
  // static status
  const [device_name, set_device_name] = useState('XXXX');
  const [camera_info, set_camera_info] = useState({
    x_pixels: 0,
    y_pixels: 0,
    x_pixel_size: 0,
    y_pixel_size: 0,
  });
  const [exposure_status, set_exposure_status] = useState(false);
  // control status
  const [control_cool_down, set_control_cool_down] = useState(false);
  // const [target_temperature, set_target_temperature] = useState(0);
  const [binning, set_binning] = useState(1);
  const [gain, set_gain] = useState(100);
  const [offset, set_offset] = useState(0);
  const [setting_show, set_setting_show] = useState({
    has_cool: true,
    has_fan: true,
    has_heater: true,
    has_binning: true,
    has_high_fullwell: false,
    has_low_noise: false,
    has_conversion_gain: false,
  });
  // input data
  const [input_binning, set_input_binning] = useState('');
  const [input_gain, set_input_gain] = useState('');
  const [input_offset, set_input_offset] = useState('');

  const process_ws_message = (msg) => {
    if (msg.type == 'error') {
      return;
    }
    if (msg.device_name == 'guider') {
      switch (msg.instruction) {
        case 'get_ccd_name': {
          set_device_name(msg.data.device_name);
          break;
        }
        case 'get_static_info': {
          let new_camera_info = {
            x_pixels: 0,
            y_pixels: 0,
            x_pixel_size: 0,
            y_pixel_size: 0,
          };
          for (let i = 0; i < msg.data.data.length; i++) {
            if (msg.data.data[i].name == 'CCD_MAX_X') {
              new_camera_info.x_pixels = msg.data.data[i].value;
            } else if (msg.data.data[i].name == 'CCD_MAX_Y') {
              new_camera_info.y_pixels = msg.data.data[i].value;
            } else if (msg.data.data[i].name == 'CCD_PIXEL_SIZE_X') {
              new_camera_info.x_pixel_size = msg.data.data[i].value;
            } else if (msg.data.data[i].name == 'CCD_PIXEL_SIZE_Y') {
              new_camera_info.y_pixel_size = msg.data.data[i].value;
            }
          }
          set_camera_info(new_camera_info);
          break;
        }
        case 'set_cool_target_temperature': {
          break;
        }
        case 'get_set_params': {
          set_gain(msg.data.gain);
          set_offset(msg.data.offset);
          // binning
          if (msg.data.binning !== null) {
            set_binning(msg.data.binning);
            set_setting_show((prevState) => ({
              ...prevState,
              has_binning: true,
            }));
          } else {
            set_setting_show((prevState) => ({
              ...prevState,
              has_binning: false,
            }));
          }
          break;
        }
        case 'get_real_time_info': {
          set_exposure_status(msg.data.in_exposure);
          break;
        }
        case 'start_cool_camera': {

          break;
        }
        case 'stop_cool_camera': {

          break;
        }
        case 'start_fan': {

          break;
        }
        case 'stop_fan': {

          break;
        }
        case 'start_tc_heat': {

          break;
        }
        case 'stop_tc_heat': {

          break;
        }
        case 'start_single_exposure': {

          break;
        }
        case 'abort_exposure': {

          break;
        }
        case 'set_number_parameters': {
          // call helper
          // resend get set params
          sendMessage(JSON.stringify({
            device_name: 'guider',
            instruction: 'get_set_params',
            params: [],
          }));
          break;
        }
        default: {

        }
      }
    }
  };

  const { sendMessage, removeListener } = useEchoWebSocket(process_ws_message);

  const MINUTE_MS = 1000;
  useEffect(() => {
    // This will fire only on mount.
    // first get static info
    sendMessage(JSON.stringify({
      device_name: 'guider',
      instruction: 'get_ccd_name',
      params: [],
    }));
    sendMessage(JSON.stringify({
      device_name: 'guider',
      instruction: 'get_static_info',
      params: [],
    }));
    sendMessage(JSON.stringify({
      device_name: 'guider',
      instruction: 'get_set_params',
      params: [],
    }));
    const interval = setInterval(() => {
      set_control_cool_down(false);
      sendMessage(JSON.stringify({
        device_name: 'guider',
        instruction: 'get_real_time_info',
        params: [],
      }));
    }, MINUTE_MS);

    return () => {
      clearInterval(interval);
      removeListener(process_ws_message);
    };
  }, []);

  return (
    <div className="container mt-3">
      <div className="row">
        <div className="col-md-6">
          <Paper className="p-3">
            <Typography variant="h4" className="mb-3">
              导星相机信息
            </Typography>
            <Typography variant="h6" className="mb-3">
              设备型号: {device_name}
            </Typography>
            <Typography variant="h6" className="mb-3">
              相机物理参数:
            </Typography>
            <Typography variant="body1" className="mb-3">
              x像素数量: {camera_info.x_pixels}
            </Typography>
            <Typography variant="body1" className="mb-3">
              y像素数量: {camera_info.y_pixels}
            </Typography>
            <Typography variant="body1" className="mb-3">
              x像元大小: {camera_info.x_pixel_size.toFixed(2)} um
              </Typography>
            <Typography variant="body1" className="mb-3">
              y像元大小: {camera_info.y_pixel_size.toFixed(2)} um
            </Typography>
          </Paper>
        </div>
        <div className="col-md-6">
          <Paper className="p-3">
            <Typography variant="h4" className="mb-3">
              导星相机控制面板
            </Typography>
            <div className="d-flex align-items-center mb-3">
              <AcUnit size={30} />
              <Typography variant="h6" className="ml-2 mb-0">
                制冷控制
              </Typography>
            </div>
            <div className="form-group">
              {/* <label htmlFor="target-temp">目标温度</label>
              <InputGroup className="mb-3">
                <FormControl
                  id="target-temp"
                  type="number"
                  min="-40"
                  max="0"
                  value={target_temperature}
                  onChange={(e) => set_target_temperature(e.target.value)}
                />
                <InputGroup.Append>
                  <Button
                    variant="outline-secondary"
                    disabled={control_cool_down || !setting_show.has_cool}
                    onClick={() => {
                      sendMessage(JSON.stringify({
                        device_name: 'guider',
                        instruction: 'set_cool_target_temperature',
                        params: [target_temperature],
                      }));
                      set_control_cool_down(true);
                    }}
                  >
                    {control_cool_down ? (
                      <>
                        <HourglassEmpty size={20} className="mr-1" />
                        等待中...
                      </>
                    ) : (
                      <>
                        <KeyboardDoubleArrowUp size={20} className="mr-1" />
                        开始制冷
                      </>
                    )}
                  </Button>
                </InputGroup.Append>
              </InputGroup> */}
            </div>
            {/* <div className="d-flex align-items-center mb-3">
              <Grid4x4 size={30} />
              <Typography variant="h6" className="ml-2 mb-0">
                风扇控制
              </Typography>
            </div>
            <Button
              variant="outline-secondary"
              disabled={control_cool_down || !setting_show.has_fan}
              onClick={() => {
                sendMessage(JSON.stringify({
                  device_name: 'guider',
                  instruction: 'start_fan',
                  params: [],
                }));
                set_control_cool_down(true);
              }}
            >
              {control_cool_down ? (
                <>
                  <HourglassEmpty size={20} className="mr-1" />
                  等待中...
                </>
              ) : (
                <>
                  <KeyboardDoubleArrowUp size={20} className="mr-1" />
                  打开风扇
                </>
              )}
            </Button>
            <Button
              variant="outline-secondary"
              className="ml-2"
              disabled={control_cool_down || !setting_show.has_fan}
              onClick={() => {
                sendMessage(JSON.stringify({
                  device_name: 'guider',
                  instruction: 'stop_fan',
                  params: [],
                }));
                set_control_cool_down(true);
              }}
            >
              {control_cool_down ? (
                <>
                  <HourglassEmpty size={20} className="mr-1" />
                  等待中...
                </>
              ) : (
                <>
                  <KeyboardDoubleArrowRight size={20} className="mr-1" />
                  关闭风扇
                </>
              )}
            </Button>
            <div className="d-flex align-items-center my-3">
              <AcUnit size={30} />
              <Typography variant="h6" className="ml-2 mb-0">
                加热控制
              </Typography>
            </div>
            <Button
              variant="outline-secondary"
              disabled={control_cool_down || !setting_show.has_heater}
              onClick={() => {
                sendMessage(JSON.stringify({
                  device_name: 'guider',
                  instruction: 'start_tc_heat',
                  params: [],
                }));
                set_control_cool_down(true);
              }}
            >
              {control_cool_down ? (
                <>
                  <HourglassEmpty size={20} className="mr-1" />
                  等待中...
                </>
              ) : (
                <>
                  <KeyboardDoubleArrowUp size={20} className="mr-1" />
                  打开加热
                </>
              )}
            </Button>
            <Button
              variant="outline-secondary"
              className="ml-2"
              disabled={control_cool_down || !setting_show.has_heater}
              onClick={() => {
                sendMessage(JSON.stringify({
                  device_name: 'guider',
                  instruction: 'stop_tc_heat',
                  params: [],
                }));
                set_control_cool_down(true);
              }}
            >
              {control_cool_down ? (
                <>
                  <HourglassEmpty size={20} className="mr-1" />
                  等待中...
                </>
              ) : (
                <>
                  <KeyboardDoubleArrowRight size={20} className="mr-1" />
                  关闭加热
                </>
              )}
            </Button> */}
            <div className="form-group my-3">
              <Typography variant="h6" className="mb-0">
                相机设置
              </Typography>
              {setting_show.has_binning && (
                <div className="form-group my-3">
                  <label htmlFor="binning">Binning</label>
                  <InputGroup className="mb-3">
                    <FormControl
                      id="binning"
                      type="number"
                      min="1"
                      max="4"
                      value={input_binning}
                      onChange={(e) => set_input_binning(e.target.value)}
                    />
                    <InputGroup.Append>
                      <Button
                        variant="outline-secondary"
                        disabled={control_cool_down}
                        onClick={() => {
                          sendMessage(JSON.stringify({
                            device_name: 'guider',
                            instruction: 'set_number_parameters',
                            params: ['binning', input_binning],
                          }));
                          set_control_cool_down(true);
                          set_input_binning('');
                        }}
                      >
                        {control_cool_down ? (
                          <>
                            <HourglassEmpty size={20} className="mr-1" />
                            等待中...
                          </>
                        ) : (
                          <>
                            <Done size={20} className="mr-1" />
                            设置
                          </>
                        )}
                      </Button>
                    </InputGroup.Append>
                  </InputGroup>
                </div>
              )}
              {/* <div className="form-group my-3">
                <label htmlFor="high-fullwell">High Fullwell</label>
                <InputGroup className="mb-3">
                  <FormControl
                    id="high-fullwell"
                    type="text"
                    placeholder="输入值"
                    disabled={!setting_show.has_high_fullwell}
                  />
                  <InputGroup.Append>
                    <Button
                      variant="outline-secondary"
                      disabled={control_cool_down || !setting_show.has_high_fullwell}
                      onClick={() => {

                      }}
                    >
                      {control_cool_down ? (
                        <>
                          <HourglassEmpty size={20} className="mr-1" />
                          等待中...
                        </>
                      ) : (
                        <>
                          <Done size={20} className="mr-1" />
                          设置
                        </>
                      )}
                    </Button>
                  </InputGroup.Append>
                </InputGroup>
              </div>
              <div className="form-group my-3">
                <label htmlFor="low-noise">Low Noise</label>
                <InputGroup className="mb-3">
                  <FormControl
                    id="low-noise"
                    type="text"
                    placeholder="输入值"
                    disabled={!setting_show.has_low_noise}
                  />
                  <InputGroup.Append>
                    <Button
                      variant="outline-secondary"
                      disabled={control_cool_down || !setting_show.has_low_noise}
                      onClick={() => {

                      }}
                    >
                      {control_cool_down ? (
                        <>
                          <HourglassEmpty size={20} className="mr-1" />
                          等待中...
                        </>
                      ) : (
                        <>
                          <Done size={20} className="mr-1" />
                          设置
                        </>
                      )}
                    </Button>
                  </InputGroup.Append>
                </InputGroup>
              </div>
              <div className="form-group my-3">
                <label htmlFor="conversion-gain">Conversion Gain</label>
                <InputGroup className="mb-3">
                  <FormControl
                    id="conversion-gain"
                    type="text"
                    placeholder="输入值"
                    disabled={!setting_show.has_conversion_gain}
                  />
                  <InputGroup.Append>
                    <Button
                      variant="outline-secondary"
                      disabled={control_cool_down || !setting_show.has_conversion_gain}
                      onClick={() => {

                      }}
                    >
                      {control_cool_down ? (
                        <>
                          <HourglassEmpty size={20} className="mr-1" />
                          等待中...
                        </>
                      ) : (
                        <>
                          <Done size={20} className="mr-1" />
                          设置
                        </>
                      )}
                    </Button>
                  </InputGroup.Append>
                </InputGroup>
              </div> */}
              <div className="form-group my-3">
                <label htmlFor="gain">增益</label>
                <InputGroup className="mb-3">
                  <FormControl
                    id="gain"
                    type="number"
                    min="0"
                    max="1000"
                    value={input_gain}
                    onChange={(e) => set_input_gain(e.target.value)}
                  />
                  <InputGroup.Append>
                    <Button
                      variant="outline-secondary"
                      disabled={control_cool_down}
                      onClick={() => {
                        sendMessage(JSON.stringify({
                          device_name: 'guider',
                          instruction: 'set_number_parameters',
                          params: ['gain', input_gain],
                        }));
                        set_control_cool_down(true);
                        set_input_gain('');
                      }}
                    >
                      {control_cool_down ? (
                        <>
                          <HourglassEmpty size={20} className="mr-1" />
                          等待中...
                        </>
                      ) : (
                        <>
                          <Done size={20} className="mr-1" />
                          设置
                        </>
                      )}
                    </Button>
                  </InputGroup.Append>
                </InputGroup>
              </div>
              <div className="form-group my-3">
                <label htmlFor="offset">偏置</label>
                <InputGroup className="mb-3">
                  <FormControl
                    id="offset"
                    type="number"
                    min="-100"
                    max="100"
                    value={input_offset}
                    onChange={(e) => set_input_offset(e.target.value)}
                  />
                  <InputGroup.Append>
                    <Button
                      variant="outline-secondary"
                      disabled={control_cool_down}
                      onClick={() => {
                        sendMessage(JSON.stringify({
                          device_name: 'guider',
                          instruction: 'set_number_parameters',
                          params: ['offset', input_offset],
                        }));
                        set_control_cool_down(true);
                        set_input_offset('');
                      }}
                    >
                      {control_cool_down ? (
                        <>
                          <HourglassEmpty size={20} className="mr-1" />
                          等待中...
                        </>
                      ) : (
                        <>
                          <Done size={20} className="mr-1" />
                          设置
                        </>
                      )}
                    </Button>
                  </InputGroup.Append>
                </InputGroup>
              </div>
            </div>
          </Paper>
        </div>
      </div>
      <HelperSnackbar />
    </div>
  );
};

export default DeviceGuiderGeneralControlPanel;