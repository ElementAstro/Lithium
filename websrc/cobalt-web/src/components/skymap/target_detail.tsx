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

interface TargetSmallCardProps {
  open_dialog: number;
  target_info: IDSOObjectDetailedInfo;
  in_updating: boolean;
  on_choice_maken: (() => void) | null;
  in_manage: boolean;
}

const alt_fig_options_template: any = {
  grid: {
    top: 10,
    bottom: 20,
    right: "1%",
    left: "10%",
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
    top: 0,
    bottom: 0,
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
    <>
      <Card
        style={{ width: "100%", cursor: "pointer" }}
        onClick={() => set_open(true)}
      >
        <Card.Img variant="top" src={props.target_info.img_url} />
        <Card.Body>
          <h5>{props.target_info.name}</h5>
          <Row className="mt-2">
            <Col>
              <h6>RA:</h6>
              {props.in_updating ? (
                <Spinner animation="border" role="status">
                  <span className="sr-only">Loading...</span>
                </Spinner>
              ) : (
                props.target_info.ra.toFixed(4)
              )}
            </Col>
            <Col>
              <h6>DEC:</h6>
              {props.in_updating ? (
                <Spinner animation="border" role="status">
                  <span className="sr-only">Loading...</span>
                </Spinner>
              ) : (
                props.target_info.dec.toFixed(4)
              )}
            </Col>
          </Row>
          {!props.in_manage && (
            <Row className="mt-3">
              <Col>
                <Button
                  variant={add_btn_color}
                  onClick={(e) => {
                    e.stopPropagation();
                    handleAddTarget();
                  }}
                >
                  添加至我的目标
                </Button>
              </Col>
              <Col>
                <Button
                  variant="outline-secondary"
                  onClick={(e) => {
                    e.stopPropagation();
                    handleFocusTarget();
                  }}
                >
                  查看详情
                </Button>
              </Col>
            </Row>
          )}
        </Card.Body>
      </Card>

      <Modal show={open} onHide={handleClose} size="xl">
        <Modal.Header closeButton>
          <Modal.Title>{props.target_info.name}</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Row className="mb-4">
            <Col>
              <Image src={props.target_info.img_url} fluid />
            </Col>
            <Col>
              <h5 className="my-2">基本信息</h5>
              <Row>
                <Col>
                  <h6>英文名:</h6>
                  {props.target_info.aliases.en}
                </Col>
                <Col>
                  <h6>中文名:</h6>
                  {props.target_info.aliases.zh}
                </Col>
              </Row>
              <h6 className="mt-3">描述:</h6>
              {props.target_info.description}
              <Row className="mt-3">
                <Col>
                  <h6>RA:</h6>
                  {props.in_updating ? (
                    <Spinner animation="border" role="status">
                      <span className="sr-only">Loading...</span>
                    </Spinner>
                  ) : (
                    props.target_info.ra.toFixed(4)
                  )}
                </Col>
                <Col>
                  <h6>DEC:</h6>
                  {props.in_updating ? (
                    <Spinner animation="border" role="status">
                      <span className="sr-only">Loading...</span>
                    </Spinner>
                  ) : (
                    props.target_info.dec.toFixed(4)
                  )}
                </Col>
              </Row>
            </Col>
          </Row>

          <hr />

          <h5 className="my-2">高度角曲线</h5>
          {props.in_updating ? (
            <Spinner animation="border" role="status">
              <span className="sr-only">Loading...</span>
            </Spinner>
          ) : (
            <ReactECharts option={alt_fig_options} style={{ height: 300 }} />
          )}

          <hr />

          <h5 className="my-2">方位角-高度角图像</h5>
          {props.in_updating ? (
            <Spinner animation="border" role="status">
              <span className="sr-only">Loading...</span>
            </Spinner>
          ) : (
            <ReactECharts option={polar_fig_options} style={{ height: 500 }} />
          )}
        </Modal.Body>
      </Modal>
    </>
  );
};

export default TargetDetailCard;
