/*
 * WsDeviceComponent.cpp
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

Description: Device API of WebSocket Server

**************************************************/

#include "WebSocketServer.hpp"
#include "LithiumApp.hpp"

#include "atom/utils/time.hpp"
#include "atom/error/error_code.hpp"
#include "core/device_type.hpp"

#include "template/error_message.hpp"
#include "template/function.hpp"
#include "template/variable.hpp"

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "magic_enum/magic_enum.hpp"

void WebSocketServer::GetDeviceList(const json &m_params)
{
	FUNCTION_BEGIN;
	if (!m_params.contains("device_type"))
	{
		RESPONSE_ERROR(res, ServerError::MissingParameters, "Device type is required");
	}
	DeviceType device_type;
	auto it = DeviceTypeMap.find(m_params["device_type"]);
	if (it == DeviceTypeMap.end())
	{
		RESPONSE_ERROR(res, ServerError::InvalidParameters, "Unsupport device type");
	}
	device_type = it->second;
	for (const auto &device : Lithium::MyApp->getDeviceList(device_type))
	{
		res["params"].push_back(device);
	}
	FUNCTION_END;
}

void WebSocketServer::AddDevice(const json &m_params)
{
	json res = {{"command", __func__}};
	try
	{
		if (!m_params.contains("device_type") || !m_params.contains("device_name"))
		{
			RESPONSE_ERROR(res, ServerError::MissingParameters, "Device type and name are required");
		}
		DeviceType device_type;
		auto it = DeviceTypeMap.find(m_params["device_type"]);
		if (it == DeviceTypeMap.end())
		{
			RESPONSE_ERROR(res, ServerError::InvalidParameters, "Unsupport device type");
		}
		device_type = it->second;

		if (!Lithium::MyApp->addDevice(device_type, m_params["device_name"].get<std::string>(), m_params.value("lib_name", "")))
		{
			RESPONSE_ERROR(res, ServerError::RunFailed, "Failed to add device");
		}
		else
		{
			Lithium::MyApp->addDeviceObserver(device_type, m_params["device_name"].get<std::string>());
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

void WebSocketServer::AddDeviceLibrary(const json &m_params)
{
	json res = {{"command", __func__}};
	if (!m_params.contains("lib_path") || !m_params.contains("lib_name"))
	{
		RESPONSE_ERROR(res, ServerError::MissingParameters, "Device library path and name are required");
	}
	try
	{
		std::string lib_path = m_params["lib_path"].get<std::string>();
		std::string lib_name = m_params["lib_name"].get<std::string>();
		if (!Lithium::MyApp->addDeviceLibrary(lib_path, lib_name))
		{
			res["error"] = "Failed to add device library";
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

void WebSocketServer::RemoveDevice(const json &m_params)
{
	json res = {{"command", __func__}};
	if (!m_params.contains("device_type") || !m_params.contains("device_name"))
	{
		RESPONSE_ERROR(res, ServerError::MissingParameters, "Device type and name are required");
	}
	try
	{
		DeviceType device_type;
		auto it = DeviceTypeMap.find(m_params["device_type"]);
		if (it == DeviceTypeMap.end())
		{
			RESPONSE_ERROR(res, ServerError::InvalidParameters, "Unsupport device type");
		}
		device_type = it->second;

		std::string device_name = m_params["device_name"].get<std::string>();

		if (!Lithium::MyApp->removeDevice(device_type, device_name))
		{
			RESPONSE_ERROR(res, ServerError::RunFailed, "Failed to remove device");
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

void WebSocketServer::RemoveDevicesByName(const json &m_params)
{
	json res = {{"command", __func__}};
	if (!m_params.contains("device_name"))
	{
		RESPONSE_ERROR(res, ServerError::MissingParameters, "Device name is required");
	}
	try
	{
		std::string device_name = m_params["device_name"].get<std::string>();

		if (!Lithium::MyApp->removeDeviceByName(device_name))
		{
			RESPONSE_ERROR(res, ServerError::RunFailed, "Failed to remove device by name");
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

void WebSocketServer::RemoveDeviceLibrary(const json &m_params)
{
	json res = {{"command", __func__}};
	if (!m_params.contains("lib_name"))
	{
		RESPONSE_ERROR(res, ServerError::MissingParameters, "Device library name is required");
	}
	try
	{
		std::string lib_name = m_params["lib_name"].get<std::string>();

		if (!Lithium::MyApp->removeDeviceLibrary(lib_name))
		{
			RESPONSE_ERROR(res, ServerError::RunFailed, "Failed to remove device library");
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

void WebSocketServer::RunDeviceTask(const json &m_params)
{
	json res = {{"command", __func__}};
	DeviceType device_type;
	// 检查必要参数是否存在
	if (!(m_params.contains("device_name") || m_params.contains("device_uuid")) || !m_params.contains("device_type"))
	{
		RESPONSE_ERROR(res, ServerError::MissingParameters, "Device name or uuid is required");
	}
	try
	{
		std::string device_name = m_params.value("device_name", "");
		auto it = DeviceTypeMap.find(m_params["device_type"]);
		if (it == DeviceTypeMap.end())
		{
			RESPONSE_ERROR(res, ServerError::InvalidParameters, "Device type not supported");
		}
		device_type = it->second;

		// 检查任务名称是否存在
		if (!m_params.contains("task_name"))
		{
			RESPONSE_ERROR(res, ServerError::MissingParameters, "Task name is required");
		}
		std::string task_name = m_params["task_name"];

		// 获取任务并执行
		std::shared_ptr<Lithium::BasicTask> task = Lithium::MyApp->getTask(device_type, device_name, task_name, {});
		if (!task)
		{
			RESPONSE_ERROR(res, ServerError::RunFailed, "Failed to get task");
		}
		DLOG_F(INFO, "Trying to run {}", task->getName());
		task->execute();
		json result = task->getResult();

		// 检查任务执行结果
		if (result.contains("error") && result.contains("message"))
		{
			RESPONSE_ERROR(res, ServerError::RunFailed, result["message"].get<std::string>());
		}
		res["result"] = result;
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

void WebSocketServer::GetDeviceInfo(const json &m_params)
{
	json res = {{"command", __func__}};

	// 检查必要参数是否存在
	if (!m_params.contains("device_name") && !m_params.contains("device_uuid"))
	{
		RESPONSE_ERROR(res, ServerError::MissingParameters, "Device name or uuid is required");
	}

	try
	{
		std::string device_name = m_params.value("device_name", "");
		std::shared_ptr<Device> device = Lithium::MyApp->findDeviceByName(device_name);
		if (!device)
		{
			res["error"] = "Device not found";
		}
		else
		{
			std::shared_ptr<Lithium::SimpleTask> task = device->getTask("GetDeviceInfo", {});
			if (!task)
			{
				res["error"] = "GetDeviceInfo task not found";
			}
			else
			{
				task->execute();
			}
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