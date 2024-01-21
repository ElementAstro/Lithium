import React, { useEffect, useRef } from "react";
import Accordion from "react-bootstrap/Accordion";
import Card from "react-bootstrap/Card";
import Button from "react-bootstrap/Button";
import Container from "react-bootstrap/Container";
import { ChevronDown } from "react-bootstrap-icons";
import GlobalSettingOnePartSetting from "./OnePartSettingComp";
import GlobalParameterAllFilterSettings from "./OneSettingSPFilterEntryComp";

import { GlobalStore } from "../../store/globalStore";

const GlobalParameterSettingPage = () => {
  const parameter_explain = useRef<{
    [key: string]: { name: string; tooltips: string; range?: number[] };
  }>(parameter_explain_data);
  const global_parameter = GlobalStore.useAppState(
    (state) => state.GlobalParameterStore
  );
  const get_all_global_parameters =
    GlobalStore.actions.GlobalParameterStore.get_all_paramters;

  useEffect(() => {
    console.log("test");
    get_all_global_parameters();
  }, []);

  const handleSettingChange = (
    setting_part_key: string,
    setting_key: string,
    set_value: boolean | number | string | object
  ) => {
    console.log(setting_part_key, setting_key, set_value);
    let to_change_parameter =
      global_parameter.global_parameter[setting_part_key];
    to_change_parameter[setting_key] = set_value;
    GlobalStore.actions.GlobalParameterStore.set_parameter({
      parameter_name: setting_part_key,
      to_set_parameter: to_change_parameter,
    });
  };

  const handleSettingPartChange = (
    setting_part_key: string,
    setting_value: Partial<IGlobalParameters>
  ) => {
    console.log(setting_part_key, setting_value);
    GlobalStore.actions.GlobalParameterStore.set_parameter({
      parameter_name: setting_part_key,
      to_set_parameter: setting_value,
    });
  };

  const render_single_accordion = (
    key: string,
    value: Partial<IGlobalParameters>
  ) => {
    const parameter_found = key in parameter_explain.current;
    var this_name = "";
    var this_tooltips = "";

    if (parameter_found) {
      this_name = parameter_explain.current[key].name;
      this_tooltips = parameter_explain.current[key].tooltips;
    } else {
      this_name = "* " + key;
      this_tooltips = "* " + "???";
    }

    if (value === null) {
      return <div key={key}></div>;
    } else if (
      key === "global_setting_file" ||
      key === "device_list_name" ||
      key === "info_get"
    ) {
      return <div key={key}></div>;
    } else if (key === "filter_setting") {
      return (
        <Container fluid key={key}>
          <Accordion>
            <Card>
              <Card.Header>
                <Accordion.Header as={Button} variant="link" eventKey={key}>
                  <span style={{ width: "33%", flexShrink: 0 }}>
                    {this_name}
                  </span>
                  <span style={{ color: "text.secondary" }}>
                    {this_tooltips}
                  </span>
                  <ChevronDown />
                </Accordion.Header>
              </Card.Header>
              <Accordion.Collapse eventKey={key}>
                <Card.Body>
                  <GlobalParameterAllFilterSettings
                    setting_part_name={key}
                    setting_part_value={value}
                    handleSettingChange={handleSettingPartChange}
                    parameter_explain={parameter_explain.current}
                  />
                </Card.Body>
              </Accordion.Collapse>
            </Card>
          </Accordion>
        </Container>
      );
    } else {
      return (
        <Container fluid key={key}>
          <Accordion>
            <Card>
              <Card.Header>
                <Accordion.Header as={Button} variant="link" eventKey={key}>
                  <span style={{ width: "33%", flexShrink: 0 }}>
                    {this_name}
                  </span>
                  <span style={{ color: "text.secondary" }}>
                    {this_tooltips}
                  </span>
                  <ChevronDown />
                </Accordion.Header>
              </Card.Header>
              <Accordion.Collapse eventKey={key}>
                <Card.Body>
                  <GlobalSettingOnePartSetting
                    setting_part_name={key}
                    setting_part_value={value}
                    handleSettingChange={handleSettingChange}
                    parameter_explain={parameter_explain.current}
                  />
                </Card.Body>
              </Accordion.Collapse>
            </Card>
          </Accordion>
        </Container>
      );
    }
  };

  return (
    <div className="GlobalSettingSettingPage">
      <Container fluid>
        {Object.keys(global_parameter.global_parameter).map((key) =>
          render_single_accordion(key, global_parameter.global_parameter[key])
        )}
      </Container>
    </div>
  );
};

export default GlobalParameterSettingPage;

const parameter_explain_data: IGPParameterExplain = {
  // general name
  autofocus: {
    name: "自动对焦设置",
    tooltips: "用来设置自动对焦过程中的参数",
  },
  plate_solve: {
    name: "解析设置",
    tooltips: "自动解析相关的配置",
  },
  telescope_info: {
    name: "望远镜参数",
    tooltips: "用来设置望远镜相关的光学物理参数",
  },
  camera_info: {
    name: "主相机信息",
    tooltips:
      "用来设置主相机相关的物理参数。一般情况下会自动获取，但是如果与实际值略有差异，请手动修改。",
  },
  guider_camera_info: {
    name: "导星相机信息",
    tooltips:
      "用来设置导星相机相关的物理参数。一般情况下会自动获取，但是如果与实际值略有差异，请手动修改。",
  },
  guider_start_guide_settle: {
    name: "开始导星时稳定",
    tooltips: "配置在开始导星时，导星结果稳定所需要的时间和误差大小",
  },
  guider_dither_settle: {
    name: "抖动后导星稳定",
    tooltips: "配置在拍摄时抖动之后，导星结果稳定所需要的时间和误差大小",
  },
  // geo
  geo_location: {
    name: "地理坐标信息",
    tooltips: "拍摄地点的经纬度坐标信息",
    range: [-180, 180],
    unit: "度",
  },
  longitude: {
    name: "经度坐标",
    tooltips: "拍摄地点的经度坐标，东经为正",
    range: [-180, 180],
    unit: "度",
  },
  latitude: {
    name: "纬度坐标",
    tooltips: "拍摄地点的纬度坐标",
    range: [-90, 90],
    unit: "度",
  },
  height: {
    name: "海拔高度",
    tooltips: "拍摄地点的海拔高度",
    range: [],
    unit: "米",
  },
  time_zone: {
    name: "时区",
    tooltips: "设备上时间对应的时区，东时区为正，西时区为负",
    range: [-12, 12],
  },
  // merdian
  meridian_flip: {
    name: "中天翻转",
    tooltips: "中天翻转相关的配置参数",
    range: [],
  },
  before_meridian: {
    name: "中天前停止时间",
    tooltips: "在到达中天前，提前停止拍摄和跟踪，单位分钟",
    range: [0, 1000],
    unit: "分钟",
  },
  after_meridian: {
    name: "中天后停止时间",
    tooltips: "在过了中天之后，等待多久，开始翻转和开始后续拍摄流程，单位分钟",
    range: [0, 1000],
    unit: "分钟",
  },
  recenter_after_flip: {
    name: "中天翻转后居中",
    tooltips: "在完成中天翻转后，是否进行目标居中操作（建议开启）",
    range: [],
  },
  autofocus_after_flip: {
    name: "中天翻转后自动对焦",
    tooltips: "在中天翻转完成后，是否执行一次自动对焦",
    range: [],
  },
  settle_time_after: {
    name: "中天翻转后导星稳定",
    tooltips: "在完成一次中天翻转后，开始导星后，第一次稳定导星时长，单位秒",
    range: [0, 100000],
    unit: "秒",
  },
  // single filter
  filter_setting: {
    name: "滤镜设置",
    tooltips: "设置所有滤镜相关的配置，包括在自动对焦时滤镜的参数",
  },
  filter_name: {
    name: "滤镜名称",
    tooltips: "设置滤镜的名称，一般以该滤镜类型为名",
    range: [],
  },
  focus_offset: {
    name: "滤镜对焦偏置值",
    tooltips:
      "对焦时，如果选择使用偏置值，那么在拍摄过程中切换滤镜的过程，会采用这个值对调焦器进行移动，而不需要执行一次自动对焦过程。一般情况下，这个值是通过测量得到的。",
    range: [],
  },
  af_exposure_time: {
    name: "自动对焦曝光时间倍率",
    tooltips:
      "在使用该滤镜进行自动对焦时，会采用这个倍率值乘以基准滤镜的曝光时长，对于一些窄带滤镜，这个值需要设置的大一点，不然无法检测到星点",
    range: [],
  },
  // all FilterSetting
  filter_number: {
    name: "滤镜数量",
    tooltips: "设置你的滤镜轮有多少片滤镜",
    range: [],
  },
  filter_info: {
    name: "滤镜信息",
    tooltips: "每片滤镜的信息",
    range: [],
  },
  // IAutofocus
  use_filter_offset: {
    name: "使用滤镜对焦偏置值",
    tooltips:
      "如果选择该选项，那么在拍摄过程中，切换滤镜不会引发自动对焦而是采用滤镜设置中的滤镜对焦偏置值对调焦器进行调整。如果不选择此项，那么就会采用自动对焦",
    range: [],
  },
  step_size: {
    name: "自动对焦的步长",
    tooltips: "在自动对焦的过程中，每一次移动调焦器的步长",
    range: [],
  },
  initial_steps: {
    name: "初始移动步数",
    tooltips: "在开始自动对焦时，第一次移动多少步",
    range: [],
  },
  default_exposure_time: {
    name: "默认曝光时间",
    tooltips: "默认自动对焦过程中，曝光时长，单位秒",
    range: [],
    unit: "秒",
  },
  retry_times: {
    name: "重试次数",
    tooltips:
      "如果自动对焦失败了，自动对焦一共会最多执行该次数。如果设置为1，那么失败了一次就不会再执行",
    range: [],
  },
  each_step_exposure: {
    name: "每步拍摄次数",
    tooltips: "自动对焦过程中，每一步移动调焦器，会自动拍摄的张数",
    range: [],
  },
  af_after_filter_change: {
    name: "滤镜切换后是否自动对焦",
    tooltips: "",
  },
  af_offset_base_filter_slot: {
    name: "滤镜对焦偏置基准滤镜",
    tooltips: "",
  },
  // IPlateSolve
  use: {
    name: "采用解析方式",
    tooltips: "现只支持astap，不要做修改，后续其他解析方案会逐步支持。",
    range: [],
  },
  exposure_time: {
    name: "解析曝光时间",
    tooltips: "采用解析时，自动曝光的时长，单位秒",
    range: [],
    unit: "秒",
  },
  use_filter: {
    name: "采用滤镜",
    tooltips: "在需要解析时，会采用那个滤镜，该配置选择滤镜的位置。",
    range: [],
  },
  downsample: {
    name: "降采样",
    tooltips: "",
    range: [],
  },
  // ITelescopeInfo
  name: {
    name: "望远镜配置名",
    tooltips: "给这个望远镜的配置参数起个名字",
    range: [],
  },
  aperture: {
    name: "主镜口径",
    tooltips: "单位mm",
    range: [],
    unit: "mm",
  },
  focal_length: {
    name: "主镜焦距",
    tooltips: "主镜的焦距，单位mm",
    unit: "mm",
  },
  guider_aperture: {
    name: "导星镜口径",
    tooltips: "单位mm",
    range: [],
    unit: "mm",
  },
  guider_focal_length: {
    name: "导星镜焦距",
    tooltips: "单位mm",
    range: [],
    unit: "mm",
  },
  // AnyCameraInfo
  CCD_MAX_X: {
    name: "x像素值",
    tooltips: "该相机的x方向像素值",
    range: [],
  },
  CCD_MAX_Y: {
    name: "y像素值",
    tooltips: "该相机的y方向像素值",
    range: [],
  },
  CCD_PIXEL_SIZE: {
    name: "像素大小",
    tooltips: "相机的单个像素的大小，单位微米",
    range: [],
    unit: "um",
  },
  // settle
  time: {
    name: "稳定最小时间",
    tooltips: "开始稳定后，至少过多少时间开始检测是否达到稳定的目标，单位秒",
    unit: "秒",
  },
  timeout: {
    name: "稳定超时时间",
    tooltips:
      "开始稳定后，超过这个时间还没达到稳定目标，则报错停止导星，单位秒",
    unit: "秒",
  },
  pixels: {
    name: "稳定目标像素",
    tooltips:
      "开始稳定后，平均导星误差达到这个数值以下，判断为导星系统稳定，单位为导星设备的实际像素值。",
  },
  dither: {
    name: "抖动",
    tooltips: "如果设置抖动开，导星系统会根据需求在单次曝光完成后，进行抖动",
  },
  amount: {
    name: "抖动像素量",
    tooltips: "设置抖动开后，一次抖动会移动该数值的像素值",
  },
  ra_only: {
    name: "是否只抖动赤经",
    tooltips: "如果此选项打开，那么抖动时只移动赤经轴",
  },
};
