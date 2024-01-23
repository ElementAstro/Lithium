import React, { useState, useEffect } from "react";
import TargetDetailCard from "./target_detail";
import Figure from "react-bootstrap/Figure";
import Button from "react-bootstrap/Button";
import Form from "react-bootstrap/Form";
import Card from "react-bootstrap/Card";
import Accordion from "react-bootstrap/Accordion";
import { Heart, ThreeDots } from "react-bootstrap-icons";
import Image from "react-bootstrap/Image";
import ReactECharts from "echarts-for-react";
import * as AXIOSOF from "../../services/object_finding_api";
import { DateTime } from "luxon";
import { GlobalStore } from "../../store/globalStore";

interface TargetSmallCardProps {
  target_info: IDSOObjectDetailedInfo | IDSOFramingObjectInfo;
  card_index: number;
  on_card_clicked: ((card_index: number, checked: boolean) => void) | null;
  on_choice_maken: (() => void) | null;
  in_manage: boolean;
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
// todo, how to auto get the target pic url?
function isDetailed(object: any): object is IDSOObjectDetailedInfo {
  return "altitude" in object;
}

const TargetSmallCard: React.FC<TargetSmallCardProps> = (props) => {
  // ui control
  const [show_detail, set_show_detail] = React.useState(0);
  const [this_checked, set_this_checked] = React.useState(false);
  const [add_btn_color, set_add_btn_color] =
    React.useState<ColorPaletteProp>("primary");

  // display data
  const [echarts_options, set_echarts_options] =
    React.useState<any>(fig_options_template);
  const [real_target_info, set_real_target_info] =
    React.useState<IDSOObjectDetailedInfo>({
      name: "",
      ra: 0,
      dec: 0,
      target_type: "",
      bmag: 0,
      vmag: 0,
      size: 0,
      moon_distance: 0,
      altitude: [],
    });
  const [in_updating, set_in_updating] = React.useState(true);

  // store ddata
  const target_store = GlobalStore.useAppState(
    (state) => state.TargetListStore
  );
  const add_target_to_store =
    GlobalStore.actions.TargetListStore.add_target_and_focus;
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
        bmag: props.target_info.bmag,
        vmag: props.target_info.vmag,
        size: props.target_info.size,
        moon_distance: props.target_info.moon_distance,
        altitude: props.target_info.altitude,
      };
      set_real_target_info(new_target_frame_info);
    } else {
      // console.log('is framing object!');
      // let new_target_frame_info: IDSOObjectDetailedInfo = {
      //   name: props.target_info.name,
      //   ra: props.target_info.ra,
      //   dec: props.target_info.dec,
      //   target_type: props.target_info.target_type,
      //   bmag: props.target_info.bmag,
      //   vmag: props.target_info.vmag,
      //   size: props.target_info.size,
      //   moon_distance: 0,
      //   altitude: [],
      // }
      // set_real_target_info(new_target_frame_info);
      construct_framing_info2card_info();
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
    // process target info to display data
    let new_data = [];
    for (let i = 0; i < real_target_info.altitude.length; i++) {
      // console.log(props.target_info.altitude[i][0], DateTime.fromFormat(props.target_info.altitude[i][0], 'yyyy-MM-dd HH:mm:ss'));
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
          bmag: props.target_info.bmag,
          vmag: props.target_info.vmag,
          size: props.target_info.size,
          moon_distance: new_curve_data.data.moon_distance,
          altitude: new_curve_data.data.altitude,
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
      bmag: props.target_info.bmag,
      vmag: props.target_info.vmag,
      size: props.target_info.size,
      checked: false,
    };
    // add_target_to_store(to_add_object);
    set_focus_target_to_store(to_add_object);
    if (props.on_choice_maken != null) {
      props.on_choice_maken();
    }
  };
  const on_add_target_to_list_clicked = () => {
    let to_add_object: IDSOFramingObjectInfo = {
      name: props.target_info.name,
      ra: props.target_info.ra,
      dec: props.target_info.dec,
      rotation: 0,
      flag: "",
      tag: "",
      target_type: props.target_info.target_type,
      bmag: props.target_info.bmag,
      vmag: props.target_info.vmag,
      size: props.target_info.size,
      checked: false,
    };
    add_target_to_store(to_add_object);
    set_focus_target_to_store({
      name: props.target_info.name,
      ra: props.target_info.ra,
      dec: props.target_info.dec,
      rotation: 0,
      flag: "",
      tag: "",
      target_type: props.target_info.target_type,
      bmag: props.target_info.bmag,
      vmag: props.target_info.vmag,
      size: props.target_info.size,
      checked: false,
    });
    set_add_btn_color("danger");
    // if (props.on_choice_maken!=null){
    //   props.on_choice_maken();
    // }
  };

  return (
    <Card bg="light" border="primary">
      <Figure>
        <Image
          width={90}
          height={90}
          alt=""
          src="https://images.unsplash.com/photo-1507833423370-a126b89d394b?auto=format&fit=crop&w=90"
          onClick={() => {
            set_this_checked(!this_checked);
            if (props.on_card_clicked != null) {
              props.on_card_clicked(props.card_index, !this_checked);
            }
          }}
        />
        <Button
          variant="outline-dark"
          className="position-absolute bottom-50 end-0"
          onClick={() => set_show_detail(show_detail + 1)}
        >
          <ThreeDots />
        </Button>
        <Button
          variant="outline-primary"
          className="position-absolute bottom-50 start-0"
          onClick={on_add_target_to_list_clicked}
        >
          <Heart />
        </Button>
      </Figure>
      <Form onDoubleClick={on_add_target_to_framing_clicked}>
        <Card.Body style={{ width: "110px" }}>
          <Card.Title>{props.target_info.name}</Card.Title>
          <Card.Text>Ra: {props.target_info.ra.toFixed(7)} °</Card.Text>
          <Card.Text>Dec: {props.target_info.dec.toFixed(7)} °</Card.Text>
        </Card.Body>
      </Form>
      <Card.Body style={{ width: "100%" }}>
        <ReactECharts
          option={echarts_options}
          notMerge={true}
          lazyUpdate={true}
          style={{ width: "95%", height: "130px" }}
        />
      </Card.Body>
      {props.on_card_clicked == null ? (
        <></>
      ) : (
        <Card.Body
          className="border-start border-3 border-primary"
          style={{
            writingMode: "vertical-rl",
            textAlign: "center",
            fontSize: "xs",
            fontWeight: "xl",
            letterSpacing: "1px",
            textTransform: "uppercase",
          }}
          onClick={() => {
            set_this_checked(!this_checked);
            if (props.on_card_clicked != null) {
              props.on_card_clicked(props.card_index, !this_checked);
            }
          }}
        >
          {this_checked ? "checked" : "unchecked"}
        </Card.Body>
      )}
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
