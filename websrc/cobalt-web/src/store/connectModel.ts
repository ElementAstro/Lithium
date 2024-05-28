import { Action, action, thunk, Thunk } from "easy-peasy";
import {
  ConnectDevice,
  GetCurrentDeviceProfile,
  getDeviceBrand,
  getDeviceList,
  getDriverServerStatus,
  postCheckPhd2,
  postCloseDeviceServer,
  postConnectPhd2,
  postDeviceStatus,
  postStartDeviceServer,
  postStartPhd2,
} from "@/services/api";
import {
  postGlobalLoadProfile,
  postGLobalParameterOnStart,
  postGLobalParameterProfileDelete,
} from "@/services/global_parameter_api";

interface DeviceInfo {
  [key: string]: Array<ConnectionDeviceInfo>;
}
interface DeviceSelection {
  [key: string]: ConnectionDeviceInfo;
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
interface BrandConnection {
  [key: string]: number;
}
interface profileInfo {
  userConfigList: Array<string>;
  configName: string;
}

type AllowedDeviceTypes =
  | "telescope"
  | "camera"
  | "focus"
  | "filter"
  | "guider";
type AllowedDeviceTypesCN =
  | "赤道仪"
  | "相机"
  | "导星相机"
  | "电调焦"
  | "滤镜轮";
let deviceType: AllowedDeviceTypes = "telescope";

export interface ConnectModel {
  // 当前的profile
  current_profile: string;
  // 所有的驱动器
  allDrivers: { driver: string }[];

  // 所有的brand
  brand_list: IConnectBrandList;
  // 所有的device
  device_list: DeviceInfo;
  // 所有的brand类别
  brand_type_en: Array<AllowedDeviceTypes>;
  brand_type_cn: Array<AllowedDeviceTypesCN>;
  brand_connection: BrandConnection;
  // 所有的用户配置
  user_config_list: Array<string>;
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
  is_connect: boolean;

  all_profiles: [];

  // 用户选择的brand
  brand_selections: IConnectSelectedBrandList;
  device_selections: DeviceSelection;

  setState: Action<ConnectModel, Partial<ConnectModel>>;
  setSelectBrand: Action<
    ConnectModel,
    { type: AllowedDeviceTypes; brand_info: IConnectBrandSelection }
  >;
  setSelectDevice: Action<
    ConnectModel,
    { type: string; device_infor: ConnectionDeviceInfo }
  >;
  resetDeviceSelections: Action<ConnectModel>;

  // url请求
  getBrandList: Thunk<ConnectModel>;
  setBrandList: Action<ConnectModel, IConnectBrandList>;
  getDeviceList: Thunk<ConnectModel>;
  setDeviceList: Action<ConnectModel, Array<ConnectionDeviceInfo>>;
  getProfileList: Thunk<ConnectModel>;
  setProfileList: Action<ConnectModel, profileInfo>;
  getProfileDevice: Thunk<ConnectModel>;

  // 新建和加载合用
  setProfile: Thunk<ConnectModel, string>;
  deleteProfile: Thunk<ConnectModel, string>;

  checkPhd2Profile: Thunk<ConnectModel>;
  checkPhd2Device: Thunk<ConnectModel>;
  startPhd2: Thunk<ConnectModel>;

  connectDeviceServer: Thunk<ConnectModel>;
  closeDeviceServer: Thunk<ConnectModel>;
  connectDevice: Thunk<ConnectModel>;
  checkDeviceConnection: Thunk<ConnectModel, { device: string }>;
  // 用于一口气刷新所有device的connect状态的
  refreshDeviceConnectionStatus: Thunk<ConnectModel>;
  setBrandInLoading: Action<ConnectModel, { device: string; flag: boolean }>;
}

export const getConnectModel = (): ConnectModel => ({
  brand_list: {
    camera: [],
    telescope: [],
    guider: [],
    focus: [],
    filter: [],
  },
  brand_selections: {
    camera: { zh: "", en: "", driver: "" },
    telescope: { zh: "", en: "", driver: "" },
    guider: { zh: "", en: "", driver: "" },
    focus: { zh: "", en: "", driver: "" },
    filter: { zh: "", en: "", driver: "" },
  },
  allDrivers: [],
  device_list: {
    camera: [],
    telescope: [],
    guider: [],
    focus: [],
    filter: [],
  },
  device_selections: {
    camera: { device_name: "", device_driver_name: "", device_driver_exec: "" },
    telescope: {
      device_name: "",
      device_driver_name: "",
      device_driver_exec: "",
    },
    guider: { device_name: "", device_driver_name: "", device_driver_exec: "" },
    focus: { device_name: "", device_driver_name: "", device_driver_exec: "" },
    filter: { device_name: "", device_driver_name: "", device_driver_exec: "" },
  },
  all_profiles: [],
  user_config_list: [],
  open_dialog: false,
  brand_selection_over: false,
  setting_mode: 1,
  alreadyHaveProfile: false,
  config_name: "",
  already_connect: false,
  brand_type_cn: ["赤道仪", "相机", "导星相机", "电调焦", "滤镜轮"],
  brand_type_en: ["telescope", "camera", "guider", "focus", "filter"],
  current_profile: "",
  phd2_config: {
    name: "",
    camera: "",
    camera_ccd: "0",
    pixel_size: 0,
    telescope: "",
    focal_length: 0,
    mass_change_threshold: 0.5,
    mass_change_flag: false,
    calibration_duration: 0,
    calibration_distance: 0,
  },
  is_connect: false,
  phd2_connect_ready: false,

  // 0:未选中，1:连接失败, 2:连接成功，-1更新中
  brand_connection: {
    telescope: 0,
    camera: 0,
    guider: 0,
    focus: 0,
    filter: 0,
  },

  setState: action((state, payload) => {
    state = Object.assign(state, payload);
  }),

  setSelectBrand: action((state, payload) => {
    const { type, brand_info } = payload;
    if (state.brand_selections !== null) {
      state.brand_selections[type] = brand_info;
    }
  }),

  setSelectDevice: action(
    (
      state,
      payload: { type: AllowedDeviceTypes; device_infor: ConnectionDeviceInfo }
    ) => {
      const { type, device_infor } = payload;
      if (state.device_selections !== null) {
        if (device_infor == null) {
          state.device_selections[type] = {
            device_name: "",
            device_driver_name: "",
            device_driver_exec: "",
          };
        } else {
          const deviceList = JSON.parse(JSON.stringify(state.device_list));
          const deviceName = Object.keys(deviceList[type]).map(
            (deviceName) => deviceList[type][deviceName].device_name
          );
          if (deviceName.indexOf(device_infor.device_name) !== -1) {
            state.device_selections[type] = device_infor;
          } else {
            state.device_selections[type] = {
              device_name: "",
              device_driver_name: "",
              device_driver_exec: "",
            };
          }
          if (type === "guider") {
            state.phd2_config["camera"] = device_infor.device_driver_exec;
          }
          if (type === "telescope") {
            state.phd2_config[type] = device_infor.device_driver_exec;
          }
        }
      }
    }
  ),

  getProfileList: thunk(async (actions) => {
    const res = await postGLobalParameterOnStart();
    if (res) {
      actions.setProfileList({
        userConfigList: res.data.all_profiles,
        configName: res.data.current_profile,
      });
    }
  }),

  setProfileList: action((state, payload) => {
    state.user_config_list = payload.userConfigList;
    state.config_name = payload.configName;
    if (payload.userConfigList.length == 0) {
      state.setting_mode = 0;
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
            device_name = "";
          }
          state.setSelectDevice({
            type: deviceType,
            device_infor: {
              device_name: device_name,
              device_driver_name: "",
              device_driver_exec: "",
            },
          });
          for (const brandInfo of res.data["all_drivers"]) {
            if (brandInfo.en === device_name) {
              state.setSelectBrand({
                type: deviceType,
                brand_info: brandInfo,
              });
            }
          }
        }
      }
    }
  }),

  setProfile: thunk(async (state, payload, { getState }) => {
    if (payload === "") {
      state.resetDeviceSelections();
    } else {
      const res = await postGlobalLoadProfile(payload);
      if (res) {
        state.setState({
          config_name: payload,
        });
        // 新建
        if (!res.success) {
          const updatedList = [...getState().user_config_list, payload];
          state.setState({
            user_config_list: updatedList,
          });
        }
        // 更新当前页面
        await state.getProfileDevice();
      }
    }
  }),

  deleteProfile: thunk(async (state, payload, { getState }) => {
    const res = await postGLobalParameterProfileDelete(payload);
    // todo modify later: change res to res.success
    if (res) {
      // 删除对应的元素
      const updatedList = getState().user_config_list.filter(
        (item) => item !== payload
      );

      state.setState({
        user_config_list: updatedList,
      });

      // 自动更新到新的第一个
      await state.setProfile(updatedList[0]);
    }
  }),

  getBrandList: thunk(async (state, payload, { getState }) => {
    const res = await getDeviceBrand();
    state.setBrandList(res.data);
  }),

  setBrandList: action((state, payload: IConnectBrandList) => {
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
      // Handle errors that may occur during the API call
      console.error("Error fetching device list:", error);
      return false;
    }
  }),

  setDeviceList: action((state, payload: Array<ConnectionDeviceInfo>) => {
    // 初始化一个空的对象，用于存储分类后的设备列表
    const categorizedDevices: Record<string, Array<ConnectionDeviceInfo>> = {
      camera: [],
      telescope: [],
      focus: [],
      filter: [],
      guider: [],
    };

    // 遍历传入的 payload 数组
    let SUPPORTED_DEVICE = JSON.parse(JSON.stringify(state.brand_list));
    payload.forEach((device: ConnectionDeviceInfo) => {
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
      camera: {
        device_name: "",
        device_driver_name: "",
        device_driver_exec: "",
      },
      telescope: {
        device_name: "",
        device_driver_name: "",
        device_driver_exec: "",
      },
      guider: {
        device_name: "",
        device_driver_name: "",
        device_driver_exec: "",
      },
      focus: {
        device_name: "",
        device_driver_name: "",
        device_driver_exec: "",
      },
      filter: {
        device_name: "",
        device_driver_name: "",
        device_driver_exec: "",
      },
    };
    state.brand_connection = {
      telescope: 0,
      camera: 0,
      guider: 0,
      focus: 0,
      filter: 0,
    };
    state.config_name = null;
  }),

  checkPhd2Profile: thunk(async (state, payload, { getState }) => {
    try {
      let props = getState().phd2_config;
      const res = await postCheckPhd2(props);
      return res.data;
    } catch (error) {
      return false;
    }
  }),

  connectDeviceServer: thunk(async (state, payload, { getState }) => {
    let postBrandData: string[] = [];
    state.resetDeviceSelections();
    Object.entries(getState().brand_selections).forEach(
      async ([device, values]) => {
        if (values.driver != "") {
          postBrandData.push(values);
        }
      }
    );
    if (postBrandData.length == 0) {
      return false;
    }
    const res_stop = await postCloseDeviceServer();
    const res = await postStartDeviceServer(postBrandData);
    return res;
  }),

  closeDeviceServer: thunk(async (state, payload, { getState }) => {
    const res = await postCloseDeviceServer();
    return res;
  }),

  // 正式连接，一个一个连，生成连接状态并且即时返回给用户
  connectDevice: thunk(async (state, payload, { getState }) => {
    if (getState().device_selections === null) {
      alert("当前尚未选择设备");
    } else {
      state.setState({
        brand_connection: {
          telescope: 0,
          camera: 0,
          guider: 0,
          focus: 0,
          filter: 0,
        },
      });
      // 禁用button
      state.setState({
        is_connect: true,
      });
      // 创建一个 Promise 数组，用于存放每个设备连接操作的 Promise
      const connectPromises = Object.entries(
        getState().device_selections as Record<string, ConnectionDeviceInfo>
      ).map(async ([device, values]) => {
        if (values.device_name && values.device_name !== "") {
          // 等待设备连接完毕
          state.setState({
            brand_connection: { ...getState().brand_connection, [device]: -1 },
          });
          await ConnectDevice(
            "start",
            device as AllowedDeviceTypes,
            values.device_name
          );
          state.checkDeviceConnection({
            device: device,
          });
        } else {
          state.setState({
            brand_connection: { ...getState().brand_connection, [device]: 1 },
          });
        }
      });

      // 等待所有设备连接操作的 Promise 完成
      await Promise.all(connectPromises);

      state.setState({
        is_connect: false,
      });

      // 自动填入phd2并检查
      state.checkPhd2Device();
    }
  }),

  checkDeviceConnection: thunk(async (state, payload, { getState }) => {
    let check_connect = false;
    try {
      const res = await postDeviceStatus({
        device: payload.device,
      });

      if (res && res.data === "Connected") {
        check_connect = true;
        state.setState({
          brand_connection: {
            ...getState().brand_connection,
            [payload.device]: 2,
          },
        });
      } else {
        state.setState({
          brand_connection: {
            ...getState().brand_connection,
            [payload.device]: 1,
          },
        });
      }
    } catch (error) {
      state.setState({
        brand_connection: {
          ...getState().brand_connection,
          [payload.device]: 1,
        },
      });
    }
    // 如果存在一个连接了, 说明已经有连接过
    state.setState({
      already_connect: check_connect,
    });
  }),

  startPhd2: thunk(async (state) => {
    try {
      const res = await postStartPhd2();
      if (res.data.flag == true) {
        await Sleep(1000);
        const res2 = await postConnectPhd2();
        if (res2.data.flag === true) {
          return true;
        }
      }
      return false;
    } catch (error) {
      return false;
    }
  }),

  refreshDeviceConnectionStatus: thunk(async (state, payload, { getState }) => {
    let to_update_devices = [
      "camera",
      "telescope",
      "focus",
      "guider",
      "filter",
    ];
    to_update_devices.map(async (device, index) => {
      state.setBrandInLoading({ device: device, flag: true });
      try {
        const res = await postDeviceStatus({
          device: device,
        });
        // 回显
        if (res && res.data === "Connected") {
          state.setState({
            brand_connection: { ...getState().brand_connection, [device]: 2 },
          });
        } else {
          state.setState({
            brand_connection: { ...getState().brand_connection, [device]: 1 },
          });
        }
      } catch (error) {
        state.setState({
          brand_connection: { ...getState().brand_connection, [device]: 1 },
        });
      }
    });
  }),

  setBrandInLoading: action((state, payload) => {
    if (payload.flag) {
      state.brand_connection[payload.device] = -1;
    } else {
      state.brand_connection[payload.device] = 0;
    }
  }),

  checkPhd2Device: thunk(async (state, payload, { getState }) => {
    const guiderDeviceName =
      getState().device_selections["guider"].device_name || "";
    const telescopeDeviceName =
      getState().device_selections["telescope"].device_name || "";

    const phd2_check_res = await postCheckPhd2({
      camera: guiderDeviceName,
      telescope: telescopeDeviceName,
    });
    if (phd2_check_res && !phd2_check_res.data.flag) {
      alert("注意与上次配置不一致");
    }
  }),
});

const Sleep = (ms: number) => {
  return new Promise((resolve) => setTimeout(resolve, ms));
};
