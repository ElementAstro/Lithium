import React from "react";
import {
  Container,
  Row,
  Col,
  Form,
  Button,
  Card,
  ButtonGroup,
} from "react-bootstrap";
import {
  ArrowUp,
  ArrowDown,
  ArrowLeft,
  ArrowRight,
  XCircle,
  CheckCircle,
  HourglassSplit,
} from "react-bootstrap-icons";
import { useImmer } from "use-immer";
import { useEchoWebSocket } from "../../utils/websocketProvider";
import { map } from "lodash";
import Scrollbar from "react-smooth-scrollbar";

const DeviceTelescopeGeneralControlPanel: React.FC = () => {
  const [geo_location, update_geo_location] = useImmer({
    longitude: 0,
    latitude: 0,
    height: 0,
    time_zone: "+8",
  });
  const [device_working_status, set_device_working_status] =
    React.useState(false);
  const [heading_degree, update_heading_degree] = useImmer({
    ra: 0,
    dec: 0,
  });
  const [tracking_status, set_tracking_status] = React.useState(false);
  const [moving_time, set_moving_time] = React.useState("1"); // in seconds

  // ui data
  const [fix_time_selection, set_fix_time_selection] = React.useState(true);
  const [slew_speed_selections, set_slew_speed_selections] = React.useState<
    Array<{ label: String; value: String }>
  >([
    { label: "1x", value: "1x" },
    { label: "2x", value: "2x" },
    { label: "4x", value: "4x" },
    { label: "8x", value: "8x" },
    { label: "16x", value: "16x" },
    { label: "32x", value: "32x" },
    { label: "64x", value: "64x" },
    { label: "128x", value: "128x" },
    { label: "256x", value: "256x" },
    { label: "512x", value: "512x" },
    { label: "1024x", value: "1024x" },
  ]);
  const [selected_slew_speed, set_selected_slew_speed] =
    React.useState("0(0.25x)");

  const time_selections = [
    { label: "0.001", value: "0.001" },
    { label: "0.01", value: "0.01" },
    { label: "0.05", value: "0.05" },
    { label: "0.1", value: "0.1" },
    { label: "0.5", value: "0.5" },
    { label: "1", value: "1" },
    { label: "2", value: "2" },
  ];
  // const slew_speed_all_available_selections = [
  //   [
  //     {label: '0(0.25x)', value: '0(0.25x)'},
  //     {label: '1(0.5x)', value: '1(0.5x)'},
  //     {label: '2(1x)', value: '2(1x)'},
  //     {label: '3(2x)', value: '3(2x)'},
  //     {label: '4(4x)', value: '4(4x)'},
  //     {label: '5(8x)', value: '5(8x)'},
  //     {label: '6(20x)', value: '6(20x)'},
  //     {label: '7(48x)', value: '7(48x)'},
  //     {label: '8(Half-Max)', value: '8(Half-Max)'},
  //     {label: '9(Max)', value: '9(Max)'},
  //   ]
  // ];

  const process_ws_message = (msg: any): void => {
    // console.log(msg);
    if (msg.type == "error") {
      return;
    }
    if (msg.device_name == "telescope") {
      switch (msg.instruction) {
        case "set_long_lat": {
          break;
        }
        case "get_static_info": {
          update_geo_location((draft) => {
            draft.longitude = msg.data.long;
            draft.latitude = msg.data.lat;
            draft.height = msg.data.elev;
          });
          if (msg.data.time_zone !== null) {
            update_geo_location((draft) => {
              draft.time_zone = msg.data.time_zone;
            });
          }
          break;
        }
        case "get_set_params": {
          // update all selections to selects.
          let new_selections = [];
          for (let i = 0; i < msg.data.slew_rate.selections.length; i++) {
            new_selections.push({
              label: msg.data.slew_rate.selections[i],
              value: msg.data.slew_rate.selections[i],
            });
          }
          set_slew_speed_selections(new_selections);
          // set current slection.
          set_selected_slew_speed(msg.data.slew_rate.value);
          break;
        }
        case "get_real_time_info": {
          update_heading_degree((draft) => {
            draft.ra = msg.data.ra;
            draft.dec = msg.data.dec;
          });
          set_device_working_status(msg.data.is_moving);
          set_tracking_status(msg.data.tracking);
          break;
        }
        case "set_telescope_info": {
          break;
        }
        case "set_track": {
          break;
        }
        case "start_track": {
          break;
        }
        case "set_track_mode": {
          break;
        }
        case "stop_track": {
          break;
        }
        case "go_home": {
          break;
        }
        case "park": {
          break;
        }
        case "unpark": {
          break;
        }
        case "abort": {
          break;
        }
        case "handle_move_button": {
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
    sendMessage(
      JSON.stringify({
        device_name: "telescope",
        instruction: "get_static_info",
        params: [],
      })
    );
    sendMessage(
      JSON.stringify({
        device_name: "telescope",
        instruction: "get_set_params",
        params: [],
      })
    );
    // This will fire only on mount.
    const interval = setInterval(() => {
      // console.log('Logs every second');
      sendMessage(
        JSON.stringify({
          device_name: "telescope",
          instruction: "get_real_time_info",
          params: [],
        })
      );
    }, MINUTE_MS);

    return () => {
      console.log("clear interval");
      clearInterval(interval);
      removeListener(process_ws_message);
    }; // This represents the unmount function, in which you need to clear your interval to prevent memory leaks.
  }, []);
  React.useEffect(() => {
    sendMessage(
      JSON.stringify({
        device_name: "telescope",
        instruction: "set_slew_rate",
        params: [selected_slew_speed],
      })
    );
  }, [selected_slew_speed]);

  const [arrow_in_action, set_arrow_in_action] = React.useState(false);
  const arrow_action_func = (btn_type: string, action: string) => {
    // console.log('!!!arrow', btn_type, action);

    if (fix_time_selection) {
      if (action == "press") {
        if (arrow_in_action) {
          // console.log('in action, no effect');
          return;
        }
        set_arrow_in_action(true);
        let wait_time = 0;
        if (moving_time[0] == null) {
          wait_time = 1000;
        } else {
          wait_time = parseFloat(moving_time[0]);
        }
        // set time out
        // console.log('set timeout');
        sendMessage(
          JSON.stringify({
            device_name: "telescope",
            instruction: "handle_move_button",
            params: [btn_type, true],
          })
        );
        setTimeout(() => {
          // console.log('timeout finished');
          sendMessage(
            JSON.stringify({
              device_name: "telescope",
              instruction: "handle_move_button",
              params: [btn_type, false],
            })
          );
          set_arrow_in_action(false);
        }, wait_time);
      }
    } else {
      if (action == "press") {
        if (arrow_in_action) {
          console.log("in action, no effect");
          return;
        }
        // console.log('press down');
        sendMessage(
          JSON.stringify({
            device_name: "telescope",
            instruction: "handle_move_button",
            params: [btn_type, true],
          })
        );
        set_arrow_in_action(true);
      } else if (action == "release") {
        // console.log('released');
        sendMessage(
          JSON.stringify({
            device_name: "telescope",
            instruction: "handle_move_button",
            params: [btn_type, false],
          })
        );
        set_arrow_in_action(false);
      } else {
        console.log("error!");
      }
    }

    // if (arrow_in_action){
    //   console.log('in action, no effect');
    //   return;
    // }else{
    //   set_arrow_in_action(true);
    //   if (fix_time_selection){
    //     if (action == 'press'){
    //       let wait_time = 0;
    //       if (moving_time[0] == null){
    //         wait_time = 1000;
    //       }else{
    //         wait_time = parseFloat(moving_time[0]) * 1000;
    //       }
    //       // set time out
    //       console.log('set timeout');
    //       setTimeout(() => {
    //         console.log('timeout finished');
    //         set_arrow_in_action(false);
    //       }, wait_time)
    //     }
    //   }else{
    //     if (action == 'press'){
    //       console.log('press down');
    //       set_arrow_in_action(true);
    //     }else if (action == 'release'){
    //       console.log('released');
    //       set_arrow_in_action(false);
    //     }else {
    //       console.log('error!');
    //     }
    //   }
    // }
  };

  return (
    <Container fluid>
      <Row>
        <Col xs={12} md={6}>
          <Card className="border-primary">
            <Card.Body>
              <Card.Title>设备记录经度: {geo_location.longitude} °</Card.Title>
              <Card.Text>设备记录纬度: {geo_location.latitude} °</Card.Text>
              <Card.Text>设备时区: {geo_location.time_zone}</Card.Text>
              <Card.Text>
                {device_working_status ? (
                  <Button variant="danger">
                    <HourglassSplit />
                    设备忙
                  </Button>
                ) : (
                  <Button variant="success">
                    <CheckCircle />
                    设备待机
                  </Button>
                )}
              </Card.Text>
              <Card.Title>设备指向</Card.Title>
              <Card.Text>ra: {heading_degree.ra} °</Card.Text>
              <Card.Text>dec: {heading_degree.dec} °</Card.Text>
            </Card.Body>
            <Card.Footer>
              <Row>
                <Col>
                  <div className="d-grid gap-2">
                    {tracking_status ? (
                      <Button variant="success" size="lg">
                        <CheckCircle />
                        跟踪中
                      </Button>
                    ) : (
                      <Button variant="danger" size="lg">
                        <XCircle />
                        未跟踪
                      </Button>
                    )}
                  </div>
                </Col>
                <Col>
                  <div className="d-grid gap-2">
                    <Button
                      variant="outline-primary"
                      size="lg"
                      onClick={() => {
                        if (tracking_status) {
                          sendMessage(
                            JSON.stringify({
                              device_name: "telescope",
                              instruction: "stop_track",
                              params: [],
                            })
                          );
                        } else {
                          sendMessage(
                            JSON.stringify({
                              device_name: "telescope",
                              instruction: "start_track",
                              params: [],
                            })
                          );
                        }
                        set_device_working_status(true);
                      }}
                    >
                      跟踪
                    </Button>
                  </div>
                </Col>
              </Row>

              <Row className="mt-3">
                <Col>
                  <Button
                    variant="outline-primary"
                    size="lg"
                    className="w-100"
                    onClick={() => {
                      sendMessage(
                        JSON.stringify({
                          device_name: "telescope",
                          instruction: "park",
                          params: [],
                        })
                      );
                    }}
                  >
                    Park
                  </Button>
                </Col>
                <Col>
                  <Button
                    variant="outline-primary"
                    size="lg"
                    className="w-100"
                    onClick={() => {
                      sendMessage(
                        JSON.stringify({
                          device_name: "telescope",
                          instruction: "unpark",
                          params: [],
                        })
                      );
                    }}
                  >
                    UnPark
                  </Button>
                </Col>
                <Col>
                  <Button
                    variant="outline-primary"
                    size="lg"
                    className="w-100"
                    onClick={() => {
                      sendMessage(
                        JSON.stringify({
                          device_name: "telescope",
                          instruction: "go_home",
                          params: [],
                        })
                      );
                    }}
                  >
                    归零
                  </Button>
                </Col>
              </Row>
            </Card.Footer>
          </Card>
        </Col>
        <Col xs={12} md={6}>
          <Scrollbar continuousScrolling={true}>
            <Card className="border-primary">
              <Form.Group>
                <Form.Label>设置赤道仪移动速率</Form.Label>
                <Form.Control
                  as="select"
                  value={selected_slew_speed}
                  onChange={(event) => {
                    set_selected_slew_speed(event.target.value);
                  }}
                >
                  {slew_speed_selections.map((one_slew_selection, index) => {
                    return (
                      <option value={one_slew_selection.value} key={index}>
                        {one_slew_selection.label}
                      </option>
                    );
                  })}
                </Form.Control>
              </Form.Group>
            </Card>
            <Card className="border-info">
              <Form.Group>
                <Form.Check
                  type="switch"
                  id="fixTimeSwitch"
                  label={fix_time_selection ? "固定时间模式" : "模拟手柄模式"}
                  checked={fix_time_selection}
                  onChange={(event) => {
                    if (!arrow_in_action) {
                      set_fix_time_selection(event.target.checked);
                    } else {
                      // 提示松开按钮
                    }
                  }}
                />
                {fix_time_selection && (
                  <Form.Group>
                    <Form.Label>选择手动移动时长(s)</Form.Label>
                    <Form.Control
                      as="select"
                      value={moving_time}
                      onChange={(event) => {
                        set_moving_time(event.target.value);
                      }}
                    >
                      {time_selections.map((one_time_select, index) => {
                        return (
                          <option value={one_time_select.value} key={index}>
                            {one_time_select.label}
                          </option>
                        );
                      })}
                    </Form.Control>
                  </Form.Group>
                )}
                <Card.Title>
                  {fix_time_selection ? "固定移动时间" : "长按手柄模式"}
                </Card.Title>
                <Container>
                  <Row>
                    <Col xs={4}></Col>
                    <Col xs={4}>
                      <div className="d-grid gap-2">
                        <Button
                          variant="outline-primary"
                          size="lg"
                          onMouseDown={() => {
                            arrow_action_func("up", "press");
                          }}
                          onMouseUp={() => {
                            arrow_action_func("up", "release");
                          }}
                        >
                          <ArrowUp />
                        </Button>
                      </div>
                    </Col>
                    <Col xs={4}></Col>
                  </Row>
                  <Row className="mt-3">
                    <Col xs={4}>
                      <div className="d-grid gap-2">
                        <Button
                          variant="outline-primary"
                          size="lg"
                          onMouseDown={() => {
                            arrow_action_func("left", "press");
                          }}
                          onMouseUp={() => {
                            arrow_action_func("left", "release");
                          }}
                        >
                          <ArrowLeft />
                        </Button>
                      </div>
                    </Col>
                    <Col xs={4}>
                      <div className="d-grid gap-2">
                        <Button
                          variant="danger"
                          size="lg"
                          onClick={() => {
                            sendMessage(
                              JSON.stringify({
                                device_name: "telescope",
                                instruction: "abort",
                                params: [],
                              })
                            );
                          }}
                        >
                          <XCircle />
                        </Button>
                      </div>
                    </Col>
                    <Col xs={4}>
                      <div className="d-grid gap-2">
                        <Button
                          variant="outline-primary"
                          size="lg"
                          onMouseDown={() => {
                            arrow_action_func("right", "press");
                          }}
                          onMouseUp={() => {
                            arrow_action_func("right", "release");
                          }}
                        >
                          <ArrowRight />
                        </Button>
                      </div>
                    </Col>
                  </Row>
                  <Row className="mt-3">
                    <Col xs={4}></Col>
                    <Col xs={4}>
                      <div className="d-grid gap-2">
                        <Button
                          variant="outline-primary"
                          size="lg"
                          onMouseDown={() => {
                            arrow_action_func("down", "press");
                          }}
                          onMouseUp={() => {
                            arrow_action_func("down", "release");
                          }}
                        >
                          <ArrowDown />
                        </Button>
                      </div>
                    </Col>
                    <Col xs={4}></Col>
                  </Row>
                </Container>
              </Form.Group>
            </Card>
          </Scrollbar>
        </Col>
      </Row>
    </Container>
  );
};

export default DeviceTelescopeGeneralControlPanel;
