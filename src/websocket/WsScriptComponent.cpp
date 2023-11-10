/*
 * WsScriptComponent.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-8-10

Description: Script API of WebSocket Server

**************************************************/

#include "WebSocketServer.hpp"
#include "LithiumApp.hpp"

#include "atom/utils/time.hpp"
#include "websocket/template/error_message.hpp"
#include "atom/error/error_code.hpp"

#include "loguru/loguru.hpp"
#include "nlohmann/json.hpp"
#include "magic_enum/magic_enum.hpp"

void WebSocketServer::runChaiCommand(const json &m_params)
{
    json res = {{"command", __func__}};
    if (!m_params.contains("command"))
    {
        RESPONSE_ERROR(res, ServerError::MissingParameters, "command content is required");
    }
    try
    {
        std::string command = m_params["command"].get<std::string>();
        if (!Lithium::MyApp->runChaiCommand(command))
        {
            res["error"] = "ScriptError";
            res["message"] = "Failed to run command";
        }
    }
    catch (const json::exception &e)
    {
        RESPONSE_EXCEPTION(res, ServerError::InvalidParameters, e.what());
    }
    catch (const std::exception &e)
    {
        RESPONSE_EXCEPTION(res, ServerError::UnknownError, e.what());
    }
}

void WebSocketServer::runChaiMultiCommand(const json &m_params)
{
    json res = {{"command", __func__}};
    if (!m_params.contains("command"))
    {
        RESPONSE_ERROR(res, ServerError::MissingParameters, "command content is required");
    }
    try
    {
        if (!Lithium::MyApp->runChaiMultiCommand(m_params["command"].get<std::vector<std::string>>()))
        {
            res["error"] = "ScriptError";
            res["message"] = "Failed to run multiline command";
        }
    }
    catch (const json::exception &e)
    {
        RESPONSE_EXCEPTION(res, ServerError::InvalidParameters, e.what());
    }
    catch (const std::exception &e)
    {
        RESPONSE_EXCEPTION(res, ServerError::UnknownError, e.what());
    }
}

void WebSocketServer::runChaiScript(const json &m_params)
{
    json res = {{"command", __func__}};
    if (!m_params.contains("script"))
    {
        RESPONSE_ERROR(res, ServerError::MissingParameters, "script name is required");
    }
    try
    {
        if (!Lithium::MyApp->runChaiScript(m_params["script"].get<std::string>()))
        {
            res["error"] = "ScriptError";
            res["message"] = "Failed to run script";
        }
    }
    catch (const json::exception &e)
    {
        RESPONSE_EXCEPTION(res, ServerError::InvalidParameters, e.what());
    }
    catch (const std::exception &e)
    {
        RESPONSE_EXCEPTION(res, ServerError::UnknownError, e.what());
    }
}

void WebSocketServer::loadChaiFile(const json &m_params)
{
    json res = {{"command", __func__}};
    if (!m_params.contains("command"))
    {
        RESPONSE_ERROR(res, ServerError::MissingParameters, "script name is required");
    }
    try
    {
        if (!Lithium::MyApp->loadChaiScriptFile(m_params["script"].get<std::string>()))
        {
            RESPONSE_ERROR(res, ServerError::RunFailed, "Failed to load script");
        }
    }
    catch (const json::exception &e)
    {
        RESPONSE_EXCEPTION(res, ServerError::InvalidParameters, e.what());
    }
    catch (const std::exception &e)
    {
        RESPONSE_EXCEPTION(res, ServerError::UnknownError, e.what());
    }
}
