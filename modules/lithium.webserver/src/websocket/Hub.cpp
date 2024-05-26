/*
 * Hub.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-13

Description: Websocket Connection Hub

**************************************************/

#include "Hub.hpp"

Hub::Hub(const oatpp::Object<HubConfigDto>& config)
    : m_state(std::make_shared<State>()) {
    m_state->config = config;
    m_state->isPingerActive = false;
}

void Hub::startPinger() {
    class Pinger : public oatpp::async::Coroutine<Pinger> {
    private:
        std::shared_ptr<State> m_state;

    public:
        Pinger(const std::shared_ptr<State>& state) : m_state(state) {}

        Action act() override {
            std::lock_guard<std::mutex> lock(m_state->mutex);

            if (m_state->sessions.empty()) {
                m_state->isPingerActive = false;
                OATPP_LOGD("Pinger", "Stopped")
                return finish();
            }

            for (auto& session : m_state->sessions) {
                session.second->checkAllConnectionsPings();
            }

            for (auto& session : m_state->sessions) {
                session.second->pingAllConnections();
            }

            return waitRepeat(
                std::chrono::milliseconds(m_state->config->pingIntervalMillis));
        }
    };

    if (!m_state->isPingerActive) {
        OATPP_LOGD("Pinger", "Started")
        m_state->isPingerActive = true;
        m_asyncExecutor->execute<Pinger>(m_state);
    }
}

std::shared_ptr<Session> Hub::createNewSession(
    const oatpp::String& sessionId) {
    std::lock_guard<std::mutex> lock(m_state->mutex);

    auto it = m_state->sessions.find(sessionId);
    if (it != m_state->sessions.end()) {
        return nullptr;
    }

    auto session = std::make_shared<Session>(sessionId, m_state->config);
    m_state->sessions.insert({sessionId, session});

    startPinger();

    return session;
}

std::shared_ptr<Session> Hub::findSession(const oatpp::String& sessionId) {
    std::lock_guard<std::mutex> lock(m_state->mutex);
    auto it = m_state->sessions.find(sessionId);
    if (it != m_state->sessions.end()) {
        return it->second;
    }
    return nullptr;  // Session not found.
}

void Hub::deleteSession(const oatpp::String& sessionId) {
    std::lock_guard<std::mutex> lock(m_state->mutex);
    m_state->sessions.erase(sessionId);
}
