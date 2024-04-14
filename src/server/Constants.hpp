/*
 * Constants.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-16

Description: Constants for Lithium

**************************************************/

#ifndef LITHIUM_CONSTANTS_HPP
#define LITHIUM_CONSTANTS_HPP

class Constants {
public:
    static constexpr const char* COMPONENT_REST_API = "REST_API";
    static constexpr const char* COMPONENT_WS_API = "WS_API";

public:
    static constexpr const char* PARAM_GAME_ID = "gameId";
    static constexpr const char* PARAM_GAME_SESSION_ID = "sessionId";
    static constexpr const char* PARAM_PEER_TYPE = "peerType";
    static constexpr const char* PARAM_PEER_TYPE_HOST = "host";
    static constexpr const char* PARAM_PEER_TYPE_CLIENT = "client";
};

#endif  // LITHIUM_CONSTANTS_HPP
