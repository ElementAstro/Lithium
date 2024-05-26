/*
 * Registry.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-13

Description: Websocket Hub Registry

**************************************************/

#include "Registry.hpp"

#include "Constants.hpp"

Registry::Registry() {}

void Registry::sendSocketErrorAsync(
    const std::shared_ptr<AsyncWebSocket>& socket,
    const oatpp::Object<ErrorDto>& error, bool fatal) {
    class SendErrorCoroutine
        : public oatpp::async::Coroutine<SendErrorCoroutine> {
    private:
        std::shared_ptr<AsyncWebSocket> m_websocket;
        oatpp::String m_message;
        bool m_fatal;

    public:
        SendErrorCoroutine(const std::shared_ptr<AsyncWebSocket>& websocket,
                           const oatpp::String& message, bool fatal)
            : m_websocket(websocket), m_message(message), m_fatal(fatal) {}

        Action act() override {
            /* synchronized async pipeline */
            auto call = m_websocket->sendOneFrameTextAsync(m_message);

            if (m_fatal) {
                return call.next(m_websocket->sendCloseAsync()).next(finish());
            }

            return call.next(finish());
        }
    };

    auto message = MessageDto::createShared();
    message->code = MessageCodes::OUTGOING_ERROR;
    message->payload = error;

    m_asyncExecutor->execute<SendErrorCoroutine>(
        socket, m_objectMapper->writeToString(message), fatal);
}

oatpp::String Registry::getRequiredParameter(
    const oatpp::String& name,
    const std::shared_ptr<const ParameterMap>& params,
    SessionInfo& sessionInfo) {
    auto it = params->find(name);
    if (it != params->end() && it->second) {
        return it->second;
    }
    sessionInfo.error =
        ErrorDto::createShared(ErrorCodes::BAD_REQUEST,
                               "Missing required parameter - '" + name + "'.");
    return nullptr;
}

Registry::SessionInfo Registry::getSessionForConnection(
    const std::shared_ptr<AsyncWebSocket>& socket,
    const std::shared_ptr<const ParameterMap>& params) {
    SessionInfo result;

    auto hubId =
        getRequiredParameter(Constants::PARAM_GAME_ID, params, result);
    if (result.error)
        return result;

    auto sessionId =
        getRequiredParameter(Constants::PARAM_GAME_SESSION_ID, params, result);
    if (result.error)
        return result;

    auto connectionType =
        getRequiredParameter(Constants::PARAM_PEER_TYPE, params, result);
    if (result.error)
        return result;

    result.isHost = connectionType == Constants::PARAM_PEER_TYPE_HOST;

    auto hub = getHubById(hubId);
    if (!hub) {
        result.error =
            ErrorDto::createShared(ErrorCodes::GAME_NOT_FOUND,
                                   "Hub config not found. Hub config should "
                                   "be present on the server.");
        return result;
    }

    if (result.isHost) {
        result.session = hub->createNewSession(sessionId);
        if (!result.session) {
            result.error =
                ErrorDto::createShared(ErrorCodes::OPERATION_NOT_PERMITTED,
                                       "Session with such ID already exists. "
                                       "Can't create new session session.");
            return result;
        }
    } else {
        result.session = hub->findSession(sessionId);
        if (!result.session) {
            result.error = ErrorDto::createShared(
                ErrorCodes::SESSION_NOT_FOUND,
                "No hub session found for given sessionId.");
            return result;
        }
    }

    return result;
}

std::shared_ptr<Hub> Registry::getHubById(const oatpp::String& hubId) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_hubs.find(hubId);
    if (it != m_hubs.end()) {
        return it->second;
    }

    auto config = m_hubConfig->getHubConfig(hubId);
    if (config) {
        auto hub = std::make_shared<Hub>(config);
        m_hubs.insert({config->hubId, hub});
        return hub;
    }

    return nullptr;
}

void Registry::onAfterCreate_NonBlocking(
    const std::shared_ptr<AsyncWebSocket>& socket,
    const std::shared_ptr<const ParameterMap>& params) {
    OATPP_LOGD("Registry", "socket created - %d", socket.get())

    auto sessionInfo = getSessionForConnection(socket, params);

    if (sessionInfo.error) {
        sendSocketErrorAsync(socket, sessionInfo.error, true);
        return;
    }

    auto connection = std::make_shared<Connection>(
        socket, sessionInfo.session, sessionInfo.session->generateNewConnectionId());

    socket->setListener(connection);

    OATPP_LOGD("Registry", "connection created for socket - %d", socket.get())

    sessionInfo.session->addConnection(connection, sessionInfo.isHost);
}

void Registry::onBeforeDestroy_NonBlocking(
    const std::shared_ptr<AsyncWebSocket>& socket) {
    OATPP_LOGD("Registry", "destroying socket - %d", socket.get())

    auto connection = std::static_pointer_cast<Connection>(socket->getListener());
    if (connection) {
        connection->invalidateSocket();

        auto session = connection->getHubSession();

        bool isEmptySession;
        session->removeConnectionById(connection->getConnectionId(), isEmptySession);

        if (isEmptySession) {
            auto hub = getHubById(session->getConfig()->hubId);
            hub->deleteSession(session->getId());
            OATPP_LOGD("Registry", "Session deleted - %d", session.get())
        }

    } else {
        socket->getConnection().invalidate();
    }
}
