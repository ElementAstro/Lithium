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

void WebSocketServer::AddTask(const json &m_params)
{
	json res;
	res["command"] = "AddTask";

	// 检查必要参数是否存在
	if (!m_params.contains("device_name") && !m_params.contains("device_uuid"))
	{
		res = {{"error", "Invalid Parameters"}, {"message", "Device name or uuid is required"}};
		LOG_F(ERROR, "WebSocketServer::AddTask() : %s", res.dump().c_str());
		return res;
	}
	// 检查任务的来源是否指定，主要是来自设备管理器和插件管理器
	if (!m_params.contains("task_origin") || !m_params.contains("task_name"))
	{
		res = {{"error", "Invalid Parameters"}, {"message", "Task origin and name are required"}};
		LOG_F(ERROR, "WebSocketServer::AddTask() : %s", res.dump().c_str());
		return res;
	}

	try
	{
		const std::string device_name = m_params.value("device_name", "");
		const std::string device_uuid = m_params.value("device_uuid", "");
		const std::string task_origin = m_params.value("task_origin","");
		const std::string task_name = m_params.value("task_name","");
		if (task_origin == "device")
		{
			Lithium::DeviceType device_type;
			auto it = DeviceTypeMap.find(m_params["device_type"]);
			if (it == DeviceTypeMap.end())
			{
				res["error"] = "Unsupport device type";
				LOG_F(ERROR, "Unsupport device type, AddTask() : %s", res.dump().c_str());
				return res;
			}
			device_type = it->second;
			std::shared_ptr<Lithium::SimpleTask> task = Lithium::MyApp->getTask(device_type, device_name, task_name, m_params["task_params"]);
			if (!task)
			{
				res = {{"error", "Task Failed"}, {"message", "Failed to add device task"}};
				return res;
			}
			if (!Lithium::MyApp->addTask(task))
			{
				res = {{"error", "Task Failed"}, {"message", "Failed to add task to task manager"}};
				return res;
			}
		}
		else if (task_origin == "plugin")
		{
		}
		else
		{
			res = {{"error", "Invalid Parameters"}, {"message", "Unknown task origin"}};
		}
	}
	catch (const json::exception &e)
	{
		LOG_F(ERROR, "WebSocketServer::AddTask() json exception: %s", e.what());
		res = {{"error", "Invalid Parameters"}, {"message", e.what()}};
	}
	catch (const std::exception &e)
	{
		res = {{"error", "Unknown Error"}, {"message", e.what()}};
		LOG_F(ERROR, "WebSocketServer::AddTask(): %s", e.what());
	}
	return res;
}

void WebSocketServer::InsertTask(const json &m_params)
{
	return {};
}

void WebSocketServer::ExecuteAllTasks(const json &m_params)
{
	json res;
	res["command"] = "ExecuteAllTasks";

	try
	{
		if (!Lithium::MyApp->executeAllTasks())
		{
			res = {{"error", "Task Failed"}, {"message", "Failed to execute task in sequence"}};
			LOG_F(ERROR, "WebSocketServer::ExecuteAllTasks : Failed to start executing all tasks");
		}
	}
	catch (const json::exception &e)
	{
		LOG_F(ERROR, "WebSocketServer::ExecuteAllTasks() json exception: %s", e.what());
		res = {{"error", "Invalid Parameters"}, {"message", e.what()}};
	}
	catch (const std::exception &e)
	{
		res = {{"error", "Unknown Error"}, {"message", e.what()}};
		LOG_F(ERROR, "WebSocketServer::ExecuteAllTasks(): %s", e.what());
	}
	return res;
}

void WebSocketServer::StopTask(const json &m_params)
{
	json res;
	res["command"] = "StopTask";

	try
	{
		if (!Lithium::MyApp->stopTask())
		{
			res = {{"error", "Task Failed"}, {"message", "Failed to stop current task"}};
			LOG_F(ERROR, "WebSocketServer::StopTask(): Failed to stop current task");
		}
	}
	catch (const json::exception &e)
	{
		LOG_F(ERROR, "WebSocketServer::StopTask() json exception: %s", e.what());
		res = {{"error", "Invalid Parameters"}, {"message", e.what()}};
	}
	catch (const std::exception &e)
	{
		res = {{"error", "Unknown Error"}, {"message", e.what()}};
		LOG_F(ERROR, "WebSocketServer::StopTask(): %s", e.what());
	}
	return res;
}

void WebSocketServer::ExecuteTaskByName(const json &m_params)
{
	json res;
	res["command"] = "ExecuteTaskByName";

	if (!m_params.contains("task_name"))
	{
		res = {{"error", "Invalid Parameters"}, {"message", "Task name is required"}};
		LOG_F(ERROR, "WebSocketServer::ExecuteTaskByName() : %s", res.dump().c_str());
		return res;
	}

	try
	{
		const std::string task_name = m_params.value("task_name", "");
		if (!Lithium::MyApp->executeTaskByName(task_name))
		{
			res = {{"error", "Task Failed"}, {"message", "Failed to execute specific task"}};
			LOG_F(ERROR, "WebSocketServer::ExecuteTaskByName(): Failed to execute specific task");
		}
	}
	catch (const json::exception &e)
	{
		LOG_F(ERROR, "WebSocketServer::ExecuteTaskByName() json exception: %s", e.what());
		res = {{"error", "Invalid Parameters"}, {"message", e.what()}};
	}
	catch (const std::exception &e)
	{
		res = {{"error", "Unknown Error"}, {"message", e.what()}};
		LOG_F(ERROR, "WebSocketServer::ExecuteTaskByName(): %s", e.what());
	}
	return res;
}

void WebSocketServer::ModifyTask(const json &m_params)
{
	return {};
}

void WebSocketServer::ModifyTaskByName(const json &m_params)
{
	return {};
}

void WebSocketServer::DeleteTask(const json &m_params)
{
	return {};
}

void WebSocketServer::DeleteTaskByName(const json &m_params)
{
	return {};
}

void WebSocketServer::QueryTaskByName(const json &m_params)
{
	return {};
}

void WebSocketServer::GetTaskList(const json &m_params)
{
	return {};
}

void WebSocketServer::SaveTasksToJson(const json &m_params)
{
	json res;
	res["command"] = "SaveTasksToJson";

	try
	{
		if (!Lithium::MyApp->saveTasksToJson())
		{
			res = {{"error", "Task Failed"}, {"message", "Failed to save task in sequence to a JSON file"}};
			LOG_F(ERROR, "WebSocketServer::SaveTasksToJson(): Failed to save task in sequence to a JSON file");
		}
	}
	catch (const json::exception &e)
	{
		LOG_F(ERROR, "WebSocketServer::SaveTasksToJson() json exception: %s", e.what());
		res = {{"error", "Invalid Parameters"}, {"message", e.what()}};
	}
	catch (const std::exception &e)
	{
		res = {{"error", "Unknown Error"}, {"message", e.what()}};
		LOG_F(ERROR, "WebSocketServer::SaveTasksToJson(): %s", e.what());
	}
	return res;
}