
#include "WsPluginInstance.hpp"
#include "WsPluginHub.hpp"

void WsPluginInstance::sendMessage(const oatpp::String &message)
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

void WsPluginInstance::sendBinaryMessage(const void *binary_message, int size)
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

std::shared_ptr<WsPluginHub> WsPluginInstance::getHub()
{
	return m_hub;
}

oatpp::String WsPluginInstance::getPluginName()
{
	return m_plugin_name;
}

v_int32 WsPluginInstance::getUserId()
{
	return m_userId;
}

oatpp::async::CoroutineStarter WsPluginInstance::onPing(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message)
{
	return oatpp::async::synchronize(&m_writeLock, socket->sendPongAsync(message));
}

oatpp::async::CoroutineStarter WsPluginInstance::onPong(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message)
{
	return nullptr; // do nothing
}

oatpp::async::CoroutineStarter WsPluginInstance::onClose(const std::shared_ptr<AsyncWebSocket> &socket, v_uint16 code, const oatpp::String &message)
{
	return nullptr; // do nothing
}

oatpp::async::CoroutineStarter WsPluginInstance::readMessage(const std::shared_ptr<AsyncWebSocket> &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size)
{

	if (size == 0)
	{ // message transfer finished

		auto wholeMessage = m_messageBuffer.toString();
		m_messageBuffer.setCurrentPosition(0);

		m_hub->sendMessage(m_plugin_name + ": " + wholeMessage);
	}
	else if (size > 0)
	{ // message frame received
		m_messageBuffer.writeSimple(data, size);
	}

	return nullptr; // do nothing
}