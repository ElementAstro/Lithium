/*
 * WsTelescopeInstance.cpp
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

#include "WsTelescopeInstance.hpp"
#include "WsDeviceHub.hpp"

#include "device/device_manager.hpp"
#include "atom/server/serialize.hpp"
#include "atom/server/deserialize.hpp"

#include "atom/utils/time.hpp"
#include "websocket/template/error_message.hpp"
#include "atom/error/error_code.hpp"

#include "loguru/loguru.hpp"
#include "atom/type/json.hpp"
#include "magic_enum/magic_enum.hpp"

WsTelescopeInstance::WsTelescopeInstance(const std::shared_ptr<AsyncWebSocket> &socket,
                                   const std::shared_ptr<WsDeviceHub> &hub,
                                   const oatpp::String &device_name,
                                   v_int32 userId)
    : WsDeviceInstance(socket, hub, device_name, userId)
{
    LiRegisterFunc("startExopsure", &WsTelescopeInstance::startExposure, this);
}

WsTelescopeInstance::~WsTelescopeInstance()
{
}

void WsTelescopeInstance::startExposure(const json &m_params)
{
    // 实现参数检查
    if (m_params.contains("exposure_time") && m_params["exposure_time"].is_number_integer())
    {
        int exposure_time = m_params["exposure_time"];
        if (exposure_time < 1)
        {
            json error_message = {{"error_code", ServerError::InvalidParameters},
                                   {"error_message", "exposure_time must be greater than 0"}};
            sendMessage(error_message.dump());
            return;
        }
        if (exposure_time > 1000000)
        {
            json error_message = {{"error_code", ServerError::InvalidParameters},
                                   {"error_message", "exposure_time must be less than 1000000"}};
            sendMessage(error_message.dump());
            return;
        }
    }
    else
    {
        json error_message = {{"error_code", ServerError::InvalidParameters},
                               {"error_message", "exposure_time must be an integer"}};
        sendMessage(error_message.dump());
        return;
    }
}