/*
 * camera.hpp
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

Date: 2023-6-1

Description: Basic Camera Defination

*************************************************/

#pragma once

#include "device.hpp"

class Camera : public Device
{
public:
    /**
     * @brief 构造函数
     *
     * @param name 摄像机名称
     */
    explicit Camera(const std::string &name) : Device(name)
    {
        init();
    }

    virtual ~Camera() {}

    std::shared_ptr<Lithium::SimpleTask> getTask(const std::string &name, const nlohmann::json &params) override
    {
        if (name == "captureImage")
        {
            auto imageCaptureTask = std::make_shared<Lithium::SimpleTask>(
                [this](const nlohmann::json &params) -> nlohmann::json
                {
                    setProperty("gain", "10");
                    setProperty("offset", "10");
                    setProperty("is_exposure", "true");
                    std::cout << "Image captured" << std::endl;
                    setProperty("is_exposure", "false");
                    return {"result", "success"};
                },
                params,
                [this]()
                {
                    std::cout << "Image capture stopped" << std::endl;
                },
                true);
            return imageCaptureTask;
        }
        else
        {
            return nullptr;
        }
    }

protected:
    /**
     * @brief 启动曝光
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool startExposure(const nlohmann::json &params) { return true; }

    /**
     * @brief 中止曝光
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool abortExposure(const nlohmann::json &params) { return true; }

    /**
     * @brief 获取曝光状态
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getExposureStatus(const nlohmann::json &params) { return true; }

    /**
     * @brief 获取曝光结果
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getExposureResult(const nlohmann::json &params) { return true; }

    /**
     * @brief 保存曝光结果
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool saveExposureResult(const nlohmann::json &params) { return true; }

    /**
     * @brief 启动视频
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool startVideo(const nlohmann::json &params) { return true; }

    /**
     * @brief 停止视频
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool stopVideo(const nlohmann::json &params) { return true; }

    /**
     * @brief 获取视频状态
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getVideoStatus(const nlohmann::json &params) { return true; }

    /**
     * @brief 获取视频结果
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getVideoResult(const nlohmann::json &params) { return true; }

    /**
     * @brief 保存视频结果
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool saveVideoResult(const nlohmann::json &params) { return true; }

    /**
     * @brief 启动冷却
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool startCooling(const nlohmann::json &params) { return true; }

    /**
     * @brief 停止冷却
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool stopCooling(const nlohmann::json &params) { return true; }

    /**
     * @brief 获取温度
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getTemperature(const nlohmann::json &params) { return true; }

    /**
     * @brief 获取冷却功率
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getCoolingPower(const nlohmann::json &params) { return true; }

    /**
     * @brief 设置温度
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool setTemperature(const nlohmann::json &params) { return true; }

    /**
     * @brief 设置冷却功率
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool setCoolingPower(const nlohmann::json &params) { return true; }

    /**
     * @brief 获取增益值
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getGain(const nlohmann::json &params) { return true; }

    /**
     * @brief 设置增益值
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool setGain(const nlohmann::json &params) { return true; }

    /**
     * @brief 获取偏移量
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getOffset(const nlohmann::json &params) { return true; }

    /**
     * @brief 设置偏移量
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool setOffset(const nlohmann::json &params) { return true; }

    /**
     * @brief 获取ISO值
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getISO(const nlohmann::json &params) { return true; }

    /**
     * @brief 设置ISO值
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool setISO(const nlohmann::json &params) { return true; }

    /**
     * @brief 获取帧数
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool getFrame(const nlohmann::json &params) { return true; }

    /**
     * @brief 设置帧数
     *
     * @param params 参数
     * @return 成功返回true，失败返回false
     */
    virtual bool setFrame(const nlohmann::json &params) { return true; }
};
