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

#include "loguru/loguru.hpp"
#include "nlohmann/json.hpp"

std::unordered_map<std::string, Lithium::DeviceType> DeviceTypeMap = {
	{"Camera", Lithium::DeviceType::Camera},
	{"Telescope", Lithium::DeviceType::Telescope},
	{"Focuser", Lithium::DeviceType::Focuser},
	{"FilterWheel", Lithium::DeviceType::FilterWheel},
	{"Solver", Lithium::DeviceType::Solver},
	{"Guider", Lithium::DeviceType::Guider}};

nlohmann::json WebSocketServer::GetDeviceList(const nlohmann::json &m_params)
{
	try
	{
		nlohmann::json res;
		res["command"] = "GetDeviceList";
		if (!m_params.contains("device_type"))
		{
			LOG_F(ERROR, "GetDeviceList() : Device type is required");
			res["error"] = "Device type is required";
			return res;
		}
		Lithium::DeviceType device_type;
		auto it = DeviceTypeMap.find(m_params["device_type"]);
		if (it == DeviceTypeMap.end())
		{
			res["error"] = "Unsupport device type";
			LOG_F(ERROR, "Unsupport device type, GetDeviceList() : %s", res.dump().c_str());
			return res;
		}
		device_type = it->second;
		for (const auto &device : Lithium::MyApp.getDeviceList(device_type))
		{
			res["result"].push_back(device);
		}
		return res;
	}
	catch (const std::exception &e)
	{
		LOG_F(ERROR, "Error occurred in GetDeviceList: %s", e.what());
		return {{"error", "Error occurred in GetDeviceList"}, {"message", e.what()}};
	}
}

nlohmann::json WebSocketServer::AddDevice(const nlohmann::json &m_params)
{
	nlohmann::json res;
	res["command"] = "AddDevice";
	try
	{
		if (!m_params.contains("device_type") || !m_params.contains("device_name"))
		{
			LOG_F(ERROR, "GetDeviceList() : Device type and name are required");
			res["error"] = "Device type and name are required";
			return res;
		}
		Lithium::DeviceType device_type;
		auto it = DeviceTypeMap.find(m_params["device_type"]);
		if (it == DeviceTypeMap.end())
		{
			res["error"] = "Unsupport device type";
			LOG_F(ERROR, "Unsupport device type, AddDevice() : %s", res.dump().c_str());
			return res;
		}
		device_type = it->second;

		if (!Lithium::MyApp.addDevice(device_type, m_params["device_name"].get<std::string>(), m_params.value("lib_name", "")))
		{
			res["error"] = "Failed to add device";
		}
		else
		{
			Lithium::MyApp.addDeviceObserver(device_type, m_params["device_name"].get<std::string>());
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		LOG_F(ERROR, "WebSocketServer::AddDevice() json exception: %s", e.what());
		res["error"] = "Invalid parameters";
	}
	catch (const std::exception &e)
	{
		LOG_F(ERROR, "Error occurred in AddDevice: %s", e.what());
		res["error"] = "Error occurred in AddDevice";
		res["message"] = e.what();
	}
	return res;
}

nlohmann::json WebSocketServer::AddDeviceLibrary(const nlohmann::json &m_params)
{
	nlohmann::json res;
	res["command"] = "AddDeviceLibrary";
	if (!m_params.contains("lib_path") || !m_params.contains("lib_name"))
	{
		LOG_F(ERROR, "WebSocketServer::AddDevice() : Device library path and name are required");
		res["error"] = "Invalid parameters";
		res["message"] = "Device library path and name are required";
		return res;
	}
	try
	{
		std::string lib_path = m_params["lib_path"].get<std::string>();
		std::string lib_name = m_params["lib_name"].get<std::string>();
		if (!Lithium::MyApp.addDeviceLibrary(lib_path, lib_name))
		{
			res["error"] = "Failed to add device library";
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		LOG_F(ERROR, "WebSocketServer::AddDeviceLibrary() json exception: %s", e.what());
		res["error"] = "Invalid parameters";
		res["message"] = e.what();
	}
	catch (const std::exception &e)
	{
		res["error"] = "Error occurred in WebSocketServer::AddDeviceLibrary";
		res["message"] = e.what();
		LOG_F(ERROR, "WebSocketServer::AddDeviceLibrary: %s", e.what());
	}
	return res;
}

nlohmann::json WebSocketServer::RemoveDevice(const nlohmann::json &m_params)
{
	nlohmann::json res;
	res["command"] = "RemoveDevice";
	if (!m_params.contains("device_type") || !m_params.contains("device_name"))
	{
		LOG_F(ERROR, "WebSocketServer::RemoveDevice() : Device type and name are required");
		res["error"] = "Invalid parameters";
		res["message"] = "Device type and name are required";
		return res;
	}
	try
	{
		Lithium::DeviceType device_type;
		auto it = DeviceTypeMap.find(m_params["device_type"]);
		if (it == DeviceTypeMap.end())
		{
			res["error"] = "Unsupport device type";
			LOG_F(ERROR, "WebSocketServer::RemoveDevice() : Unsupport device type %s", res.dump().c_str());
			return res;
		}
		device_type = it->second;

		std::string device_name = m_params["device_name"].get<std::string>();

		if (!Lithium::MyApp.removeDevice(device_type, device_name))
		{
			res["error"] = "Failed to remove device";
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		LOG_F(ERROR, "WebSocketServer::RemoveDevice() json exception: %s", e.what());
		res["error"] = "Invalid parameters";
		res["message"] = e.what();
	}
	catch (const std::exception &e)
	{
		res["error"] = "Error occurred in WebSocketServer::RemoveDevice";
		res["message"] = e.what();
		LOG_F(ERROR, "WebSocketServer::RemoveDevice(): %s", e.what());
	}
	return res;
}

nlohmann::json WebSocketServer::RemoveDevicesByName(const nlohmann::json &m_params)
{
	nlohmann::json res;
	res["command"] = "RemoveDeviceByName";
	if (!m_params.contains("device_name"))
	{
		LOG_F(ERROR, "WebSocketServer::RemoveDevice() : Device name is required");
		res["error"] = "Invalid parameters";
		res["message"] = "Device name is required";
		return res;
	}
	try
	{
		std::string device_name = m_params["device_name"].get<std::string>();

		if (!Lithium::MyApp.removeDevicesByName(device_name))
		{
			res["error"] = "Failed to remove device by name";
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		LOG_F(ERROR, "WebSocketServer::RemoveDeviceByName() json exception: %s", e.what());
		res["error"] = "Invalid parameters";
		res["message"] = e.what();
	}
	catch (const std::exception &e)
	{
		res["error"] = "Error occurred in WebSocketServer::RemoveDeviceByName";
		res["message"] = e.what();
		LOG_F(ERROR, "WebSocketServer::RemoveDeviceByName(): %s", e.what());
	}
	return res;
}

nlohmann::json WebSocketServer::RemoveDeviceLibrary(const nlohmann::json &m_params)
{
	nlohmann::json res;
	res["command"] = "RemoveDeviceLibrary";
	if (!m_params.contains("lib_name"))
	{
		LOG_F(ERROR, "WebSocketServer::RemoveDeviceLibrary() : Device name is required");
		res["error"] = "Invalid parameters";
		res["message"] = "Device library name is required";
		return res;
	}
	try
	{
		std::string lib_name = m_params["lib_name"].get<std::string>();

		if (!Lithium::MyApp.removeDeviceLibrary(lib_name))
		{
			res["error"] = "Failed to remove device library";
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		LOG_F(ERROR, "WebSocketServer::RemoveDeviceLibrary() json exception: %s", e.what());
		res["error"] = "Invalid parameters";
		res["message"] = e.what();
	}
	catch (const std::exception &e)
	{
		res["error"] = "Error occurred in WebSocketServer::RemoveDeviceLibrary";
		res["message"] = e.what();
		LOG_F(ERROR, "WebSocketServer::RemoveDeviceLibrary(): %s", e.what());
	}
	return res;
}

nlohmann::json WebSocketServer::RunDeviceTask(const nlohmann::json &m_params)
{
	nlohmann::json res;
	res["command"] = "RunDeviceTask";
	std::string device_name;
	Lithium::DeviceType device_type;

	// 检查必要参数是否存在
	if (!(m_params.contains("device_name") || m_params.contains("device_uuid")) || !m_params.contains("device_type"))
	{
		res["error"] = "Device name or uuid is required";
		LOG_F(ERROR, "WebSocketServer::RunDeviceTask() : %s", res.dump().c_str());
		return res;
	}

	// 获取设备名称和类型
	device_name = m_params.value("device_name", "");
	auto it = DeviceTypeMap.find(m_params["device_type"]);
	if (it == DeviceTypeMap.end())
	{
		res["error"] = "Device type not supported";
		LOG_F(ERROR, "WebSocketServer::RunDeviceTask() : %s", res.dump().c_str());
		return res;
	}
	device_type = it->second;

	// 检查任务名称是否存在
	if (!m_params.contains("task_name"))
	{
		res["error"] = "Task name is required";
		LOG_F(ERROR, "WebSocketServer::RunDeviceTask() : %s", res.dump().c_str());
		return res;
	}
	std::string task_name = m_params["task_name"];

	// 获取任务并执行
	std::shared_ptr<Lithium::SimpleTask> task = Lithium::MyApp.getTask(device_type, device_name, task_name, {});
	if (task == nullptr)
	{
		res["error"] = "Failed to get task";
		LOG_F(ERROR, "WebSocketServer::RunDeviceTask() : Failed to get task %s ,error %s", task_name.c_str(), res.dump().c_str());
		return res;
	}
	task->Execute();
	nlohmann::json result = task->GetResult();

	// 检查任务执行结果
	if (result.contains("error"))
	{
		res["error"] = result["error"];
		LOG_F(ERROR, "WebSocketServer::RunDeviceTask() : Error happened in task %s - %s", task_name.c_str(), result.dump().c_str());
	}
	res["result"] = result;
	return res;
}

nlohmann::json WebSocketServer::GetDeviceInfo(const nlohmann::json &m_params)
{
	nlohmann::json res;
	res["command"] = "GetDeviceInfo";

	// 检查必要参数是否存在
	if (!m_params.contains("device_name") && !m_params.contains("device_uuid"))
	{
		res["error"] = "Device name or uuid is required";
		LOG_F(ERROR, "WebSocketServer::GetDeviceInfo() : %s", res.dump().c_str());
		return res;
	}

	try
	{
		std::string device_name = m_params.value("device_name", "");
		std::shared_ptr<Device> device = Lithium::MyApp.findDeviceByName(device_name);
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
				task->Execute();
			}
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		LOG_F(ERROR, "WebSocketServer::GetDeviceInfo() json exception: %s", e.what());
		res["error"] = "Invalid parameters";
		res["message"] = e.what();
	}
	catch (const std::exception &e)
	{
		res["error"] = "Error occurred in WebSocketServer::GetDeviceInfo";
		res["message"] = e.what();
		LOG_F(ERROR, "WebSocketServer::GetDeviceInfo(): %s", e.what());
	}
	return res;
}