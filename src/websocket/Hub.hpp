/*
 * Hub.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-13

Description: Websocket Connection Hub

**************************************************/

#ifndef Helicopter_hub_Hub_hpp
#define Helicopter_hub_Hub_hpp

#include "./Session.hpp"
#include "config/HubsConfig.hpp"

class Hub {
private:
    struct State {
        oatpp::Object<HubConfigDto> config;
        std::unordered_map<oatpp::String, std::shared_ptr<Session>> sessions;
        std::mutex mutex;
        bool isPingerActive;
    };

private:
    std::shared_ptr<State> m_state;

private:
    OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, m_asyncExecutor);

private:
    void startPinger();

public:
    /**
     * Constructor.
     * @param config
     */
    Hub(const oatpp::Object<HubConfigDto>& config);

    /**
     * Not thread safe.
     * Create new hub session.
     * @param sessionId
     * @param config
     * @return - `std::shared_ptr` to a new Session or `nullptr` if session with
     * such ID already exists.
     */
    std::shared_ptr<Session> createNewSession(const oatpp::String& sessionId);

    /**
     * NOT thread-safe
     * @param sessionId
     * @return
     */
    std::shared_ptr<Session> findSession(const oatpp::String& sessionId);

    /**
     * NOT thread-safe
     * @param sessionId
     */
    void deleteSession(const oatpp::String& sessionId);
};

#endif  // Helicopter_hub_Hub_hpp
