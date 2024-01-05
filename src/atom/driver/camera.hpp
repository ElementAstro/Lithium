/*
 * camera.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-6-1

Description: Camera Simulator and Basic Definition

*************************************************/

#pragma once

#include "device.hpp"

#ifdef ENABLE_SHARED_MEMORY
#include "shared_memory.hpp"
#endif

class CameraFrame
{
public:
    std::atomic_int binning_x;
    std::atomic_int binning_y;

    std::atomic<double> pixel;
    std::atomic<double> pixel_x;
    std::atomic<double> pixel_y;
    std::atomic<double> pixel_depth;

    std::atomic<double> frame_x;
    std::atomic<double> frame_y;
    std::atomic<double> max_frame_x;
    std::atomic<double> max_frame_y;

    std::atomic_int frame_height;
    std::atomic_int frame_width;
    
    std::string frame_type;
    std::string frame_format;
    std::string upload_mode;
    std::atomic_bool is_fastread;
};

class Camera : public Device
{
public:
    /**
     * @brief 构造函数
     *
     * @param name 摄像机名称
     */
    explicit Camera(const std::string &name);

    virtual ~Camera();

    virtual bool connect(const json &params) override;

    virtual bool disconnect(const json &params) override;

    virtual bool reconnect(const json &params) override;

    virtual bool isConnected() override;

public:
    /**
     * @brief 启动曝光
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool startExposure(const json &params);

    /**
     * @brief 中止曝光
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool abortExposure(const json &params);

    /**
     * @brief 获取曝光状态
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getExposureStatus(const json &params);

    /**
     * @brief 获取曝光结果
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getExposureResult(const json &params);

    /**
     * @brief 保存曝光结果
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool saveExposureResult(const json &params);

    /**
     * @brief 启动视频
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool startVideo(const json &params);

    /**
     * @brief 停止视频
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool stopVideo(const json &params);

    /**
     * @brief 获取视频状态
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getVideoStatus(const json &params);

    /**
     * @brief 获取视频结果
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getVideoResult(const json &params);

    /**
     * @brief 保存视频结果
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool saveVideoResult(const json &params);

    /**
     * @brief 启动冷却
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool startCooling(const json &params);

    /**
     * @brief 停止冷却
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool stopCooling(const json &params);

    virtual bool getCoolingStatus(const json &params);

    virtual bool isCoolingAvailable();

    /**
     * @brief 获取温度
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getTemperature(const json &params);

    /**
     * @brief 获取冷却功率
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getCoolingPower(const json &params);

    /**
     * @brief 设置温度
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool setTemperature(const json &params);

    /**
     * @brief 设置冷却功率
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool setCoolingPower(const json &params);

    /**
     * @brief 获取增益值
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getGain(const json &params);

    /**
     * @brief 设置增益值
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool setGain(const json &params);

    virtual bool isGainAvailable();

    /**
     * @brief 获取偏移量
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getOffset(const json &params);

    /**
     * @brief 设置偏移量
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool setOffset(const json &params);

    virtual bool isOffsetAvailable();

    /**
     * @brief 获取ISO值
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getISO(const json &params);

    /**
     * @brief 设置ISO值
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool setISO(const json &params);

    virtual bool isISOAvailable();

    /**
     * @brief 获取帧数
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getFrame(const json &params);

    /**
     * @brief 设置帧数
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool setFrame(const json &params);

    virtual bool isFrameSettingAvailable();
};
