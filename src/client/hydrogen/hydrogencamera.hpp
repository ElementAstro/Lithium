/*
 * indicamera.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-4-9

Description: Hydrogen Camera

**************************************************/

#pragma once

#include "hydrogendevice.hpp"

class HydrogenCamera : public Camera, public LithiumIndiClient
{
    // Hydrogen Parameters
private:
    // 连接属性
    ISwitchVectorProperty *connection_prop;
    // 曝光属性
    INumberVectorProperty *expose_prop;
    // 帧属性
    INumberVectorProperty *frame_prop;
    // 温度属性
    INumberVectorProperty *temperature_prop;
    // 增益属性
    INumberVectorProperty *gain_prop;
    // 偏移属性
    INumberVectorProperty *offset_prop;
    // 帧区域参数
    INumber *indi_frame_x;
    INumber *indi_frame_y;
    INumber *indi_frame_width;
    INumber *indi_frame_height;
    // 帧类型
    ISwitchVectorProperty *frame_type_prop;
    // 图像类型
    ISwitchVectorProperty *image_type_prop;
    // CCD 设备信息
    INumberVectorProperty *ccdinfo_prop;
    // 二次取样属性
    INumberVectorProperty *binning_prop;
    // 二次取样 X 轴
    INumber *indi_binning_x;
    // 二次取样 Y 轴
    INumber *indi_binning_y;
    // 视频属性
    ISwitchVectorProperty *video_prop;
    // 视频延迟
    INumberVectorProperty *video_delay_prop;
    // 视频曝光时间
    INumberVectorProperty *video_exposure_prop;
    // 视频帧率
    INumberVectorProperty *video_fps_prop;
    // 相机端口
    ITextVectorProperty *camera_port;
    // 相机设备
    HYDROGEN::BaseDevice *camera_device;
    // 调试模式
    ISwitchVectorProperty *debug_prop;
    // 信息刷新间隔
    INumberVectorProperty *polling_prop;
    // 已连接的辅助设备
    ITextVectorProperty *active_device_prop;
    // 是否压缩
    ISwitchVectorProperty *compression_prop;
    // 图像上传模式
    ISwitchVectorProperty *image_upload_mode_prop;
    // 快速读出模式
    ISwitchVectorProperty *fast_read_out_prop;
    // 相机限制
    INumberVectorProperty *camera_limit_prop;

    // 标志位
    bool is_ready; // 是否就绪
    bool has_blob; // 是否有 BLOB 数据

    // Hydrogen 指令
    std::string indi_camera_cmd = "CCD_"; // Hydrogen 控制命令前缀
    std::string indi_blob_name;           // BLOB 文件名
    std::string indi_camera_exec = "";    // Hydrogen 执行命令
    std::string indi_camera_version;
    std::string indi_camera_interface;

    std::atomic_bool is_connected;
    std::atomic_bool is_exposure;
    std::atomic_bool is_video;

    nlohmann::json device_info;

private:
    // For Hydrogen Toupcamera

    ISwitchVectorProperty *toupcam_fan_control_prop;

    ISwitchVectorProperty *toupcam_heat_control_prop;

    ISwitchVectorProperty *toupcam_hcg_control_prop;

    ISwitchVectorProperty *toupcam_low_noise_control_prop;

    ISwitchVectorProperty *toupcam_simulation_prop;

    ISwitchVectorProperty *toupcam_binning_mode_prop;

    // For Hydrogen ZWOASI

    // 图像翻转
    ISwitchVectorProperty *asi_image_flip_prop;
    // 图像翻转
    ISwitchVectorProperty *asi_image_flip_hor_prop;
    ISwitchVectorProperty *asi_image_flip_ver_prop;
    // 控制模式
    INumberVectorProperty *asi_controls_prop;
    // 控制模式
    ISwitchVectorProperty *asi_controls_mode_prop;

    // For Hydrogen QHYCCD

public:
    // 构造函数
    HydrogenCamera(const std::string &name);
    // 析构函数
    ~HydrogenCamera();

    bool connect(const IParams &params) override;

    bool disconnect(const IParams &params) override;

    bool reconnect(const IParams &params) override;

    bool isConnected(const IParams &params) override;

public:
    /**
     * @brief 启动曝光
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool startExposure(const nlohmann::json &params);

    /**
     * @brief 中止曝光
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool abortExposure(const nlohmann::json &params);

    /**
     * @brief 获取曝光状态
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getExposureStatus(const nlohmann::json &params);

    /**
     * @brief 获取曝光结果
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getExposureResult(const nlohmann::json &params);

    /**
     * @brief 保存曝光结果
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool saveExposureResult(const nlohmann::json &params);

    /**
     * @brief 启动视频
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool startVideo(const nlohmann::json &params);

    /**
     * @brief 停止视频
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool stopVideo(const nlohmann::json &params);

    /**
     * @brief 获取视频状态
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getVideoStatus(const nlohmann::json &params);

    /**
     * @brief 获取视频结果
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getVideoResult(const nlohmann::json &params);

    /**
     * @brief 保存视频结果
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool saveVideoResult(const nlohmann::json &params);

    /**
     * @brief 启动冷却
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool startCooling(const nlohmann::json &params);

    /**
     * @brief 停止冷却
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool stopCooling(const nlohmann::json &params);

    bool isCoolingAvailable();

    /**
     * @brief 获取温度
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getTemperature(const nlohmann::json &params);

    /**
     * @brief 获取冷却功率
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getCoolingPower(const nlohmann::json &params);

    /**
     * @brief 设置温度
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool setTemperature(const nlohmann::json &params);

    /**
     * @brief 设置冷却功率
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool setCoolingPower(const nlohmann::json &params);

    /**
     * @brief 获取增益值
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getGain(const nlohmann::json &params);

    /**
     * @brief 设置增益值
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool setGain(const nlohmann::json &params);

    bool isGainAvailable();

    /**
     * @brief 获取偏移量
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getOffset(const nlohmann::json &params);

    /**
     * @brief 设置偏移量
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool setOffset(const nlohmann::json &params);

    bool isOffsetAvailable();

    /**
     * @brief 获取ISO值
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getISO(const nlohmann::json &params);

    /**
     * @brief 设置ISO值
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool setISO(const nlohmann::json &params);

    bool isISOAvailable();

    /**
     * @brief 获取帧数
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool getFrame(const nlohmann::json &params);

    /**
     * @brief 设置帧数
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    bool setFrame(const nlohmann::json &params);

    bool isFrameSettingAvailable();

protected:
    // 清空状态
    void ClearStatus();

    // Hydrogen Client API
protected:
    // 新设备
    void newDevice(HYDROGEN::BaseDevice *dp) override;
    // 删除设备
    void removeDevice(HYDROGEN::BaseDevice *dp) override;
    // 新属性
    void newProperty(HYDROGEN::Property *property) override;
    // 删除属性
    void removeProperty(HYDROGEN::Property *property) override {}
    // 新 BLOB 数据
    void newBLOB(IBLOB *bp) override;
    // 新开关属性
    void newSwitch(ISwitchVectorProperty *svp) override;
    // 新数值属性
    void newNumber(INumberVectorProperty *nvp) override;
    // 新消息
    void newMessage(HYDROGEN::BaseDevice *dp, int messageID) override;
    // 新文本属性
    void newText(ITextVectorProperty *tvp) override;
    // 新灯属性
    void newLight(ILightVectorProperty *lvp) override {}
    // Hydrogen 服务器连接成功
    void IndiServerConnected() override;
    // Hydrogen 服务器断开连接
    void IndiServerDisconnected(int exit_code) override;
};