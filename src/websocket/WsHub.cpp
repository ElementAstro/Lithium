
#include "WsHub.hpp"

#include "atom/"

WsHub::WsHub(const oatpp::String &name)
		: m_name(name)
	{
	}

void WsHub::addConnection(const std::shared_ptr<WsInstance> &Connection)
{
    std::lock_guard<std::mutex> guard(m_ConnectionByIdLock);
    m_ConnectionById[Connection->getId()] = Connection;
}

void WsHub::removeConnectionByUserId(v_int32 userId)
{
    std::lock_guard<std::mutex> guard(m_ConnectionByIdLock);
    m_ConnectionById.erase(userId);
}

void WsHub::sendMessage(const oatpp::String &message)
{
    std::lock_guard<std::mutex> guard(m_ConnectionByIdLock);
    for (auto &pair : m_ConnectionById)
    {
        pair.second->sendMessage(message);
    }
}

void WsHub::sendBinaryMessage(const void *binary_message, int size)
{
    std::lock_guard<std::mutex> guard(m_ConnectionByIdLock);
    for (auto &pair : m_ConnectionById)
    {
        pair.second->sendBinaryMessage(binary_message, size);
    }
}