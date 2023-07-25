/*
 * camera.cpp
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

Date: 2023-3-29

Description: Camera Simulator and Basic Definition

**************************************************/

#include "camera.hpp"

#include "loguru/loguru.hpp"

Camera::Camera(const std::string &name) : Device(name)
{
    LOG_F(INFO, "Camera Simulator Loaded : %s", name.c_str());
    init();
}

Camera::~Camera()
{
    LOG_F(INFO, "Camera Simulator Destructed");
}

bool Camera::connect(const std::string &name)
{
    LOG_F(INFO, "%s is connected", name.c_str());
    return true;
}

bool Camera::disconnect()
{
    LOG_F(INFO, "%s is disconnected", getProperty("name").c_str());
    return true;
}

bool Camera::reconnect()
{
    return true;
}

std::shared_ptr<Lithium::SimpleTask> Camera::getTask(const std::string &name, const nlohmann::json &params)
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
    return nullptr;
}

bool Camera::startExposure(const nlohmann::json &params)
{
    return true;
}

bool Camera::abortExposure(const nlohmann::json &params)
{
    return true;
}

bool Camera::getExposureStatus(const nlohmann::json &params)
{
    return true;
}

bool Camera::getExposureResult(const nlohmann::json &params)
{
    return true;
}

bool Camera::saveExposureResult(const nlohmann::json &params)
{
    return true;
}

bool Camera::startVideo(const nlohmann::json &params)
{
    return true;
}

bool Camera::stopVideo(const nlohmann::json &params)
{
    return true;
}

bool Camera::getVideoStatus(const nlohmann::json &params)
{
    return true;
}

bool Camera::getVideoResult(const nlohmann::json &params)
{
    return true;
}

bool Camera::saveVideoResult(const nlohmann::json &params)
{
    return true;
}

bool Camera::startCooling(const nlohmann::json &params)
{
    return true;
}

bool Camera::stopCooling(const nlohmann::json &params)
{
    return true;
}

bool Camera::getTemperature(const nlohmann::json &params)
{
    return true;
}

bool Camera::getCoolingPower(const nlohmann::json &params)
{
    return true;
}

bool Camera::setTemperature(const nlohmann::json &params)
{
    return true;
}

bool Camera::setCoolingPower(const nlohmann::json &params)
{
    return true;
}

bool Camera::getGain(const nlohmann::json &params)
{
    return true;
}

bool Camera::setGain(const nlohmann::json &params)
{
    return true;
}

bool Camera::getOffset(const nlohmann::json &params)
{
    return true;
}

bool Camera::setOffset(const nlohmann::json &params)
{
    return true;
}

bool Camera::getISO(const nlohmann::json &params)
{
    return true;
}

bool Camera::setISO(const nlohmann::json &params)
{
    return true;
}

bool Camera::getFrame(const nlohmann::json &params)
{
    return true;
}

bool Camera::setFrame(const nlohmann::json &params)
{
    return true;
}