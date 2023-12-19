
#include "WsInstance.hpp"
#include "WsHub.hpp"

#include "atom/utils/time.hpp"
#include "websocket/template/error_message.hpp"
#include "websocket/template/function.hpp"
#include "websocket/template/variable.hpp"
#include "atom/error/error_code.hpp"

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "magic_enum/magic_enum.hpp"

WsInstance::WsInstance(const std::shared_ptr<AsyncWebSocket> &socket,
                     const std::shared_ptr<WsHub> &hub,
                     const oatpp::String &connection_name,
                     v_int32 userId)
        : m_socket(socket), m_hub(hub), m_connection_name(connection_name), m_userId(userId)
    {
    }

void WsInstance::sendMessage(const oatpp::String &message)
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

void WsInstance::sendBinaryMessage(const void *binary_message, int size)
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

std::shared_ptr<WsHub> WsInstance::getHub()
{
	return m_hub;
}

oatpp::String WsInstance::getName()
{
	return m_connection_name;
}

v_int32 WsInstance::getId()
{
	return m_userId;
}

oatpp::async::CoroutineStarter WsInstance::onPing(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message)
{
	return oatpp::async::synchronize(&m_writeLock, socket->sendPongAsync(message));
}

oatpp::async::CoroutineStarter WsInstance::onPong(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message)
{
	return nullptr; // do nothing
}

oatpp::async::CoroutineStarter WsInstance::onClose(const std::shared_ptr<AsyncWebSocket> &socket, v_uint16 code, const oatpp::String &message)
{
	return nullptr; // do nothing
}

oatpp::async::CoroutineStarter WsInstance::readMessage(const std::shared_ptr<AsyncWebSocket> &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size)
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