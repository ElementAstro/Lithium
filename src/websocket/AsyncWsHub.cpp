/*
 * AsyncWsHub.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-15

Description: WebSocket Connections hub.

**************************************************/

#include "AsyncWsHub.hpp"

AsyncWsHub::AsyncWsHub(const oatpp::String &name)
		: m_name(name)
	{
	}

void AsyncWsHub::addConnection(const std::shared_ptr<WsInstance> &Connection)
{
    std::lock_guard<std::mutex> guard(m_ConnectionByIdLock);
    m_ConnectionById[Connection->getId()] = Connection;
}

void AsyncWsHub::removeConnectionByUserId(v_int32 userId)
{
    std::lock_guard<std::mutex> guard(m_ConnectionByIdLock);
    m_ConnectionById.erase(userId);
}

void AsyncWsHub::sendMessage(const oatpp::String &message)
{
    std::lock_guard<std::mutex> guard(m_ConnectionByIdLock);
    for (auto &pair : m_ConnectionById)
    {
        pair.second->sendMessage(message);
    }
}

void AsyncWsHub::sendBinaryMessage(const void *binary_message, int size)
{
    std::lock_guard<std::mutex> guard(m_ConnectionByIdLock);
    for (auto &pair : m_ConnectionById)
    {
        pair.second->sendBinaryMessage(binary_message, size);
    }
}