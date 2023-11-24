import * as React from 'react';

// 容器/组件相关
import {
    InputLabel, MenuItem, FormControl, Select, SelectChangeEvent, Container,
    Dialog, DialogActions, DialogContent, DialogContentText, DialogTitle, TextField, Grid, Button
} from '@mui/material';

// ICON
import { Delete, Send, AddCircleOutlineOutlined, ErrorOutline, CheckCircleOutline } from '@mui/icons-material';
import { ListItemIcon } from '@mui/material';

import { GlobalStore } from '../../store/globalStore';

const DeviceConnection = () => {
    const { device_list, user_config_list, open_dialog,
        brand_type_cn, brand_type_en, brand_connection, device_selections } = GlobalStore.useAppState(state => state.connect);


    // 用户新建立一个配置后，将其通过loadNewConfig加载到已有配置清单中
    const handleNewSetting = () => {
        GlobalStore.actions.connect.loadNewConfig();
        // 关闭Dialog
        handleCreateClose();
    };

    // 用户完成配置时，调用connectDevice正式启动连接
    const handleConnectDevice = () => {
        GlobalStore.actions.connect.connectDevice();
    };

    const handleDeviceChange = (event: SelectChangeEvent, index: number) => {
        const value = event.target.value;
        const selectedConfig = device_list[brand_type_en[index]].find((config: { device_name: string; }) => config.device_name === value);
        const deviceType = brand_type_en[index];
        if (device_selections === null) {
            handleResetDeviceSelections();
        }
        GlobalStore.actions.connect.setSelectDevice({ type: deviceType, device_infor: selectedConfig });
    };

    const handleUserConfigLoad = (event: SelectChangeEvent) => {
        const value = event.target.value;
        const selectedConfig = user_config_list.find((config: { name: string; }) => config.name === value);
        if (selectedConfig !== undefined) {
            GlobalStore.actions.connect.setState({
                device_selections: selectedConfig,
            });
        }
    };
    // 重置
    const handleResetDeviceSelections = () => {
        GlobalStore.actions.connect.resetDeviceSelections();
    };

    const handleSettingChange = (event: any) => {
        GlobalStore.actions.connect.setState({
            config_name: event.target.value
        })
    };

    const handleCreateOpen = () => {
        GlobalStore.actions.connect.setState({
            open_dialog: true
        })
    };

    const handleCreateClose = () => {
        GlobalStore.actions.connect.setState({
            open_dialog: false
        })
    };

    //   休眠函数
    const Sleep = (ms: number) => {
        return new Promise(resolve => setTimeout(resolve, ms))
    }

    return (
        <Grid container spacing={1}>
            <Grid item xs={6}>
                {brand_type_cn.map((item: string, type_index: number) => (
                    <React.Fragment key={type_index}>
                        <div style={{ display: 'flex', alignItems: 'center' }}>
                            <FormControl sx={{ m: 1, width: "90%" }}>
                                <InputLabel>选择{item}配置</InputLabel>
                                <Select
                                    label="选择滤镜轮配置"
                                    value={
                                        device_selections !== null
                                            ? device_selections[brand_type_en[type_index] as keyof typeof device_selections]?.device_name || ""
                                            : ""
                                    }
                                    onChange={(e) => handleDeviceChange(e, type_index)}
                                >
                                    <MenuItem value="">
                                        <em>空</em>
                                    </MenuItem>
                                    {device_list !== null ? (
                                        device_list[brand_type_en[type_index]].map((item: any, index: number) => (
                                            <MenuItem key={index} value={item.device_name}>
                                                {item.device_name}
                                            </MenuItem>
                                        ))
                                    ) : null}
                                </Select>
                            </FormControl>
                            {brand_connection[brand_type_en[type_index]] === 0 ? null : (
                                <ListItemIcon>
                                    {brand_connection[brand_type_en[type_index]] === 1 ? (
                                        <ErrorOutline color="error" />
                                    ) : (
                                        <CheckCircleOutline color="success" />
                                    )}
                                </ListItemIcon>
                            )}
                        </div>
                    </React.Fragment>
                ))}
            </Grid>
            <Grid item xs={6}>
                <Container maxWidth="sm">
                    <FormControl sx={{ m: 1, width: "100%" }}>
                        <InputLabel>加载已有配置</InputLabel>
                        <Select
                            label="加载已有配置"
                            value={device_selections !== null
                                ? device_selections.name
                                : ""}
                            onChange={handleUserConfigLoad}>
                            <MenuItem value="">
                                <em>空</em>
                            </MenuItem>
                            {user_config_list !== null ? (
                                user_config_list.map((item: any, index: number) => (
                                    <MenuItem key={index} value={item.name}>{item.name}</MenuItem>
                                ))) : null}
                        </Select>
                    </FormControl>
                    <Button variant="outlined" startIcon={<Delete />} sx={{ m: 1, width: "100%" }}
                        // 重置信息
                        onClick={() => {
                            handleResetDeviceSelections();
                        }}
                    >
                        重置
                    </Button>
                    <Button variant="outlined" startIcon={<AddCircleOutlineOutlined />} sx={{ m: 1, width: "100%" }}
                        onClick={handleCreateOpen}>
                        建立一个新的配置
                    </Button>
                    <Button variant="contained" endIcon={<Send />} sx={{ m: 2, width: "100%" }}
                        onClick={async () => {
                            handleConnectDevice();
                        }}
                    >
                        连接设备
                    </Button>
                    <Dialog open={open_dialog} onClose={handleCreateClose}>
                        <DialogTitle>配置命名</DialogTitle>
                        <DialogContent>
                            <DialogContentText>
                                给新配置取一个名字，使得之后可以使用
                            </DialogContentText>
                            <TextField
                                autoFocus
                                margin="dense"
                                id="name"
                                label="名称"
                                onChange={handleSettingChange}
                                fullWidth
                                variant="standard"
                            />
                        </DialogContent>
                        <DialogActions>
                            <Button onClick={handleNewSetting}>提交</Button>
                        </DialogActions>
                    </Dialog>
                </Container>
            </Grid>
        </Grid>
    )
}
export default DeviceConnection;