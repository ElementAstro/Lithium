/*
 * AsyncWsInstance.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-15

Description: WebSocket Instance

**************************************************/

// Max: The main problem is that how to handle multiple connections
//      We must keep the status of each connection, and make sure the thread safe
//      We can use oatpp::async::Lock to make sure the thread safe

#include "AsyncWsInstance.hpp"
#include "WsHub.hpp"

#include "atom/utils/time.hpp"
#include "websocket/template/error_message.hpp"
#include "websocket/template/function.hpp"
#include "websocket/template/variable.hpp"
#include "atom/error/error_code.hpp"

#include "LithiumApp.hpp"

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "magic_enum/magic_enum.hpp"
using json = nlohmann::json;

AsyncWsInstance::AsyncWsInstance(const std::shared_ptr<AsyncWebSocket> &socket,
                     const std::shared_ptr<WsHub> &hub,
                     const oatpp::String &connection_name,
                     v_int32 userId)
        : m_socket(socket), m_hub(hub), m_connection_name(connection_name), m_userId(userId)
    {
    }

void AsyncWsInstance::sendMessage(const oatpp::String &message)
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

void AsyncWsInstance::sendBinaryMessage(const void *binary_message, int size)
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

std::shared_ptr<WsHub> AsyncWsInstance::getHub()
{
	return m_hub;
}

oatpp::String AsyncWsInstance::getName()
{
	return m_connection_name;
}

v_int32 AsyncWsInstance::getId()
{
	return m_userId;
}

oatpp::async::CoroutineStarter AsyncWsInstance::onPing(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message)
{
	DLOG_F(INFO, "Received ping from {} with message {}", m_connection_name->c_str(), message->c_str());
	return oatpp::async::synchronize(&m_writeLock, socket->sendPongAsync(message));
}

oatpp::async::CoroutineStarter AsyncWsInstance::onPong(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message)
{
	DLOG_F(INFO, "Received pong from {} with message {}", m_connection_name->c_str(), message->c_str());
	return nullptr; // do nothing
}

oatpp::async::CoroutineStarter AsyncWsInstance::onClose(const std::shared_ptr<AsyncWebSocket> &socket, v_uint16 code, const oatpp::String &message)
{
	DLOG_F(INFO, "Received close from {} with code {} and message {}", m_connection_name->c_str(), code, message->c_str());
	return nullptr; // do nothing
}

oatpp::async::CoroutineStarter AsyncWsInstance::readMessage(const std::shared_ptr<AsyncWebSocket> &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size)
{
	if (size == 0)
	{ // message transfer finished
		auto wholeMessage = m_messageBuffer.toString();
		m_messageBuffer.setCurrentPosition(0);
		DLOG_F(INFO, "Received message from {} with size {}", m_connection_name->c_str(), size);

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
					if (Lithium::MyApp->hasCommand(name))
					{
						res = Lithium::MyApp->DispatchCommand(name, jdata["params"]);
					}
					else
					{
						RESPONSE_ERROR_C(res, ServerError::UnknownCommand, "Unknown command: " + name);
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
		sendMessage(res.dump());
	}
	else if (size > 0)
	{ // message frame received
		m_messageBuffer.writeSimple(data, size);
	}
	return nullptr; // do nothing
}