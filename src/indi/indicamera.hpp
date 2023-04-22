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

Description: INDI Camera

**************************************************/

#pragma once

#include "api/indiclient.hpp"
#include "device/basic_device.hpp"
#include "task/camera_task.hpp"

#include <libindi/basedevice.h>
#include <libindi/indiproperty.h>

#include <string>

#include <spdlog/spdlog.h>
#include "nlohmann/json.hpp"

namespace OpenAPT
{
    class INDICamera : public Camera, public OpenAptIndiClient
    {
        // INDI Parameters
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
        INDI::BaseDevice *camera_device;
        // 调试模式
        ISwitchVectorProperty *debug_prop;
        // 信息刷新间隔
        INumberVectorProperty *polling_prop;
        // 已连接的辅助设备
        ITextVectorProperty * active_device_prop;
        //是否压缩
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

        // INDI 指令
        std::string indi_camera_cmd = "CCD_"; // INDI 控制命令前缀
        std::string indi_blob_name;           // BLOB 文件名
        std::string indi_camera_exec = "";    // INDI 执行命令
        std::string indi_camera_version;
        std::string indi_camera_interface;

        nlohmann::json camera_info;

    public:
        // 构造函数
        INDICamera(const std::string &name);
        // 析构函数
        ~INDICamera();

        // 连接相机
        bool connect(std::string name) override;
        // 断开连接
        bool disconnect() override;
        // 重新连接
        bool reconnect() override;
        // 搜索可用设备
        bool scanForAvailableDevices() override;

        bool getParameter(const std::string &paramName) override;

        bool setParameter(const std::string &paramName, const std::string &paramValue) override;

        // 开始曝光
        bool startExposure(int duration_ms) override;
        // 停止曝光
        bool stopExposure() override;
        // 等待曝光完成
        bool waitForExposureComplete() override;

        // 开始实时预览
        bool startLiveView() override;
        // 停止实时预览
        bool stopLiveView() override;

        // 设置制冷
        bool setCoolingOn(bool on) override;
        // 设置温度
        bool setTemperature(double temperature) override;
        // 获取当前温度
        double getTemperature();

        // 设置快门开关
        bool setShutterOpen(bool open) override;

        // 设置图像子区域
        // bool setSubframe(const ImageRect& rect);

        // 设置二次取样
        bool setBinning(int binning) override;

        // 设置增益
        bool setGain(int gain) override;

        // 设置偏移
        bool setOffset(int offset) override;

        // 设置帧区域
        bool setROIFrame(int start_x, int start_y, int frame_x, int frame_y) override;

        // 获取简单任务
        std::shared_ptr<OpenAPT::SimpleTask> getSimpleTask(const std::string &task_name, const nlohmann::json &params) override;
        // 获取条件任务
        std::shared_ptr<OpenAPT::ConditionalTask> getCondtionalTask(const std::string &task_name, const nlohmann::json &params) override;
        // 获取循环任务
        std::shared_ptr<OpenAPT::LoopTask> getLoopTask(const std::string &task_name, const nlohmann::json &params) override;

    protected:
        // 清空状态
        void ClearStatus();

        // INDI Client API
    protected:
        // 新设备
        void newDevice(INDI::BaseDevice *dp) override;
        // 删除设备
        void removeDevice(INDI::BaseDevice *dp) override;
        // 新属性
        void newProperty(INDI::Property *property) override;
        // 删除属性
        void removeProperty(INDI::Property *property) override {}
        // 新 BLOB 数据
        void newBLOB(IBLOB *bp) override;
        // 新开关属性
        void newSwitch(ISwitchVectorProperty *svp) override;
        // 新数值属性
        void newNumber(INumberVectorProperty *nvp) override;
        // 新消息
        void newMessage(INDI::BaseDevice *dp, int messageID) override;
        // 新文本属性
        void newText(ITextVectorProperty *tvp) override;
        // 新灯属性
        void newLight(ILightVectorProperty *lvp) override {}
        // INDI 服务器连接成功
        void IndiServerConnected() override;
        // INDI 服务器断开连接
        void IndiServerDisconnected(int exit_code) override;
    };
}