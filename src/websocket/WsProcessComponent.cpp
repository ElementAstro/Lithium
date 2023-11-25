/*
 * WsProcessComponent.cpp
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

Date: 2023-7-13

Description: Process API of WebSocket Server

**************************************************/

#include "WebSocketServer.hpp"
#include "LithiumApp.hpp"

#include "atom/utils/time.hpp"
#include "websocket/template/error_message.hpp"
#include "atom/error/error_code.hpp"

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "magic_enum/magic_enum.hpp"

void WebSocketServer::CreateProcessLi(const json &m_params)
{
	json res = {{"command", __func__}};
	if (!m_params.contains("command") || !m_params.contains("cmd_id"))
	{
		RESPONSE_ERROR(res, ServerError::MissingParameters, "Command and ID are required");
	}
	try
	{
		if (!Lithium::MyApp->createProcess(m_params["command"].get<std::string>(), m_params["cmd_id"].get<std::string>()))
		{
			RESPONSE_ERROR(res, ServerError::RunFailed, "Failed to create process");
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

void WebSocketServer::RunScript(const json &m_params)
{
	json res = {{"command", __func__}};
	if (!m_params.contains("script_name") || !m_params.contains("script_id"))
	{
		RESPONSE_ERROR(res, ServerError::MissingParameters, "Script name and ID are required");
	}
	try
	{
		if (!Lithium::MyApp->runScript(m_params["script_name"].get<std::string>(), m_params["script_id"].get<std::string>()))
		{
			RESPONSE_ERROR(res, ServerError::RunFailed, fmt::format("Failed to script"));
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

void WebSocketServer::TerminateProcessByName(const json &m_params)
{
	json res = {{"command", __func__}};
	if (!m_params.contains("process_name"))
	{
		RESPONSE_ERROR(res,ServerError::MissingParameters,"Process name is required");
	}
	try
	{

		std::string process_name = m_params["process_name"].get<std::string>();
		if (!Lithium::MyApp->terminateProcessByName(process_name))
		{
			RESPONSE_ERROR(res,ServerError::RunFailed,"Failed to terminate process");
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

void WebSocketServer::GetRunningProcesses(const json &m_params)
{
	json res = {{"command", __func__}};
	try
	{
		for (auto process : Lithium::MyApp->getRunningProcesses())
		{
			res["result"][process.name]["name"] = process.name;
			res["result"][process.name]["pid"] = process.pid;
			res["result"][process.name]["output"] = process.output;
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

void WebSocketServer::GetProcessOutput(const json &m_params)
{
	json res = {{"command", __func__}};
	if (!m_params.contains("process_name") || !m_params.contains("cmd_id"))
	{
		RESPONSE_ERROR(res, ServerError::MissingParameters, "Process name is required");
	}
	try
	{
		for (auto output : Lithium::MyApp->getProcessOutput(m_params["process_name"].get<std::string>()))
		{
			res["result"].push_back(output);
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
