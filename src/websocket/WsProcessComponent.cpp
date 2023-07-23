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

#include "loguru/loguru.hpp"
#include "nlohmann/json.hpp"

nlohmann::json WebSocketServer::CreateProcessLi(const nlohmann::json &m_params)
{
	nlohmann::json res;
	res["command"] = "CreateProcess";
	try
	{
		if (!m_params.contains("command") || !m_params.contains("cmd_id"))
		{
			LOG_F(ERROR, "CreateProcess() : Command and ID are required");
			res["error"] = "Command and ID are required";
			return res;
		}
		std::string command = m_params["command"].get<std::string>();
		std::string cmd_id = m_params["cmd_id"].get<std::string>();
		if (!Lithium::MyApp.createProcess(command, cmd_id))
		{
			res["error"] = "Failed to create process";
			return res;
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		LOG_F(ERROR, "WebSocketServer::CreateProcess() json exception: %s", e.what());
		res["error"] = "Invalid parameters";
		res["message"] = e.what();
	}
	catch (const std::exception &e)
	{
		LOG_F(ERROR, "Error occurred in CreateProcess: %s", e.what());
		res["error"] = "Error occurred in CreateProcess";
		res["message"] = e.what();
	}
	return res;
}

nlohmann::json WebSocketServer::RunScript(const nlohmann::json &m_params)
{
	nlohmann::json res;
	res["command"] = "RunScript";
	try
	{
		if (!m_params.contains("script_name") || !m_params.contains("script_id"))
		{
			LOG_F(ERROR, "RunScript() : Script name and ID are required");
			res["error"] = "Script name and ID are required";
			return res;
		}
		std::string script_name = m_params["script_name"].get<std::string>();
		std::string script_id = m_params["script_id"].get<std::string>();
		if (!Lithium::MyApp.runScript(script_name, script_id))
		{
			res["error"] = "Failed to run script";
			return res;
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		LOG_F(ERROR, "WebSocketServer::RunScript() json exception: %s", e.what());
		res["error"] = "Invalid parameters";
		res["message"] = e.what();
	}
	catch (const std::exception &e)
	{
		LOG_F(ERROR, "Error occurred in RunScript: %s", e.what());
		res["error"] = "Error occurred in RunScript";
		res["message"] = e.what();
	}
	return res;
}

nlohmann::json WebSocketServer::TerminateProcessByName(const nlohmann::json &m_params)
{
	nlohmann::json res;
	res["command"] = "TerminateProcessByName";
	try
	{
		if (!m_params.contains("process_name"))
		{
			LOG_F(ERROR, "TerminateProcessByName() : Process name is required");
			res["error"] = "Process name is required";
			return res;
		}
		std::string process_name = m_params["process_name"].get<std::string>();
		if (!Lithium::MyApp.terminateProcessByName(process_name))
		{
			res["error"] = "Failed to terminate process";
			return res;
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		LOG_F(ERROR, "WebSocketServer::TerminateProcessByName() json exception: %s", e.what());
		res["error"] = "Invalid parameters";
		res["message"] = e.what();
	}
	catch (const std::exception &e)
	{
		LOG_F(ERROR, "Error occurred in TerminateProcessByName: %s", e.what());
		res["error"] = "Error occurred in TerminateProcessByName";
		res["message"] = e.what();
	}
	return res;
}

nlohmann::json WebSocketServer::GetRunningProcesses(const nlohmann::json &m_params)
{
	nlohmann::json res;
	res["command"] = "GetRunningProcesses";
	try
	{
		for (auto process : Lithium::MyApp.getRunningProcesses())
		{
			res["result"][process.name]["name"] = process.name;
			res["result"][process.name]["pid"] = process.pid;
			res["result"][process.name]["output"] = process.output;
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		LOG_F(ERROR, "WebSocketServer::GetRunningProcesses() json exception: %s", e.what());
		res["error"] = "Invalid parameters";
		res["message"] = e.what();
	}
	catch (const std::exception &e)
	{
		LOG_F(ERROR, "Error occurred in GetRunningProcesses: %s", e.what());
		res["error"] = "Error occurred in GetRunningProcesses";
		res["message"] = e.what();
	}
	return res;
}

nlohmann::json WebSocketServer::GetProcessOutput(const nlohmann::json &m_params)
{
	nlohmann::json res;
	res["command"] = "GetProcessOutput";
	try
	{
		if (!m_params.contains("process_name") || !m_params.contains("cmd_id"))
		{
			LOG_F(ERROR, "GetProcessOutput() : Process name is required");
			res["error"] = "Process name is required";
			return res;
		}
		std::string process_name = m_params["process_name"].get<std::string>();
		for (auto output : Lithium::MyApp.getProcessOutput(process_name))
		{
			res["result"].push_back(output);
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		LOG_F(ERROR, "WebSocketServer::GetProcessOutput() json exception: %s", e.what());
		res["error"] = "Invalid parameters";
		res["message"] = e.what();
	}
	catch (const std::exception &e)
	{
		LOG_F(ERROR, "Error occurred in GetProcessOutput: %s", e.what());
		res["error"] = "Error occurred in GetProcessOutput";
		res["message"] = e.what();
	}
	return res;
}
