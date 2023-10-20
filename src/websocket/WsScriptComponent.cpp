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

#include "loguru/loguru.hpp"
#include "nlohmann/json.hpp"

void WebSocketServer::runChaiCommand(const json &m_params)
{
    json res;
    res["command"] = "runChaiCommand";
    try
    {
        if (!m_params.contains("command"))
        {
            LOG_F(ERROR, "runChaiCommand() : Command is required");
            res["error"] = "Invalid parameters";
            res["message"] = "command content is required";
            return res;
        }
        std::string command = m_params["command"].get<std::string>();
        if (!Lithium::MyApp->runChaiCommand(command))
        {
            res["error"] = "ScriptError";
            res["message"] = "Failed to run command";
        }
    }
    catch (const json::exception &e)
    {
        LOG_F(ERROR, "WebSocketServer::runChaiCommand() json exception: %s", e.what());
        res["error"] = "Invalid parameters";
        res["message"] = e.what();
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Error occurred in runChaiCommand: %s", e.what());
        res["error"] = "StdError";
        res["message"] = e.what();
    }
    return res;
}

void WebSocketServer::runChaiMultiCommand(const json &m_params)
{
    json res;
    res["command"] = "runChaiMultiCommand";
    try
    {
        if (!m_params.contains("command"))
        {
            LOG_F(ERROR, "runChaiMultiCommand() : Command is required");
            res["error"] = "Invalid parameters";
            res["message"] = "command content is required";
            return res;
        }
        if (!Lithium::MyApp->runChaiMultiCommand(m_params["command"].get<std::vector<std::string>>()))
        {
            res["error"] = "ScriptError";
            res["message"] = "Failed to run multiline command";
        }
    }
    catch (const json::exception &e)
    {
        LOG_F(ERROR, "WebSocketServer::runChaiMultiCommand() json exception: %s", e.what());
        res["error"] = "Invalid parameters";
        res["message"] = e.what();
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Error occurred in runChaiMultiCommand: %s", e.what());
        res["error"] = "StdError";
        res["message"] = e.what();
    }
    return res;
}

void WebSocketServer::runChaiScript(const json &m_params)
{
    json res;
    res["command"] = "runChaiScript";
    try
    {
        if (!m_params.contains("script"))
        {
            LOG_F(ERROR, "runChaiScript() : Command is required");
            res["error"] = "Invalid parameters";
            res["message"] = "script name is required";
            return res;
        }
        if (!Lithium::MyApp->runChaiScript(m_params["script"].get<std::string>()))
        {
            res["error"] = "ScriptError";
            res["message"] = "Failed to run script";
        }
    }
    catch (const json::exception &e)
    {
        LOG_F(ERROR, "WebSocketServer::runChaiScript() json exception: %s", e.what());
        res["error"] = "Invalid parameters";
        res["message"] = e.what();
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Error occurred in runChaiScript: %s", e.what());
        res["error"] = "StdError";
        res["message"] = e.what();
    }
    return res;
}

void WebSocketServer::loadChaiFile(const json &m_params)
{
    json res;
    res["command"] = "loadChaiFile";
    try
    {
        if (!m_params.contains("command"))
        {
            LOG_F(ERROR, "loadChaiFile() : Command is required");
            res["error"] = "Invalid parameters";
            res["message"] = "script name is required";
            return res;
        }
        if (!Lithium::MyApp->loadChaiScriptFile(m_params["script"].get<std::string>()))
        {
            res["error"] = "ScriptError";
            res["message"] = "Failed to load script";
        }
    }
    catch (const json::exception &e)
    {
        LOG_F(ERROR, "WebSocketServer::loadChaiFile() json exception: %s", e.what());
        res["error"] = "Invalid parameters";
        res["message"] = e.what();
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Error occurred in loadChaiFile: %s", e.what());
        res["error"] = "StdError";
        res["message"] = e.what();
    }
    return res;
}
