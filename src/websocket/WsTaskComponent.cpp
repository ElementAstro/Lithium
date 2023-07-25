/*
 * WsTaskComponent.cpp
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

Description: Task API of WebSocket Server

**************************************************/

#include "WebSocketServer.hpp"
#include "LithiumApp.hpp"

#include "loguru/loguru.hpp"
#include "nlohmann/json.hpp"

nlohmann::json WebSocketServer::AddTask(const nlohmann::json &m_params)
{
    nlohmann::json res;
	res["command"] = "AddTask";

	// 检查必要参数是否存在
	if (!m_params.contains("device_name") && !m_params.contains("device_uuid"))
	{
		res["error"] = "Device name or uuid is required";
		LOG_F(ERROR, "WebSocketServer::AddTask() : %s", res.dump().c_str());
		return res;
	}

	try
	{
		std::string device_name = m_params.value("device_name", "");
		std::shared_ptr<Device> device = Lithium::MyApp.findDeviceByName(device_name);
		if(!device)
		{
			res["error"] = "Device not found";
		}
		else{
			std::shared_ptr<Lithium::SimpleTask> task = device->getTask("AddTask",{});
			if(!task)
			{
				res["error"] = "AddTask task not found";
			}
			else
			{
				task->Execute();
			}
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		LOG_F(ERROR, "WebSocketServer::AddTask() json exception: %s", e.what());
		res["error"] = "Invalid parameters";
		res["message"] = e.what();
	}
	catch (const std::exception &e)
	{
		res["error"] = "Error occurred in WebSocketServer::AddTask";
		res["message"] = e.what();
		LOG_F(ERROR, "WebSocketServer::AddTask(): %s", e.what());
	}
	return res;
}
nlohmann::json WebSocketServer::InsertTask(const nlohmann::json &m_params)
{
	return {};
}
nlohmann::json WebSocketServer::ExecuteAllTasks(const nlohmann::json &m_params)
{
	return {};
}
nlohmann::json WebSocketServer::StopTask(const nlohmann::json &m_params)
{
	return {};
}
nlohmann::json WebSocketServer::ExecuteTaskByName(const nlohmann::json &m_params)
{
	return {};
}
nlohmann::json WebSocketServer::ModifyTask(const nlohmann::json &m_params)
{
	return {};
}
nlohmann::json WebSocketServer::ModifyTaskByName(const nlohmann::json &m_params)
{
	return {};
}
nlohmann::json WebSocketServer::DeleteTask(const nlohmann::json &m_params)
{
	return {};
}
nlohmann::json WebSocketServer::DeleteTaskByName(const nlohmann::json &m_params)
{
	return {};
}
nlohmann::json WebSocketServer::QueryTaskByName(const nlohmann::json &m_params)
{
	return {};
}
nlohmann::json WebSocketServer::GetTaskList(const nlohmann::json &m_params)
{
	return {};
}
nlohmann::json WebSocketServer::SaveTasksToJson(const nlohmann::json &m_params)
{
	return {};
}