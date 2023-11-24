/*
 * WsDeviceInstance.cpp
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

#include "WsDeviceInstance.hpp"
#include "WsDeviceHub.hpp"

#include "device/device_manager.hpp"
#include "atom/server/serialize.hpp"
#include "atom/server/deserialize.hpp"

#include "atom/utils/time.hpp"
#include "websocket/template/error_message.hpp"
#include "websocket/template/function.hpp"
#include "websocket/template/variable.hpp"
#include "atom/error/error_code.hpp"

#include "loguru/loguru.hpp"
#include "atom/type/json.hpp"
#include "magic_enum/magic_enum.hpp"

WsDeviceInstance::WsDeviceInstance(const std::shared_ptr<AsyncWebSocket> &socket,
								   const std::shared_ptr<WsDeviceHub> &hub,
								   const oatpp::String &device_name,
								   v_int32 userId)
	: m_socket(socket), m_hub(hub), m_device_name(device_name), m_userId(userId)
{
	OATPP_LOGD(m_device_name.getValue("").c_str(), "%s created", m_device_name.getValue("").c_str());

	m_CommandDispatcher = std::make_unique<CommandDispatcher<void, json>>();

	LiRegisterFunc("getProperty", &WsDeviceInstance::getProperty, this);
	LiRegisterFunc("getProperties", &WsDeviceInstance::getProperties, this);
	LiRegisterFunc("setProperty", &WsDeviceInstance::setProperty, this);
	LiRegisterFunc("runTask", &WsDeviceInstance::runTask, this);
	LiRegisterFunc("runFunc", &WsDeviceInstance::runFunc, this);
	LiRegisterFunc("loadDriverLibrary", &WsDeviceInstance::loadDriverLibrary, this);
	LiRegisterFunc("unloadDriverLibrary", &WsDeviceInstance::unloadDriverLibrary, this);
	LiRegisterFunc("addDriver", &WsDeviceInstance::addDriver, this);
	LiRegisterFunc("removeDriver", &WsDeviceInstance::removeDriver, this);

	m_SerializationEngine = std::make_unique<SerializationEngine>();
	m_DeserializationEngine = std::make_unique<DeserializationEngine>();
}

WsDeviceInstance::~WsDeviceInstance()
{
}

void WsDeviceInstance::sendMessage(const oatpp::String &message)
{

	class SendMessageCoroutine : public oatpp::async::Coroutine<SendMessageCoroutine>
	{
	private:
		oatpp::async::Lock *m_lock;
		std::shared_ptr<AsyncWebSocket> m_websocket;
		oatpp::String m_message;

	public:
		SendMessageCoroutine(oatpp::async::Lock *lock,
							 const std::shared_ptr<AsyncWebSocket> &websocket,
							 const oatpp::String &message)
			: m_lock(lock), m_websocket(websocket), m_message(message)
		{
		}

		Action act() override
		{
			return oatpp::async::synchronize(m_lock, m_websocket->sendOneFrameTextAsync(m_message)).next(finish());
		}
	};

	m_asyncExecutor->execute<SendMessageCoroutine>(&m_writeLock, m_socket, message);
}

void WsDeviceInstance::sendBinaryMessage(void *binary_message, int size)
{
	oatpp::String binary((const char *)binary_message, size);
	class SendMessageCoroutine : public oatpp::async::Coroutine<SendMessageCoroutine>
	{
	private:
		oatpp::async::Lock *m_lock;
		std::shared_ptr<AsyncWebSocket> m_websocket;
		oatpp::String m_message;

	public:
		SendMessageCoroutine(oatpp::async::Lock *lock,
							 const std::shared_ptr<AsyncWebSocket> &websocket,
							 const oatpp::String &message)
			: m_lock(lock), m_websocket(websocket), m_message(message)
		{
		}

		Action act() override
		{
			return oatpp::async::synchronize(m_lock, m_websocket->sendOneFrameTextAsync(m_message)).next(finish());
		}
	};

	m_asyncExecutor->execute<SendMessageCoroutine>(&m_writeLock, m_socket, binary);
}

std::shared_ptr<WsDeviceHub> WsDeviceInstance::getHub()
{
	return m_hub;
}

oatpp::String WsDeviceInstance::getDeviceName()
{
	return m_device_name;
}

v_int32 WsDeviceInstance::getUserId()
{
	return m_userId;
}

oatpp::async::CoroutineStarter WsDeviceInstance::onPing(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message)
{
	return oatpp::async::synchronize(&m_writeLock, socket->sendPongAsync(message));
}

oatpp::async::CoroutineStarter WsDeviceInstance::onPong(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message)
{
	return nullptr; // do nothing
}

oatpp::async::CoroutineStarter WsDeviceInstance::onClose(const std::shared_ptr<AsyncWebSocket> &socket, v_uint16 code, const oatpp::String &message)
{
	return nullptr; // do nothing
}

oatpp::async::CoroutineStarter WsDeviceInstance::readMessage(const std::shared_ptr<AsyncWebSocket> &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size)
{

	if (size == 0)
	{ // message transfer finished
		auto wholeMessage = m_messageBuffer.toString();
		m_messageBuffer.setCurrentPosition(0);

		json res;
		if (!json::accept(wholeMessage->c_str()))
		{
			RESPONSE_ERROR_C(res, ServerError::InvalidFormat, "Message is not in JSON format");
		}
		else
		{
			try
			{
				json jdata = json::parse(wholeMessage->c_str());
				if (jdata.contains("name") && jdata.contains("params"))
				{
					const std::string name = jdata["name"].get<std::string>();
					if (m_CommandDispatcher->HasHandler(name))
					{
						m_CommandDispatcher->Dispatch(name, jdata["params"]);
					}
				}
				else
				{
					RESPONSE_ERROR_C(res, ServerError::MissingParameters, "Missing parameter: name or params");
				}
			}
			catch (const std::exception &e)
			{
				RESPONSE_EXCEPTION_C(res, ServerError::UnknownError, e.what());
			}
		}
		if (res.contains("error") || res.contains("message"))
		{
			sendMessage(res.dump());
		}
	}
	else if (size > 0)
	{ // message frame received
		m_messageBuffer.writeSimple(data, size);
	}
	return nullptr; // do nothing
}

void WsDeviceInstance::loadDriverLibrary(const json &m_params)
{
	FUNCTION_BEGIN;
	CHECK_PARAM_EXISTS(lib_path);
	CHECK_PARAM_EXISTS(lib_name);
	GET_STRING_PARAM_VALUE(m_params["lib_path"], lib_path);
	GET_STRING_PARAM_VALUE(m_params["lib_name"], lib_name);
	if (!Lithium::MyApp->addDeviceLibrary(lib_path, lib_name))
	{
		res["error"] = "Failed to add device library";
	}
	FUNCTION_END;
}

void WsDeviceInstance::unloadDriverLibrary(const json &m_params)
{
	FUNCTION_BEGIN;
	CHECK_PARAM_EXISTS(lib_name);
	GET_STRING_PARAM_VALUE(m_params["lib_name"], lib_name);
	if (!Lithium::MyApp->removeDeviceLibrary(lib_name))
	{
		RESPONSE_ERROR(res, ServerError::RunFailed, "Failed to remove device library");
	}
	FUNCTION_END;
}

void WsDeviceInstance::addDriver(const json &m_params)
{
	FUNCTION_BEGIN;
	CHECK_PARAM_EXISTS(device_type);
	CHECK_PARAM_EXISTS(device_name);
	SET_DEVICE_TYPE(m_params["device_type"]);
	GET_STRING_PARAM_VALUE(m_params["device_name"], device_name);
	if (!Lithium::MyApp->addDevice(device_type, device_name, m_params.value("lib_name", "")))
	{
		RESPONSE_ERROR(res, ServerError::RunFailed, "Failed to add device");
	}
	else
	{
		SET_STRING_PARAM_VALUE(device_name, m_device_name);
		Lithium::MyApp->addDeviceObserver(device_type, device_name);
	}
	FUNCTION_END;
}

void WsDeviceInstance::removeDriver(const json &m_params)
{
	FUNCTION_BEGIN;
	CHECK_PARAM_EXISTS(device_type);
	CHECK_PARAM_EXISTS(device_name);
	SET_DEVICE_TYPE(m_params["device_type"]);
	GET_STRING_PARAM_VALUE(m_params["device_name"], device_name);
	if (!Lithium::MyApp->removeDevice(device_type, device_name))
	{
		RESPONSE_ERROR(res, ServerError::RunFailed, "Failed to remove device");
	}
	FUNCTION_END;
}

void WsDeviceInstance::setProperty(const json &m_params)
{
	FUNCTION_BEGIN;
	CHECK_PARAM_EXISTS(name);
	CHECK_PARAM_EXISTS(value);
	GET_STRING_PARAM_VALUE(m_params["name"], name);
	GET_STRING_PARAM_VALUE(m_params["value"], value);
	if (!Lithium::MyApp->setProperty(m_device_name, name, value))
	{
		RESPONSE_ERROR(res, ServerError::RunFailed, "Failed to remove device library");
	}
	FUNCTION_END;
}

void WsDeviceInstance::getProperty(const json &m_params)
{
	FUNCTION_BEGIN;
	CHECK_PARAM_EXISTS(name);
	GET_STRING_PARAM_VALUE(m_params["name"], name);
	if (!Lithium::MyApp->getProperty(m_device_name, name))
	{
		RESPONSE_ERROR(res, ServerError::RunFailed, "Failed to get device property");
	}
	FUNCTION_END;
}

void WsDeviceInstance::getProperties(const json &m_params)
{
	FUNCTION_BEGIN;
	FUNCTION_END;
}

void WsDeviceInstance::runTask(const json &m_params)
{
	FUNCTION_BEGIN;
	CHECK_PARAM_EXISTS(task_name);
	GET_STRING_PARAM_VALUE(m_params["task_name"], task_name);
	FUNCTION_END;
}

void WsDeviceInstance::runFunc(const json &m_params)
{
	FUNCTION_BEGIN;
	CHECK_PARAM_EXISTS(func_name);
	GET_STRING_PARAM_VALUE(m_params["func_name"], func_name);
	FUNCTION_END;
}