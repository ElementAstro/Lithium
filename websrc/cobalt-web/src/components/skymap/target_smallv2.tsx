import React from 'react';
import Card from 'react-bootstrap/Card';
import Button from 'react-bootstrap/Button';
import Image from 'react-bootstrap/Image';
import Form from 'react-bootstrap/esm/Form';

import ReactECharts from 'echarts-for-react';
import { DateTime } from "luxon";

import TargetDetailCard from './target_detail';
import { GlobalStore } from '../../store/globalStore';

interface TargetSmallCardProps {
  target_info: IDSOObjectDetailedInfo;
  card_index: number;
  on_card_clicked: ((
    card_index: number,
    checked: boolean
  )=>void) | null;
  on_choice_maken: (()=>void) | null;
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
    trigger: 'axis'
  },
  xAxis: {
    type: 'time'
  },
  yAxis: {
    type: 'value',
    min: 0,
    max: 90,
  },
  series: [
    {
      data: [],
      type: 'line',
      smooth: 0.6,
      markLine:{
        silent: true,
        data: []
      }
    }
  ]
}
const fig_line_data_template: any = [
  { name: '日落', xAxis: new Date(2023, 1, 1, 1,20,30), label: {
    formatter: '{b}',
    position: 'insideEnd',
  }, lineStyle:{ color: 'grey'}
  },
  { name: '天文昏影', xAxis: new Date(2023, 1, 1, 2,10,30), label: {
      formatter: '{b}',
      position: 'insideEnd',
    }, lineStyle:{ color: 'black'}
  },
  { name: '日出', xAxis: new Date(2023, 1, 1, 2,10,30), label: {
      formatter: '{b}',
      position: 'insideEnd',
    }, lineStyle:{ color: 'grey'}
  },
  { name: '天文晨光', xAxis: new Date(2023, 1, 1, 2,10,30), label: {
      formatter: '{b}',
      position: 'insideEnd',
    }, lineStyle:{ color: 'black'}
  },
]

// todo, how to auto get the target pic url?


const TargetSmallCardHori: React.FC<TargetSmallCardProps> = (props) => {
  // ui control
  const [show_detail, set_show_detail] = React.useState(0);
  const [this_checked, set_this_checked] = React.useState(false);
  const [in_updating, set_in_updating] = React.useState(false);

  // display data
  const [echarts_options, set_echarts_options] = React.useState<any>(fig_options_template);

  // store ddata
  const target_store = GlobalStore.useAppState((state) => state.TargetListStore);
  const add_target_to_store = GlobalStore.actions.TargetListStore.add_target_and_focus;
  const set_focus_target_to_store = GlobalStore.actions.TargetListStore.change_focus_target;

  // on mount
  React.useEffect(()=>{
    // process target info to display data
    let new_data = [];
    for (let i=0;i<props.target_info.altitude.length;i++){
      // console.log(props.target_info.altitude[i][0], DateTime.fromFormat(props.target_info.altitude[i][0], 'yyyy-MM-dd HH:mm:ss'));
      new_data.push([
        DateTime.fromFormat(props.target_info.altitude[i][0], 'yyyy-MM-dd HH:mm:ss').toJSDate(),
        props.target_info.altitude[i][2].toFixed(2),
      ])
    }
    let new_options = fig_options_template;
    new_options.series[0].data = new_data;
    let new_mark_line = fig_line_data_template;
    new_mark_line[0].xAxis=target_store.twilight_data.evening.sun_set_time;
    new_mark_line[1].xAxis=target_store.twilight_data.evening.evening_astro_time;
    new_mark_line[2].xAxis=target_store.twilight_data.morning.sun_rise_time;
    new_mark_line[3].xAxis=target_store.twilight_data.morning.morning_astro_time;
    new_options.series[0].markLine.data = new_mark_line;
    set_echarts_options(new_options);
  }, [])
  // 
  // React.useEffect( () => {
  //   if (props.on_card_clicked != null){
  //     props.on_card_clicked(props.card_index, this_checked);
  //   }
  // }, [this_checked])

  // globalstore handle
  const on_add_target_to_framing_clicked = () => {
    let to_add_object: IDSOFramingObjectInfo = {
      name: props.target_info.name,
      ra: props.target_info.ra,
      dec: props.target_info.dec,
      rotation: 0,
      flag: '',
      tag: '',
      target_type: props.target_info.target_type,
      bmag: props.target_info.bmag,
      vmag: props.target_info.vmag,
      size: props.target_info.size,
      checked: false,
    }
    // add_target_to_store(to_add_object);
    set_focus_target_to_store(to_add_object);
    if (props.on_choice_maken != null){
      props.on_choice_maken();
    }
  }

  return (
    <Card style={{ maxWidth: 345 }}>
      <Card.Body>
        <Card.Title>{props.target_info.name}</Card.Title>
        <Card.Text>Ra: {props.target_info.ra} °</Card.Text>
        <Card.Text>Dec: {props.target_info.dec} °</Card.Text>
        <Image src="/static/images/cards/contemplative-reptile.jpg" thumbnail />
        <ReactECharts
          option={echarts_options}
          style={{ width: "330px", margin: 0 }}
        />
      </Card.Body>
      <Card.Footer>
        {props.on_card_clicked != null && (
          <Form.Check
            checked={this_checked}
            onChange={() => {
              set_this_checked(!this_checked);
              if (props.on_card_clicked != null){
                props.on_card_clicked(props.card_index, this_checked);
              }
            }}
            aria-label="controlled"
          />
        )}
        <Button variant="primary" size="sm" onClick={on_add_target_to_framing_clicked}>
          选择目标构图
        </Button>
        <Button variant="primary" size="sm" onClick={() => set_show_detail(show_detail + 1)}>
          详细信息
        </Button>
      </Card.Footer>
      <TargetDetailCard open_dialog={show_detail} target_info={props.target_info} in_updating={in_updating} on_choice_maken={props.on_choice_maken}
      in_manage={props.in_manage}/>
    </Card>
  )
}

TargetSmallCardHori.defaultProps={
  on_choice_maken: null,
  in_manage: false,
}

export default TargetSmallCardHori;