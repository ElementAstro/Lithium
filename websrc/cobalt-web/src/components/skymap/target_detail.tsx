import * as React from "react";
import Card from "react-bootstrap/Card";
import Button from "react-bootstrap/Button";
import Image from "react-bootstrap/Image";
import Row from "react-bootstrap/Row";
import Col from "react-bootstrap/Col";
import Modal from "react-bootstrap/Modal";
import ReactECharts from "echarts-for-react";
import { DateTime } from "luxon";

import { GlobalStore } from "../../store/globalStore";
import Spinner from "react-bootstrap/esm/Spinner";
import { Tab } from "react-bootstrap";

interface TargetSmallCardProps {
  open_dialog: number;
  target_info: IDSOObjectDetailedInfo;
  in_updating: boolean;
  on_choice_maken: (() => void) | null;
  in_manage?: boolean;
}

const alt_fig_options_template: any = {
  grid: {
    top: 10,
    bottom: 20,
    right: "1%",
    left: "32",
  },
  tooltip: {
    trigger: "axis",
  },
  xAxis: {
    type: "time",
  },
  yAxis: {
    type: "value",
    min: 0,
    max: 90,
  },
  series: [
    {
      data: [],
      type: "line",
      smooth: 0.6,
      markLine: {
        silent: true,
        data: [],
      },
    },
  ],
};
const fig_line_data_template: any = [
  {
    name: "日落",
    xAxis: new Date(2023, 1, 1, 1, 20, 30),
    label: {
      formatter: "{b}",
      position: "insideEnd",
    },
    lineStyle: { color: "grey" },
  },
  {
    name: "天文昏影",
    xAxis: new Date(2023, 1, 1, 2, 10, 30),
    label: {
      formatter: "{b}",
      position: "insideEnd",
    },
    lineStyle: { color: "black" },
  },
  {
    name: "日出",
    xAxis: new Date(2023, 1, 1, 2, 10, 30),
    label: {
      formatter: "{b}",
      position: "insideEnd",
    },
    lineStyle: { color: "grey" },
  },
  {
    name: "天文晨光",
    xAxis: new Date(2023, 1, 1, 2, 10, 30),
    label: {
      formatter: "{b}",
      position: "insideEnd",
    },
    lineStyle: { color: "black" },
  },
];
const polar_fig_options_template: any = {
  grid: {
    top: 1,
    bottom: 1,
    right: "0%",
    left: "0%",
  },
  polar: {},
  tooltip: {
    trigger: "axis",
    axisPointer: {
      type: "cross",
    },
  },
  angleAxis: {
    type: "value",
    startAngle: 90,
  },
  radiusAxis: {
    min: 0,
    max: 90,
    inverse: true,
  },
  series: [
    {
      coordinateSystem: "polar",
      type: "line",
      data: [],
    },
  ],
};

const TargetDetailCard: React.FC<TargetSmallCardProps> = (props) => {
  const [open, set_open] = React.useState(false);
  const [add_btn_color, set_add_btn_color] = React.useState<
    "primary" | "secondary" | "success" | "danger" | "warning" | "info"
  >("primary");
  // chart data
  const [alt_fig_options, set_alt_fig_options] = React.useState<any>(
    alt_fig_options_template
  );
  const [polar_fig_options, set_polar_fig_options] = React.useState<any>(
    polar_fig_options_template
  );

  // store ddata
  const target_store = GlobalStore.useAppState(
    (state) => state.TargetListStore
  );
  const add_target_to_store =
    GlobalStore.actions.TargetListStore.add_target_and_focus;
  const set_focus_target_to_store =
    GlobalStore.actions.TargetListStore.change_focus_target;

  const handleClose = () => {
    set_open(false);
  };

  // on mount
  React.useEffect(() => {}, []);

  React.useEffect(() => {
    if (props.open_dialog > 0) {
      set_open(true);
    }
  }, [props.open_dialog]);
  React.useEffect(() => {
    if (!props.in_updating) {
      init_fig_data();
    }
  }, [props.in_updating]);
  // title 整一个大的card，放目标大图

  // functions
  const init_fig_data = () => {
    // process target info to display data
    let new_data = [];
    for (let i = 0; i < props.target_info.altitude.length; i++) {
      new_data.push([
        DateTime.fromFormat(
          props.target_info.altitude[i][0],
          "yyyy-MM-dd HH:mm:ss"
        ).toJSDate(),
        props.target_info.altitude[i][2].toFixed(2),
      ]);
    }
    let new_options = alt_fig_options_template;
    new_options.series[0].data = new_data;

    let new_mark_line = fig_line_data_template;
    new_mark_line[0].xAxis = target_store.twilight_data.evening.sun_set_time;
    new_mark_line[1].xAxis =
      target_store.twilight_data.evening.evening_astro_time;
    new_mark_line[2].xAxis = target_store.twilight_data.morning.sun_rise_time;
    new_mark_line[3].xAxis =
      target_store.twilight_data.morning.morning_astro_time;
    new_options.series[0].markLine.data = new_mark_line;
    set_alt_fig_options(new_options);

    // polar fig change
    let y_value = props.target_info.altitude.map((row) => row[1].toFixed(2));
    let z_value = props.target_info.altitude.map((row) => row[2].toFixed(2));
    let polar_options = polar_fig_options_template;
    let polar_data = [];
    for (let i = 0; i < y_value.length; i++) {
      polar_data.push([y_value[i], z_value[i]]);
    }
    polar_options.series[0].data = polar_data;
    set_polar_fig_options(polar_options);
  };

  const handleAddTarget = () => {
    if (add_target_to_store) {
      add_target_to_store(props.target_info);
      set_add_btn_color("success");
      setTimeout(() => {
        set_add_btn_color("primary");
      }, 500);
    }
  };

  const handleFocusTarget = () => {
    if (set_focus_target_to_store) {
      set_focus_target_to_store(props.target_info.id);
      if (props.on_choice_maken) {
        props.on_choice_maken();
      }
    }
  };

  return (
    <Container fluid className="h-100">
      <Tabs defaultActiveKey="observationData" className="mb-3">
        <Tab eventKey="observationData" title="观测数据">
          <Row className="h-100">
            <Col xs={12} className="h-50">
              <Row className="h-100">
                <Col xs={6} className="h-100">
                  <Card className="h-100 p-0">
                    <Card.Img
                      variant="top"
                      src={target_icon_link}
                      style={{ height: "calc((100vh - 54px) / 2 - 40px)" }}
                    />
                    <Card.Body className="m-0 p-0 mx-1">
                      <Card.Title>{props.target_info.name}</Card.Title>
                    </Card.Body>
                  </Card>
                </Col>
                <Col xs={6} className="h-100">
                  {props.in_updating ? (
                    <div className="h-100 d-flex align-items-center justify-content-center">
                      <Spinner animation="border" variant="primary" />
                    </div>
                  ) : (
                    <ReactECharts
                      option={polar_fig_options}
                      style={{ height: "100%", width: "100%", margin: 0 }}
                    />
                  )}
                </Col>
              </Row>
            </Col>
            <Col xs={12} className="h-50">
              {props.in_updating ? (
                <div className="h-100 d-flex align-items-center justify-content-center">
                  <Spinner animation="border" variant="primary" />
                </div>
              ) : (
                <ReactECharts
                  option={alt_fig_options}
                  style={{ height: "100%", width: "100%", margin: 0 }}
                />
              )}
            </Col>
          </Row>
        </Tab>
        <Tab eventKey="basicInfo" title="基础信息">
          <Row className="h-100">
            <Col xs={12} className="h-40">
              <Card className="h-100 p-0 d-flex">
                <Image
                  src={target_icon_link}
                  style={{
                    height: "calc((100vh - 54px) / 2 - 8px)",
                    width: "calc((100vh - 54px) / 2 - 8px)",
                  }}
                />
                <Card.Body className="d-flex flex-column flex-grow-1">
                  <Card.Title>{props.target_info.name}</Card.Title>
                  <Row>
                    <Col xs={6}>
                      <p>Ra: {props.target_info.ra.toFixed(7)} °</p>
                    </Col>
                    <Col xs={6}>
                      <p>Dec: {props.target_info.dec.toFixed(7)} °</p>
                    </Col>
                    <Col xs={6}>
                      <p className="text-secondary">
                        目标类型{" "}
                        {TranslateTargetType(props.target_info.target_type)}
                      </p>
                    </Col>
                    <Col xs={6}>
                      <p className="text-secondary">
                        目标视角大小 {props.target_info.size} ′
                      </p>
                    </Col>
                  </Row>
                </Card.Body>
              </Card>
            </Col>
            <Col xs={12} className="h-60">
              <Card className="h-100 p-0 d-flex">
                <Row>
                  <Col xs={6}>
                    <p>当前高度: {current_alt.toFixed(0)}°</p>
                  </Col>
                  <Col xs={6}>
                    <p>最高高度: {highest_alt.toFixed(0)}°</p>
                  </Col>
                  <Col xs={6}>
                    <p>估计可拍摄时间: {available_time.toFixed(1)}h</p>
                  </Col>
                  <Col xs={6}>
                    <p></p>
                  </Col>
                </Row>
              </Card>
            </Col>
          </Row>
        </Tab>
        <Tab eventKey="wiki" title="小百科">
          <div
            style={{
              width: "100%",
              height: "100%",
              overflowX: "hidden",
              overflowY: "auto",
            }}
          >
            {/* 目标维基小百科 */}
          </div>
        </Tab>
      </Tabs>

      <div className="d-flex justify-content-end mt-3">
        {props.in_manage ? (
          <></>
        ) : (
          <Button
            variant={add_btn_color}
            size="sm"
            onClick={on_add_target_to_list_clicked}
            className="mr-2"
          >
            加入目标列表
          </Button>
        )}
        <Button
          variant="primary"
          size="sm"
          onClick={on_add_focused_target_clicked}
          className="mr-2"
        >
          以该目标构图
        </Button>
        <Button variant="secondary" size="sm" onClick={() => set_open(false)}>
          退出
        </Button>
      </div>
    </Container>
  );
};

export default TargetDetailCard;
