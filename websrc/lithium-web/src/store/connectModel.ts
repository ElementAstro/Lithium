import { Action, action, thunk, Thunk } from 'easy-peasy';
import { ConnectDevice, GetCurrentDeviceProfile, getDeviceBrand, getDeviceList, postDeviceStatus, postStartDeviceServer } from '../services/api';
import { postGLobalParameterOnStart } from '../services/global_parameter_api';

interface DeviceSelection {
    device_name: string;
    device_driver_name: string;
    device_driver_exec: string;
}

type AllowedDeviceTypes = 'telescope' | 'camera' | 'focus' | 'filter' | 'polar';
let deviceType: AllowedDeviceTypes = 'telescope';

const SUPPORTED_DEVICE = {
    camera: ["indi_simulator_ccd", "indi_asi_ccd", "indi_qhy_ccd", "indi_toupcam_ccd", "indi_nikon_ccd", "indi_canon_ccd", "indi_sony_ccd"],
    telescope: ["indi_lx200gotonova", "indi_lx200zeq25", "indi_ieq_telescope", 
                "indi_ioptronv3_telescope", "indi_eqmod_telescope", 
                "indi_skywatcherAltAzMount", "indi_synscan_telescope", 
                "indi_azgti_telescope", "indi_celestron_gps", "indi_celestron_aux", 
                "indi_lx200_OnStep", "indi_simulator_telescope", "indi_lx200am5"],
    focus: ["indi_asi_focuser", "indi_simulator_focus", "indi_lx200_OnStep"],
    filter: ["indi_simulator_wheel", "indi_qhycfw1_wheel", "indi_qhycfw2_wheel", "indi_qhycfw3_wheel", "indi_manual_wheel", "indi_asi_wheel"],
    polar: ["indi_v4l2_ccd", "indi_qhy_ccd"],
}

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
    config_name: string;

    // 用户选择的brand
    brand_selections: any | null;
    device_selections: any | null;

    setState: Action<ConnectModel, Partial<ConnectModel>>;
    setSelectBrand: Action<ConnectModel, { type: string; brand_name: string | null }>;
    setSelectDevice: Action<ConnectModel, { type: string; device_infor: any }>;
    resetDeviceSelections: Action<ConnectModel>;
    loadNewConfig: Action<ConnectModel>;

    // url请求
    getBrandList: Thunk<ConnectModel>;
    setBrandList: Action<ConnectModel, any[]>;
    getDeviceList: Thunk<ConnectModel>;
    setDeviceList: Action<ConnectModel, [DeviceSelection]>;
    getProfileList: Thunk<ConnectModel>;
    getProfileLDevice: Thunk<ConnectModel>;

    // 注意，前端维护用户userConfigList，但是这个应该要用到localStorage等方法
    connectDeviceServer: Thunk<ConnectModel>;
    connectDevice: Thunk<ConnectModel>;
}

export const getConnectModel = (): ConnectModel => ({
    brand_list: null,
    brand_selections: null,
    allDrivers: [],
    device_list: null,
    device_selections: null,
    user_config_list: [],
    open_dialog: false,
    brand_selection_over: false,
    setting_mode: 1,
    alreadyHaveProfile: false,
    config_name: "名称",
    brand_type_cn: ['赤道仪', '相机', '电调焦', '滤镜轮', '极轴镜'],
    brand_type_en: ['telescope', 'camera', 'focus', 'filter', 'polar'],
    current_profile: "",

    // 0:未选中，1:连接失败, 2:连接成功
    brand_connection: {
        telescope: 0,
        camera: 0,
        focus: 0,
        filter: 0,
        polar: 0,
    },

    setState: action((state, payload) => {
        state = Object.assign(state, payload)
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
                }
            }
            else {
                state.device_selections[type] = device_infor;
            }
        }
        state.brand_connection = { ...state.brand_connection, [type]: 0 };
    }),

    loadNewConfig: action((state) => {
        if (state.device_selections === null) {
            state.device_selections = {
                name: "",
                telescope: {
                    device_name: "",
                    device_driver_name: "",
                    device_driver_exec: "",
                },
                camera: {
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
                guider: {
                    device_name: "",
                    device_driver_name: "",
                    device_driver_exec: "",
                },
                polar: {
                    device_name: "",
                    device_driver_name: "",
                    device_driver_exec: "",
                },
            }
        }
        state.device_selections.name = state.config_name;
        state.user_config_list.push(state.device_selections);
    }),

    getProfileList: thunk(async (state) => {
        const res = await postGLobalParameterOnStart();

        // 储存当前的profile
        
    }),

    getProfileLDevice: thunk(async (state, payload, { getState }) => {
        const res = await GetCurrentDeviceProfile();
        if (res.data['all_drivers'] != null) {
            // 跳转到设备连接界面，并且不允许用户更改
            state.setState({ setting_mode: 1, alreadyHaveProfile: true });
            // 启动设备
            let deviceServerConnectReady = await postStartDeviceServer(res.data['all_drivers']);
            // 如果顺利
            if (deviceServerConnectReady) {
                // await Sleep(2000);
                await state.getDeviceList();
                state.setState({
                    brand_selection_over: true,
                })
            }
            else {
                console.error("获取device list失败")
            }
            // 预加载
            for (deviceType of getState().brand_type_en) {
                if (res.data[deviceType] != null) {
                    state.setSelectDevice({
                        type: deviceType, device_infor: {
                            device_name: res.data[deviceType].device_name,
                            device_driver_name: "",
                            device_driver_exec: "",
                        }
                    })
                }
            }
        }
    }),

    getBrandList: thunk(async (state, payload, { getState }) => {
        if (getState().brand_list === null) {
            const res = await getDeviceBrand();
            state.setBrandList(res.data);
        }
    }),

    setBrandList: action((state, payload) => {
        state.brand_list = payload;
    }),

    getDeviceList: thunk(async (state) => {
        try {
            const res = await getDeviceList();
            // 对response.data进行预处理
            const filteredData = res.data.filter((item: { device_name: string; }) => item.device_name !== '');
            // 加载失败
            if (filteredData.length == 0) {
                alert("扫描设备列表为空,请检查品牌选择是否正确")
            }
            // 加载成功
            else {
                state.setDeviceList(filteredData);
                state.setState({ setting_mode: 1 });
            }
            
        } catch (error) {
            // Handle any errors that may occur during the API call
            console.error('Error fetching device list:', error);
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
        };
    
        // 遍历传入的 payload 数组
        payload.forEach((device: DeviceSelection) => {
            const driverName = device.device_driver_exec;
            // 遍历 SUPPORTED_DEVICE 中的每个设备类型
            for (const deviceType in SUPPORTED_DEVICE) {
                if (Object.prototype.hasOwnProperty.call(SUPPORTED_DEVICE, deviceType)) {
                    const supportedDrivers = SUPPORTED_DEVICE[deviceType as keyof typeof SUPPORTED_DEVICE];
                    // 如果 driverName 在支持的设备类型中，则将设备名称添加到相应类型的数组中
                    if (supportedDrivers.includes(driverName)) {
                        // 将设备名称添加到相应类型的数组中
                        categorizedDevices[deviceType].push(device);
                    }
                }
            }
        });
    
        // 将分类后的设备列表存储在 state.device_list 中
        state.device_list = categorizedDevices;
    }),

    resetDeviceSelections: action((state) => {
        state.device_selections = {
            name: "",
            telescope: {
                device_name: "",
                device_driver_name: "",
                device_driver_exec: "",
            },
            camera: {
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
            guider: {
                device_name: "",
                device_driver_name: "",
                device_driver_exec: "",
            },
            polar: {
                device_name: "",
                device_driver_name: "",
                device_driver_exec: "",
            },
        },
            state.brand_connection = {
                telescope: 0,
                camera: 0,
                focus: 0,
                filter: 0,
                polar: 0,
            }
    }),

    connectDeviceServer: thunk(async (state, payload, { getState }) => {
        let postBrandData: string[] = []
        if (getState().brand_selections === null) {
            alert("当前尚未选择品牌，请选择！");
        }
        else {
            state.resetDeviceSelections();
            Object.entries(getState().brand_selections as Record<string, string>).forEach(async ([device, values]) => {
                if (values != "") {
                    postBrandData.push(values)
                }
            });
            const res = await postStartDeviceServer(postBrandData);
            return res;
        }
    }),

    // 正式连接，一个一个连，生成连接状态并且即时返回给用户
    connectDevice: thunk(async (state, payload, { getState }) => {
        if (getState().device_selections === null) {
            alert("当前尚未选择设备，请选择！");
        }
        else {
            Object.entries(getState().device_selections as Record<string, DeviceSelection>).forEach(async ([device, values]) => {
                if (device !== "name") {
                    if (values.device_name != "") {
                        // 等待设备连接完毕
                        await ConnectDevice('start', device as AllowedDeviceTypes, values.device_name);
                        // 查询连接情况
                        const res = await postDeviceStatus({
                            'device': device
                        });
                        // 回显
                        if (res) {
                            state.setState({ brand_connection: { ...getState().brand_connection, [device]: 2 } });
                        } else {
                            state.setState({ brand_connection: { ...getState().brand_connection, [device]: 1 } });
                        }
                    }
                    else {
                        state.setState({ brand_connection: { ...getState().brand_connection, [device]: 1 } });
                    }
                }
            });
        }
    }),
})






