import { Action, action, thunk, Thunk } from "easy-peasy";
import {
  ConnectDevice,
  GetCurrentDeviceProfile,
  getDeviceBrand,
  getDeviceList,
  postDeviceStatus,
  postStartDeviceServer,
} from "../services/api";
import { postGLobalParameterOnStart } from "../services/global_parameter_api";

interface DeviceSelection {
  device_name: string;
  device_driver_name: string;
  device_driver_exec: string;
}

interface Phd2Config {
  name: string;
  camera: string;
  camera_ccd: string;
  pixel_size: number;
  focal_length: number;
  telescope: string;
  mass_change_threshold: number;
  mass_change_flag: boolean;
  calibration_duration: number;
  calibration_distance: number;
}

type AllowedDeviceTypes = "telescope" | "camera" | "focus" | "filter" | "polar";
let deviceType: AllowedDeviceTypes = "telescope";

export interface ConnectModel {
  // 当前的profile
  current_profile: string;
  // 所有的驱动器
  allDrivers: { driver: string }[];

  // 所有的brand
  brand_list: any | null;
  // 所有的device
  device_list: any | null;
  // 所有的brand类别
  brand_type_en: any;
  brand_type_cn: any;
  brand_connection: any;
  // 所有的用户配置
  user_config_list: any | null;
  // panel选择
  setting_mode: number;
  // 是否打开dialog
  open_dialog: boolean;
  // 是否已经有配置可以加载
  alreadyHaveProfile: boolean;
  // 是否完成brand选择
  brand_selection_over: boolean;
  // 配置的名字输入与保存
  config_name: string | null;

  phd2_config: Phd2Config;

  phd2_connect_ready: boolean;
  already_connect: boolean;

  all_profiles: [];

  // 用户选择的brand
  brand_selections: any | null;
  device_selections: any | null;

  setState: Action<ConnectModel, Partial<ConnectModel>>;
  setSelectBrand: Action<
    ConnectModel,
    { type: string; brand_name: string | null }
  >;
  setSelectDevice: Action<ConnectModel, { type: string; device_infor: any }>;
  resetDeviceSelections: Action<ConnectModel>;

  // url请求
  getBrandList: Thunk<ConnectModel>;
  setBrandList: Action<ConnectModel, any[]>;
  getDeviceList: Thunk<ConnectModel>;
  setDeviceList: Action<ConnectModel, [DeviceSelection]>;
  getProfileList: Thunk<ConnectModel>;
  getProfileDevice: Thunk<ConnectModel>;
  setProfile: Thunk<ConnectModel, string>;
  deleteProfile: Thunk<ConnectModel, string>;

  checkPhd2Profile: Thunk<ConnectModel, any>;
  startPhd2: Thunk<ConnectModel>;

  // 注意，前端维护用户userConfigList，但是这个应该要用到localStorage等方法
  connectDeviceServer: Thunk<ConnectModel>;
  connectDevice: Thunk<ConnectModel>;
}

export const getConnectModel = (): ConnectModel => ({
  brand_list: null,
  brand_selections: {
    camera: "",
    telescope: "",
    guider: "",
    focus: "",
    filter: "",
    polar: "",
  },
  allDrivers: [],
  device_list: null,
  device_selections: {
    camera: "",
    telescope: "",
    guider: "",
    focus: "",
    filter: "",
    polar: "",
  },
  all_profiles: [],
  user_config_list: [],
  open_dialog: false,
  brand_selection_over: false,
  setting_mode: 1,
  alreadyHaveProfile: false,
  config_name: null,
  already_connect: false,
  brand_type_cn: ["赤道仪", "相机", "导星相机", "电调焦", "滤镜轮", "极轴镜"],
  brand_type_en: ["telescope", "camera", "guider", "focus", "filter", "polar"],
  current_profile: "",
  phd2_config: {
    name: "",
    camera: "",
    camera_ccd: "1",
    pixel_size: 0,
    telescope: "",
    focal_length: 0,
    mass_change_threshold: 0.5,
    mass_change_flag: false,
    calibration_duration: 0,
    calibration_distance: 0,
  },
  phd2_connect_ready: false,

  // 0:未选中，1:连接失败, 2:连接成功
  brand_connection: {
    telescope: 0,
    camera: 0,
    guider: 0,
    focus: 0,
    filter: 0,
    polar: 0,
  },

  setState: action((state, payload) => {
    state = Object.assign(state, payload);
  }),

  setSelectBrand: action((state, payload) => {
    const { type, brand_name } = payload;
    if (state.brand_selections !== null) {
      state.brand_selections[type] = brand_name;
    }
  }),

  setSelectDevice: action((state, payload) => {
    const { type, device_infor } = payload;
    if (state.device_selections !== null) {
      if (device_infor == null) {
        state.device_selections[type] = {
          device_name: "",
          device_driver_name: "",
          device_driver_exec: "",
        };
      } else {
        state.device_selections[type] = device_infor;
        if (type === "guider") {
          state.phd2_config["camera"] = device_infor.device_driver_exec;
        }
        if (type === "telescope") {
          state.phd2_config[type] = device_infor.device_driver_exec;
        }
      }
    }
    state.brand_connection = { ...state.brand_connection, [type]: 0 };
  }),

  getProfileList: thunk(async (state) => {
    const res = await postGLobalParameterOnStart();
    if (res) {
      state.setState({
        user_config_list: res.data.all_profiles,
        config_name: res.data.current_profile,
      });
    }
  }),

  getProfileDevice: thunk(async (state, payload, { getState }) => {
    const res = await GetCurrentDeviceProfile();
    if (res.data["all_drivers"] != null) {
      // 跳转到设备连接界面，并且不允许用户更改
      state.setState({ setting_mode: 1, alreadyHaveProfile: true });
      let deviceAlreadyReady = await getDriverServerStatus();
      if (deviceAlreadyReady.data != true) {
        // 重新启动设备
        await postStartDeviceServer(res.data["all_drivers"]);
      }
      await state.getBrandList();
      await state.getDeviceList();
      state.setState({
        brand_selection_over: true,
      });

      // 预加载
      for (deviceType of getState().brand_type_en) {
        if (res.data[deviceType] != null) {
          let device_name = res.data[deviceType].device_name;
          if (
            !res.data[deviceType].device_name ||
            res.data[deviceType].device_name == "undefined"
          ) {
            device_name = null;
          }
          state.setSelectDevice({
            type: deviceType,
            device_infor: {
              device_name: device_name,
              device_driver_name: "",
              device_driver_exec: "",
            },
          });
        }
      }
    }
  }),

  setProfile: thunk(async (state, payload, { getState }) => {
    await postGlobalLoadProfile(payload);

    // 调用一次重新获取
    await state.getProfileList();

    // 更新当前页面
    await state.getProfileDevice();
  }),

  deleteProfile: thunk(async (state, payload, { getState }) => {
    await postGLobalParameterProfileDelete(payload);

    // 调用一次重新获取
    await state.getProfileList();

    state.setState({
      config_name: null,
      device_selections: {
        camera: "",
        telescope: "",
        guider: "",
        focus: "",
        filter: "",
        polar: "",
      },
    });
  }),

  getBrandList: thunk(async (state, payload, { getState }) => {
    if (getState().brand_list === null) {
      const res = await getDeviceBrand();
      state.setBrandList(res.data);
    }
  }),

  setBrandList: action((state, payload: any) => {
    payload["guider"] = payload["camera"];
    state.brand_list = payload;
  }),

  getDeviceList: thunk(async (state) => {
    try {
      const res = await getDeviceList();
      // 对response.data进行预处理
      const filteredData = res.data.filter(
        (item: { device_name: string }) => item.device_name !== ""
      );
      // 加载失败
      if (filteredData.length == 0) {
        return false;
      }
      // 加载成功
      else {
        state.setDeviceList(filteredData);
        state.setState({ setting_mode: 1 });
        return true;
      }
    } catch (error) {
      // Handle any errors that may occur during the API call
      console.error("Error fetching device list:", error);
      return false;
    }
  }),

  setDeviceList: action((state, payload: [DeviceSelection]) => {
    // 初始化一个空的对象，用于存储分类后的设备列表
    const categorizedDevices: Record<string, DeviceSelection[]> = {
      camera: [],
      telescope: [],
      focus: [],
      filter: [],
      polar: [],
      guider: [],
    };

    // 遍历传入的 payload 数组
    let SUPPORTED_DEVICE = JSON.parse(JSON.stringify(state.brand_list));
    payload.forEach((device: DeviceSelection) => {
      const driverName = device.device_driver_exec;
      let flag = true;
      // 遍历 SUPPORTED_DEVICE 中的每个设备类型
      for (const deviceType in SUPPORTED_DEVICE) {
        if (
          Object.prototype.hasOwnProperty.call(SUPPORTED_DEVICE, deviceType)
        ) {
          const supportedDrivers =
            SUPPORTED_DEVICE[deviceType as keyof typeof SUPPORTED_DEVICE];
          // 如果 driverName 在支持的设备类型中，则将设备名称添加到相应类型的数组中
          if (
            supportedDrivers.some(
              (driver: { driver: string }) => driver.driver === driverName
            )
          ) {
            // 将设备名称添加到相应类型的数组中
            categorizedDevices[deviceType].push(device);
            flag = false;
          }
        }
      }
      if (flag) {
        categorizedDevices["guider"].push(device);
        categorizedDevices["camera"].push(device);
      }
    });
    // 将分类后的设备列表存储在 state.device_list 中
    state.device_list = categorizedDevices;
  }),

  resetDeviceSelections: action((state) => {
    state.device_selections = {
      camera: "",
      telescope: "",
      guider: "",
      focus: "",
      filter: "",
      polar: "",
    };
    state.brand_connection = {
      telescope: 0,
      camera: 0,
      guider: 0,
      focus: 0,
      filter: 0,
      polar: 0,
    };
    state.config_name = null;
  }),

  checkPhd2Profile: thunk(async (state, payload, { getState }) => {
    try {
      let props = {
        camera: payload.camera,
        telescope: payload.telescope,
      };
      const res = await postCheckPhd2(props);
      return res.data;
    } catch (error) {
      return false;
    }
  }),

  connectDeviceServer: thunk(async (state, payload, { getState }) => {
    let postBrandData: string[] = [];
    if (getState().brand_selections === null) {
      alert("当前尚未选择品牌，请选择！");
    } else {
      state.resetDeviceSelections();
      Object.entries(
        getState().brand_selections as Record<string, string>
      ).forEach(async ([device, values]) => {
        if (values != "") {
          postBrandData.push(values);
        }
      });
      const res = await postStartDeviceServer(postBrandData);
      return res;
    }
  }),

  // 正式连接，一个一个连，生成连接状态并且即时返回给用户
  connectDevice: thunk(async (state, payload, { getState }) => {
    if (getState().device_selections === null) {
      alert("当前尚未选择设备");
    } else {
      Object.entries(
        getState().device_selections as Record<string, DeviceSelection>
      ).forEach(async ([device, values]) => {
        if (values.device_name && values.device_name != "") {
          // 等待设备连接完毕
          await ConnectDevice(
            "start",
            device as AllowedDeviceTypes,
            values.device_name
          );
          // 查询连接情况
          try {
            const res = await postDeviceStatus({
              device: device,
            });
            // 回显
            if (res && res.data === "Connected") {
              state.setState({
                brand_connection: {
                  ...getState().brand_connection,
                  [device]: 2,
                },
              });
            } else {
              state.setState({
                brand_connection: {
                  ...getState().brand_connection,
                  [device]: 1,
                },
              });
            }
          } catch (error) {
            state.setState({
              brand_connection: { ...getState().brand_connection, [device]: 1 },
            });
          }
        } else {
          state.setState({
            brand_connection: { ...getState().brand_connection, [device]: 1 },
          });
        }
        // 避免用户连续点击
        state.setState({
          already_connect: true,
        });
      });
    }
    // 检查是否可以启动phd2
    let selections = getState().device_selections;
    const phd2_check_ok = await postCheckPhd2({
      camera: selections["camera"].device_name,
      telescope: selections["telescope"].device_name,
    });
    if (phd2_check_ok) {
      state.setState({
        phd2_connect_ready: true,
      });
    }
  }),

  startPhd2: thunk(async (state) => {
    try {
      const res = await postStartPhd2();
      if (res.data == true) {
        await Sleep(1000);
        const res2 = await postConnectPhd2();
        if (res2.data === true) {
          return true;
        }
      }
      return false;
    } catch (error) {
      return false;
    }
  }),
});

const Sleep = (ms: number) => {
  return new Promise((resolve) => setTimeout(resolve, ms));
};
