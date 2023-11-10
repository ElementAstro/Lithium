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

#include "atom/utils/time.hpp"
#include "websocket/template/error_message.hpp"
#include "atom/error/error_code.hpp"

#include "loguru/loguru.hpp"
#include "nlohmann/json.hpp"
#include "magic_enum/magic_enum.hpp"

void WebSocketServer::AddTask(const json &m_params)
{
	json res = {{"command", __func__}};

	// 检查必要参数是否存在
	if (!m_params.contains("device_name") && !m_params.contains("device_uuid"))
	{
		RESPONSE_ERROR(res, ServerError::MissingParameters, "Device name or uuid is required");
	}
	// 检查任务的来源是否指定，主要是来自设备管理器和插件管理器
	if (!m_params.contains("task_origin") || !m_params.contains("task_name"))
	{
		RESPONSE_ERROR(res, ServerError::MissingParameters, "Task origin and name are required");
	}

	try
	{
		const std::string device_name = m_params.value("device_name", "");
		const std::string device_uuid = m_params.value("device_uuid", "");
		const std::string task_origin = m_params.value("task_origin", "");
		const std::string task_name = m_params.value("task_name", "");
		if (task_origin == "device")
		{
			Lithium::DeviceType device_type;
			auto it = DeviceTypeMap.find(m_params["device_type"]);
			if (it == DeviceTypeMap.end())
			{
				RESPONSE_ERROR(res, ServerError::InvalidParameters, "Unsupport device type");
			}
			device_type = it->second;
			std::shared_ptr<Lithium::SimpleTask> task = Lithium::MyApp->getTask(device_type, device_name, task_name, m_params["task_params"]);
			if (!task)
			{
				RESPONSE_ERROR(res, ServerError::RunFailed, "Failed to add device task");
			}
			if (!Lithium::MyApp->addTask(task))
			{
				RESPONSE_ERROR(res, ServerError::RunFailed, "Failed to add task to task manager");
			}
		}
		else if (task_origin == "plugin")
		{
		}
		else
		{
			RESPONSE_ERROR(res, ServerError::InvalidFormat, "Unknown task origin");
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

void WebSocketServer::InsertTask(const json &m_params)
{
	json res = {{"command", __func__}};
	try
	{
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

void WebSocketServer::ExecuteAllTasks(const json &m_params)
{
	json res = {{"command", __func__}};

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
		RESPONSE_EXCEPTION(res, ServerError::InvalidParameters, e.what());
	}
	catch (const std::exception &e)
	{
		RESPONSE_EXCEPTION(res, ServerError::UnknownError, e.what());
	}
}

void WebSocketServer::StopTask(const json &m_params)
{
	json res = {{"command", __func__}};

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
		RESPONSE_EXCEPTION(res, ServerError::InvalidParameters, e.what());
	}
	catch (const std::exception &e)
	{
		RESPONSE_EXCEPTION(res, ServerError::UnknownError, e.what());
	}
}

void WebSocketServer::ExecuteTaskByName(const json &m_params)
{
	json res = {{"command", __func__}};

	if (!m_params.contains("task_name"))
	{
		RESPONSE_ERROR(res, ServerError::MissingParameters, "Task name is required");
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
		RESPONSE_EXCEPTION(res, ServerError::InvalidParameters, e.what());
	}
	catch (const std::exception &e)
	{
		RESPONSE_EXCEPTION(res, ServerError::UnknownError, e.what());
	}
}

void WebSocketServer::ModifyTask(const json &m_params)
{
	json res = {{"command", __func__}};
	try
	{
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

void WebSocketServer::ModifyTaskByName(const json &m_params)
{
	json res = {{"command", __func__}};
	try
	{
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

void WebSocketServer::DeleteTask(const json &m_params)
{
	json res = {{"command", __func__}};
	try
	{
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

void WebSocketServer::DeleteTaskByName(const json &m_params)
{
	json res = {{"command", __func__}};
	try
	{
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

void WebSocketServer::QueryTaskByName(const json &m_params)
{
	json res = {{"command", __func__}};
	try
	{
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

void WebSocketServer::GetTaskList(const json &m_params)
{
	json res = {{"command", __func__}};
	try
	{
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

void WebSocketServer::SaveTasksToJson(const json &m_params)
{
	json res = {{"command", __func__}};

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
		RESPONSE_EXCEPTION(res, ServerError::InvalidParameters, e.what());
	}
	catch (const std::exception &e)
	{
		RESPONSE_EXCEPTION(res, ServerError::UnknownError, e.what());
	}
}