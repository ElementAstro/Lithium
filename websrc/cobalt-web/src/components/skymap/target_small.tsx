import * as React from "react";
import TargetDetailCard from "./target_detail";
import Card from "react-bootstrap/Card";
import Button from "react-bootstrap/Button";
import OverlayTrigger from "react-bootstrap/OverlayTrigger";
import Tooltip from "react-bootstrap/Tooltip";
import ReactECharts from "echarts-for-react";
import { DateTime } from "luxon";
import { GlobalStore } from "../../store/globalStore";
import { XCircle, Check, FilePlus, CheckCircle } from "react-bootstrap-icons";
import * as AXIOSOF from "../../services/object_finding_api";

interface TargetSmallCardProps {
  target_info: IDSOObjectDetailedInfo | IDSOFramingObjectInfo;
  card_index: number;
  on_card_clicked: ((card_index: number, checked: boolean) => void) | null;
  on_choice_maken: (() => void) | null;
  in_manage?: boolean;
}

const fig_options_template: any = {
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
    axisLabel: {
      formatter: "{HH}",
    },
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
      tooltip: {
        trigger: "none",
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

function isDetailed(object: any): object is IDSOObjectDetailedInfo {
  return "altitude" in object;
}

const TargetSmallCard: React.FC<TargetSmallCardProps> = (props) => {
  // ui control
  const [show_detail, set_show_detail] = React.useState(0);
  const [this_checked, set_this_checked] = React.useState(false);
  const [added_flag, set_added_flag] = React.useState(false);
  const [target_icon_link, set_target_icon_link] = React.useState("");
  const [add_tooltip_open, set_add_tooltip_open] = React.useState(false);

  // display data
  // display data
  const [echarts_options, set_echarts_options] =
    React.useState<any>(fig_options_template);
  const [real_target_info, set_real_target_info] =
    React.useState<IDSOObjectDetailedInfo>({
      name: "",
      alias: "",
      ra: 0,
      dec: 0,
      target_type: "",
      const: "",
      size: 0,
      transit_month: 0,
      transit_date: "",
      filter: "",
      focal_length: 0,
      altitude: [],
      Top200: null,
    });
  const [in_updating, set_in_updating] = React.useState(true);

  // store data
  const target_store = GlobalStore.useAppState(
    (state) => state.TargetListStore
  );
  const add_target_to_store =
    GlobalStore.actions.TargetListStore.add_target_and_focus;
  const save_all_targets = GlobalStore.actions.TargetListStore.save_all_targets;
  const set_focus_target_to_store =
    GlobalStore.actions.TargetListStore.change_focus_target;

  // on mount
  React.useEffect(() => {
    if (isDetailed(props.target_info)) {
      let new_target_frame_info: IDSOObjectDetailedInfo = {
        name: props.target_info.name,
        ra: props.target_info.ra,
        dec: props.target_info.dec,
        target_type: props.target_info.target_type,
        size: props.target_info.size,
        altitude: props.target_info.altitude,
        alias: props.target_info.alias,
        const: props.target_info.const,
        transit_month: props.target_info.transit_month,
        transit_date: props.target_info.transit_date,
        filter: props.target_info.filter,
        focal_length: props.target_info.focal_length,
        Top200: props.target_info.Top200,
      };
      set_real_target_info(new_target_frame_info);
    } else {
      construct_framing_info2card_info();
    }
    if (process.env.NODE_ENV == "development") {
      set_target_icon_link("/api/file/DSO/" + props.target_info.name + ".jpg");
    } else {
      set_target_icon_link("/file/DSO/" + props.target_info.name + ".jpg");
    }
  }, []);

  React.useEffect(() => {
    initial_fig_data();
  }, [real_target_info]);

  React.useEffect(() => {
    if ("checked" in props.target_info) {
      set_this_checked(props.target_info.checked);
    }
  }, [props.target_info]);

  const initial_fig_data = () => {
    let new_data = [];
    for (let i = 0; i < real_target_info.altitude.length; i++) {
      new_data.push([
        DateTime.fromFormat(
          real_target_info.altitude[i][0],
          "yyyy-MM-dd HH:mm:ss"
        ).toJSDate(),
        real_target_info.altitude[i][2].toFixed(2),
      ]);
    }
    let new_options = JSON.parse(JSON.stringify(fig_options_template));
    new_options.series[0].data = new_data;
    let new_mark_line = fig_line_data_template;
    new_mark_line[0].xAxis = target_store.twilight_data.evening.sun_set_time;
    new_mark_line[1].xAxis =
      target_store.twilight_data.evening.evening_astro_time;
    new_mark_line[2].xAxis = target_store.twilight_data.morning.sun_rise_time;
    new_mark_line[3].xAxis =
      target_store.twilight_data.morning.morning_astro_time;
    new_options.series[0].markLine.data = new_mark_line;
    set_echarts_options(new_options);
    set_in_updating(false);
  };

  const construct_framing_info2card_info = async () => {
    try {
      const new_curve_data = await AXIOSOF.getTargetALtCurveOnly(
        props.target_info.ra,
        props.target_info.dec
      );
      if (new_curve_data.success) {
        let new_target_frame_info: IDSOObjectDetailedInfo = {
          name: props.target_info.name,
          ra: props.target_info.ra,
          dec: props.target_info.dec,
          target_type: props.target_info.target_type,
          size: props.target_info.size,
          altitude: new_curve_data.data.altitude,
          alias: "",
          const: "",
          transit_month: 0,
          transit_date: "",
          filter: "",
          focal_length: 0,
          Top200: null,
        };
        set_real_target_info(new_target_frame_info);
        initial_fig_data();
        set_in_updating(false);
      } else {
        return null;
      }
    } catch (err) {
      return null;
    }
  };

  const on_add_target_to_framing_clicked = () => {
    let to_add_object: IDSOFramingObjectInfo = {
      name: props.target_info.name,
      ra: props.target_info.ra,
      dec: props.target_info.dec,
      rotation: 0,
      flag: "",
      tag: "",
      target_type: props.target_info.target_type,
      size: props.target_info.size,
      checked: false,
    };
    set_focus_target_to_store(to_add_object);
    if (props.on_choice_maken != null) {
      props.on_choice_maken();
    }
  };

  const on_add_target_to_list_clicked = () => {
    if (added_flag) {
      set_add_tooltip_open(true);
      setTimeout(() => set_add_tooltip_open(false), 3000);
    } else {
      let to_add_object: IDSOFramingObjectInfo = {
        name: props.target_info.name,
        ra: props.target_info.ra,
        dec: props.target_info.dec,
        rotation: 0,
        flag: "",
        tag: "",
        target_type: props.target_info.target_type,
        size: props.target_info.size,
        checked: false,
      };
      add_target_to_store(to_add_object);
      save_all_targets();
      set_focus_target_to_store({
        name: props.target_info.name,
        ra: props.target_info.ra,
        dec: props.target_info.dec,
        rotation: 0,
        flag: "",
        tag: "",
        target_type: props.target_info.target_type,
        size: props.target_info.size,
        checked: false,
      });
      set_added_flag(true);
    }
  };

  return (
    <Card className="mb-3">
      <Card.Body className="p-0">
        <div className="d-flex">
          <div className="position-relative" style={{ width: 130 }}>
            <img
              src={target_icon_link}
              alt=""
              className="w-100"
              onClick={() => {
                set_this_checked(!this_checked);
                if (props.on_card_clicked != null) {
                  props.on_card_clicked(props.card_index, !this_checked);
                }
              }}
            />
            <Button
              variant="primary"
              size="sm"
              className="position-absolute"
              style={{
                right: 0,
                bottom: "50%",
                transform: "translateY(50%)",
                borderRadius: "50%",
                zIndex: 2,
              }}
              onClick={() => set_show_detail(show_detail + 1)}
            >
              {/*<MoreHoriz />*/}
            </Button>
            <OverlayTrigger
              placement="top"
              show={add_tooltip_open}
              overlay={
                <Tooltip id="tooltip-top">
                  已添加到待拍摄列表，如需要删除目标，请到目标管理界面删除
                </Tooltip>
              }
            >
              <Button
                variant="primary"
                size="sm"
                className="position-absolute"
                style={{
                  left: 0,
                  bottom: "50%",
                  transform: "translateY(50%)",
                  borderRadius: "50%",
                  zIndex: 2,
                }}
                onClick={on_add_target_to_list_clicked}
              >
                {added_flag ? <Check /> : <FilePlus />}
              </Button>
            </OverlayTrigger>
            {props.on_card_clicked != null && (
              <Button
                variant="transparent"
                size="sm"
                className="position-absolute"
                style={{
                  left: 0,
                  top: 0,
                  backgroundColor: "transparent",
                  zIndex: 2,
                }}
                onClick={() => {
                  set_this_checked(!this_checked);
                  if (props.on_card_clicked != null) {
                    props.on_card_clicked(props.card_index, !this_checked);
                  }
                }}
              >
                {this_checked ? <CheckCircle /> : <XCircle />}
              </Button>
            )}
          </div>
          <div
            className="flex-grow-1 pl-3"
            onClick={on_add_target_to_framing_clicked}
          >
            <Card.Title className="text-success">
              {props.target_info.name}
            </Card.Title>
            <Card.Text>Ra: {props.target_info.ra.toFixed(5)} °</Card.Text>
            <Card.Text>Dec: {props.target_info.dec.toFixed(5)} °</Card.Text>
          </div>
        </div>
        <div className="position-relative" style={{ height: 130 }}>
          {in_updating ? (
            <div className="position-absolute w-100 h-100 d-flex align-items-center justify-content-center">
              <div className="spinner-border text-primary" role="status">
                <span className="sr-only">Loading...</span>
              </div>
            </div>
          ) : (
            <ReactECharts
              option={echarts_options}
              notMerge={true}
              lazyUpdate={true}
              style={{ width: "100%", height: "100%" }}
            />
          )}
        </div>
      </Card.Body>
      <TargetDetailCard
        open_dialog={show_detail}
        target_info={real_target_info}
        in_updating={in_updating}
        on_choice_maken={props.on_choice_maken}
        in_manage={props.in_manage}
      />
    </Card>
  );
};

TargetSmallCard.defaultProps = {
  on_choice_maken: null,
  in_manage: false,
};

export default TargetSmallCard;
