import * as React from 'react';

// 容器/组件相关
import {Tabs, Tab, Box} from '@mui/material';

// use系列方法
import { useEffect } from 'react';

import { GlobalStore } from '../../store/globalStore';

// 子组件导入
import BrandConnection from './brand_connection';
import DeviceConnection from './device_connection';


// TabPanel接口
interface TabPanelProps {
    children?: React.ReactNode;
    index: number;
    value: number;
  }

// TabPanel组件
function TabPanel(props: TabPanelProps) {
    // TabPanel获取三个内容：Children（子组件）；value（当前选中组件index）；index（组件本身对应index）
    const { children, value, index , ...other} = props;
    return (
        <div role="tabpanel" hidden={value !== index}  {...other}>
        {value === index && (
            <Box sx={{ p: 2 }}>
                {children}
            </Box>
        )}
        </div>
    );
}

const Connect = ()=>{
// UseAppState 部分
    // brand_list 后端传递的brand数据
    const {setting_mode, brand_selection_over} = GlobalStore.useAppState(state=>state.connect);


// 前后端交互部分
    // 页面加载时，调用getBrandList获取设备品牌信息
    useEffect(()=>{
        async function fetchData() {
            // 获取所有的profile
            await GlobalStore.actions.connect.getProfileList();
            // 加载全局变量
            await GlobalStore.actions.GlobalParameterStore.get_all_paramters();
            // 获取配置信息
            await GlobalStore.actions.connect.getProfileLDevice();
        }
        fetchData();
    },[])


    // 前端内部的其他 handle 部分
    const handleModeChange = (event: React.SyntheticEvent, newMode: number) => {
        GlobalStore.actions.connect.setState({
            setting_mode: newMode
        })
    };

    // html部分
    return (
        <Box sx={{ width: '100%' }}>
            <Box sx={{ borderBottom: 1, borderColor: 'divider', backgroundColor: '#E6E6E6'}}>
                <Tabs value={setting_mode} onChange={handleModeChange} variant="fullWidth">
                    <Tab label="初次连接设置"/>
                    <Tab label="按照配置连接选项" disabled={!brand_selection_over}/>
                </Tabs>
            </Box>
            <TabPanel value={setting_mode} index={0}>
                <BrandConnection />
            </TabPanel>
            <TabPanel value={setting_mode} index={1}>
                <DeviceConnection />
            </TabPanel>
        </Box>
    )
}

export default Connect;