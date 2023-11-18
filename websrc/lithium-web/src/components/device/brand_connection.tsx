// 容器/组件相关
import {Box,InputLabel,MenuItem,FormControl,Select,SelectChangeEvent,Divider, Grid, Button} from '@mui/material';

// ICON
import {Delete, Send} from '@mui/icons-material';

// use系列方法
import { useEffect } from 'react';

import { GlobalStore } from '../../store/globalStore';


const BrandConnection = () =>{

    const {brand_list, brand_type_cn, brand_type_en, brand_selections} = GlobalStore.useAppState(state=>state.connect);


    // 前后端交互部分
    // 页面加载时，调用getBrandList获取设备品牌信息
    useEffect(() => {
        GlobalStore.actions.connect.getBrandList();
    }, []);

    // 用户完成初次连接后，调用getDeviceList获取设备信息
    const handleConnectBrand = async () => {
        let deviceServerConnectReady = GlobalStore.actions.connect.connectDeviceServer()

        // 如果顺利
        if (deviceServerConnectReady){
            // await Sleep(2000);
            GlobalStore.actions.connect.getDeviceList();
            GlobalStore.actions.connect.setState({
                brand_selection_over: true,
            })
        }
        else{
            console.error("品牌连接失败")
        }
    };

    const handleBrandChange = (event: SelectChangeEvent, index: number) => {
        if(brand_selections === null){
            handleResetBrandSelections();
        }
        const value = event.target.value;
        const deviceType = brand_type_en[index];
        GlobalStore.actions.connect.setSelectBrand({type: deviceType, brand_name: value})
    };

    const handleResetBrandSelections = () => {
        GlobalStore.actions.connect.setState({
            brand_selections: {
                camera: "",
                telescope: "",
                focus: "",
                filter: "",
                guider: "",
                polar: "",
            }
        })
    };

    return(
        <Box sx={{ flexGrow: 1 }}>
            {/* 
                grid 网格设置参数：
                container spacing：列间距
                Grid xs = {?}， 一个组件占据的区域大小，?代表 ?/12 的比重，例如 6/12 是一半区域
            */}
            <Grid container spacing={1}>
                {brand_type_cn.map((item: string, type_index: number) => (
                    <Grid item xs={6} key={type_index}>
                        <FormControl  sx={{ m: 1, width: "100%" }}>
                            <InputLabel>{item}品牌</InputLabel>
                            <Select 
                                label="滤镜轮品牌"
                                value={
                                    brand_selections !== null
                                    ? brand_selections[brand_type_en[type_index] as keyof typeof brand_selections]
                                    : ""
                                } 
                                onChange={(e) => handleBrandChange(e, type_index)}>
                                <MenuItem value="">
                                    <em>空</em>
                                </MenuItem>
                                {brand_list !== null ? (
                                    brand_list[brand_type_en[type_index] as keyof typeof brand_selections].map((item: any, index: number) => (
                                        <MenuItem key={index} value={item}>{item.zh}</MenuItem>
                                    ))
                                ) : null}
                            </Select>
                        </FormControl>
                    </Grid>
                ))}
            </Grid>
            <Divider variant="middle" sx={{ my: 2}}/>
            {/* 按钮组 */}
            <Grid container spacing={1}>
                <Grid item xs={6}>
                    <Button variant="outlined" startIcon={<Delete />} sx={{ m: 1, width: "100%"}}
                        // 重置信息
                        onClick={() => {
                            handleResetBrandSelections();
                        }}
                    >
                        重置
                    </Button>
                </Grid>
                <Grid item xs={6}>
                    <Button variant="contained" endIcon={<Send />} sx={{ m: 1, width: "100%"}}
                        // 保存这个配置
                        onClick={async () => {
                            handleConnectBrand();
                        }}
                    >
                        初始化配置
                    </Button>
                </Grid>
            </Grid>
        </Box>
    )
}
export default BrandConnection;