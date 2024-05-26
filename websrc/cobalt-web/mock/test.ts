// 测试配置用的mock数据

// 数据1：brand相关，用户进到页面的时候自动向后端请求brand列表数据并渲染
// 同时，为了保证数据一致性，后续的数据post也按照这个格式
const brandList =
    {
        "camera": [
            {
                "zh": "相机品牌1",
                "en": "camera_brand1",
                "driver": "camera_driver1"
            },
            {
                "zh": "相机品牌2",
                "en": "camera_brand2",
                "driver": "camera_driver2"
            },
            {
                "zh": "相机品牌3",
                "en": "camera_brand3",
                "driver": "camera_driver3"
            }
        ],
        "telescope": [
            {
                "zh": "赤道仪品牌1",
                "en": "telescope_brand1",
                "driver": "telescope_driver1"
            },
            {
                "zh": "赤道仪品牌2",
                "en": "telescope_brand2",
                "driver": "telescope_driver2"
            }
        ],
        "focus": [
            {
                "zh": "电调焦品牌1",
                "en": "focus_brand1",
                "driver": "focus_driver1"
            },
            {
                "zh": "电调焦品牌2",
                "en": "focus_brand2",
                "driver": "focus_driver2"
            },
        ],
        "filter": [
            {
                "zh": "滤镜轮品牌1",
                "en": "filter_brand1",
                "driver": "filter_driver1"
            },
            {
                "zh": "滤镜轮品牌2",
                "en": "filter_brand2",
                "driver": "filter_driver2"
            },
        ],
        "guider": [
            {
                "zh": "导航仪品牌1",
                "en": "guider_brand1",
                "driver": "guider_driver1"
            },
            {
                "zh": "导航仪品牌2",
                "en": "guider_brand2",
                "driver": "guider_driver2"
            },
        ],
        "polar": [
            {
                "zh": "极轴镜品牌1",
                "en": "polar_brand1",
                "driver": "polar_driver1"
            },
            {
                "zh": "极轴镜品牌2",
                "en": "polar_brand2",
                "driver": "polar_driver2"
            },
        ],
      }
;

// 设备配置参数，在选定brand后隔1-2s向后端请求
const deviceList= [
    {
        device_name: 'telescope1',
        device_driver_name: 'telescope_driver1',
        device_driver_exec: 'telescope_exec1',
    },{
        device_name: 'telescope2',
        device_driver_name: 'telescope_driver2',
        device_driver_exec: 'telescope_exec2',
    },{
        device_name: 'camera1',
        device_driver_name: 'camera_driver1',
        device_driver_exec: 'camera_exec1',
    },{
        device_name: 'guider1',
        device_driver_name: 'guider_driver1',
        device_driver_exec: 'guider_exec1',
    },{
        device_name: 'focus1',
        device_driver_name: 'focus_driver1',
        device_driver_exec: 'focus_exec1',
    },{
        device_name: 'filter1',
        device_driver_name: 'filter_driver1',
        device_driver_exec: 'filter_exec1',
    },{
        device_name: 'polar1',
        device_driver_name: 'polar_driver1',
        device_driver_exec: 'polar_exec1',
    },

];

export default[
    {
        url: '/api/driver_connect/device_brand',
        type: 'get',
        response() {
            return {
                code: 200,
                msg: 'success',
                data: {
                    brandList,
                }
            };
        }
    },
    {
        url: '/api/driver_connect/device_list/',
        type: 'get',
        response() {
            return {
                code: 200,
                msg: 'success',
                data: {
                    deviceList
                }
            };
        }
    },
];
