/*
 * WsCameraInstance.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.	If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-10-20

Description: WebSocket Device Instance (each device each instance)

**************************************************/

#include "WsCameraInstance.hpp"
#include "WsDeviceHub.hpp"

#include "device/device_manager.hpp"
#include "atom/server/serialize.hpp"
#include "atom/server/deserialize.hpp"

#include "atom/utils/time.hpp"
#include "websocket/template/error_message.hpp"
#include "websocket/template/function.hpp"
#include "atom/error/error_code.hpp"
#include "atom/utils/switch.hpp"

#include "core/camera.hpp"

#include "loguru/loguru.hpp"
#include "atom/type/json.hpp"
#include "magic_enum/magic_enum.hpp"

WsCameraInstance::WsCameraInstance(const std::shared_ptr<AsyncWebSocket> &socket,
                                   const std::shared_ptr<WsDeviceHub> &hub,
                                   const oatpp::String &device_name,
                                   v_int32 userId)
    : WsDeviceInstance(socket, hub, device_name, userId)
{
    LiRegisterFunc("startExopsure", &WsCameraInstance::startExposure, this);
    LiRegisterFunc("stopExposure", &WsCameraInstance::stopExposure, this);
    LiRegisterFunc("getExposureStatus", &WsCameraInstance::getExposureStatus, this);
    LiRegisterFunc("getExposureResult", &WsCameraInstance::getExposureResult, this);
    
    LiRegisterFunc("startCooling", &WsCameraInstance::startCooling, this);
    LiRegisterFunc("stopCooling", &WsCameraInstance::stopCooling, this);
    LiRegisterFunc("getCoolingStatus", &WsCameraInstance::getCoolingStatus, this);
    LiRegisterFunc("getCurrentTemperautre", &WsCameraInstance::getCurrentTemperature, this);
    
    LiRegisterFunc("getGain", &WsCameraInstance::getGain, this);
    LiRegisterFunc("setGain", &WsCameraInstance::setGain, this);
    LiRegisterFunc("getOffset", &WsCameraInstance::getOffset, this);
    LiRegisterFunc("setOffset", &WsCameraInstance::setOffset, this);
    LiRegisterFunc("getISO", &WsCameraInstance::getISO, this);
    LiRegisterFunc("setISO", &WsCameraInstance::setISO, this);
}

WsCameraInstance::~WsCameraInstance()
{
    DLOG_F(INFO, "WsCameraInstance Destroyed");
}

// ----------------------------------------------------------------
// Exposure Functions
// ----------------------------------------------------------------

void WsCameraInstance::startExposure(const json &m_params)
{
    FUNCTION_BEGIN;
    if (!m_params.contains("exposure") || !m_params["exposure"].is_number())
    {
        RESPONSE_ERROR(res, ServerError::InvalidParameters, "Exposure time is required and must in number");
    }
    CHECK_DEVICE_VALIDITY(m_camera, Camera)
    else
    {
        if (!m_camera->startExposure(m_params))
        {
            RESPONSE_ERROR(res, ServerError::RunFailed, "Failed to start exposure");
        }
    }
    FUNCTION_END;
}

void WsCameraInstance::stopExposure(const json &m_params)
{
    FUNCTION_BEGIN;
    CHECK_DEVICE_VALIDITY(m_camera, Camera)
    else
    {
        if (!m_camera->getExposureStatus({}))
        {
            RESPONSE_ERROR(res, DeviceWarning::ExposureWarning, "Exposure is not running");
        }
        if (!m_camera->abortExposure(m_params))
        {
            RESPONSE_ERROR(res, ServerError::RunFailed, "Failed to abort exposure");
        }
    }
    FUNCTION_END;
}

// ----------------------------------------------------------------
// Cooling Functions
// ----------------------------------------------------------------

void WsCameraInstance::startCooling(const json &m_params)
{
    FUNCTION_BEGIN;
    CHECK_DEVICE_VALIDITY(m_camera, Camera)
    else
    {
        if (!m_camera->startCooling(m_params))
        {
            RESPONSE_ERROR(res, DeviceError::CoolingError, "Failed to start cooling");
        }
    }
    FUNCTION_END;
}

void WsCameraInstance::stopCooling(const json &m_params)
{
    FUNCTION_BEGIN;
    CHECK_DEVICE_VALIDITY(m_camera, Camera)
    else
    {
        if (!m_camera->getCoolingStatus(m_params))
        {
            RESPONSE_ERROR(res, DeviceWarning::CoolingWarning, "Cooling mode is not started");
        }
        if (!m_camera->stopCooling(m_params))
        {
            RESPONSE_ERROR(res, DeviceError::CoolingError, "Failed to stop cooling");
        }
    }
    FUNCTION_END;
}

void WsCameraInstance::getCoolingStatus(const json &m_params)
{
    FUNCTION_BEGIN;
    CHECK_DEVICE_VALIDITY(m_camera, Camera)
    else
    {
        if (!m_camera->getCoolingStatus(m_params))
        {
            RESPONSE_ERROR(res, DeviceWarning::CoolingWarning, "Cooling mode is not started");
        }
    }
    FUNCTION_END;
}

void WsCameraInstance::getCurrentTemperature(const json &m_params)
{
    FUNCTION_BEGIN;
    CHECK_DEVICE_VALIDITY(m_camera, Camera)
    else
    {
        if (!m_camera->getTemperature(m_params))
        {
            RESPONSE_ERROR(res, DeviceError::CoolingError, "Failed to get temperature");
        }
    }
    FUNCTION_END;
}

// ----------------------------------------------------------------
// The following get property functions will not return the result directly, but via MessageBus in async mode
// ----------------------------------------------------------------

// ----------------------------------------------------------------
// Commonly used
// ----------------------------------------------------------------

void WsCameraInstance::getGain(const json &m_params)
{
    FUNCTION_BEGIN;
    CHECK_DEVICE_VALIDITY(m_camera, Camera)
    else
    {
        if (!m_camera->getGain(m_params))
        {
            RESPONSE_ERROR(res, DeviceError::GainError, "Failed to get gain");
        }
    }
    FUNCTION_END;
}

void WsCameraInstance::setGain(const json &m_params)
{
    FUNCTION_BEGIN;
    if (!m_params.contains("gain") || !m_params["gain"].is_number())
    {
        RESPONSE_ERROR(res, ServerError::InvalidParameters, "Gain is required and must in number");
    }
    CHECK_DEVICE_VALIDITY(m_camera, Camera)
    else
    {
        if (!m_camera->setGain(m_params))
        {
            RESPONSE_ERROR(res, DeviceError::GainError, "Failed to set gain");
        }
    }
    FUNCTION_END;
}

void WsCameraInstance::getOffset(const json &m_params)
{
    FUNCTION_BEGIN;
    CHECK_DEVICE_VALIDITY(m_camera, Camera)
    else
    {
        if (!m_camera->getOffset(m_params))
        {
            RESPONSE_ERROR(res, DeviceError::OffsetError, "Failed to get offset");
        }
    }
    FUNCTION_END;
}

void WsCameraInstance::setOffset(const json &m_params)
{
    FUNCTION_BEGIN;
    if (!m_params.contains("offset") || !m_params["offset"].is_number())
    {
        RESPONSE_ERROR(res, ServerError::InvalidParameters, "Offset is required and must in number");
    }
    CHECK_DEVICE_VALIDITY(m_camera, Camera)
    else
    {
        if (!m_camera->setOffset(m_params))
        {
            RESPONSE_ERROR(res, DeviceError::OffsetError, "Failed to set offset");
        }
    }
    FUNCTION_END;
}

void WsCameraInstance::getISO(const json &m_params)
{
    FUNCTION_BEGIN;
    CHECK_DEVICE_VALIDITY(m_camera, Camera)
    else
    {
        if (!m_camera->getISO(m_params))
        {
            RESPONSE_ERROR(res, DeviceError::OffsetError, "Failed to get ISO value");
        }
    }
    FUNCTION_END;
}

void WsCameraInstance::setISO(const json &m_params)
{
    FUNCTION_BEGIN;
    if (!m_params.contains("iso") || !m_params["iso"].is_number())
    {
        RESPONSE_ERROR(res, ServerError::InvalidParameters, "ISO is required and must in number");
    }
    CHECK_DEVICE_VALIDITY(m_camera, Camera)
    else
    {
        if (!m_camera->isISOAvailable())
        {
            RESPONSE_ERROR(res, ServerError::InvalidParameters, "ISO is not supported");
        }
        if (!m_camera->setOffset(m_params))
        {
            RESPONSE_ERROR(res, DeviceError::ISOError, "Failed to set ISO");
        }
    }
    FUNCTION_END;
}
