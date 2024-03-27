/*
 * Session.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-13

Description: Websocket Hub Connection Session

**************************************************/

#ifndef LITHIUM_WEBSOCKET_SESSION_HPP
#define LITHIUM_WEBSOCKET_SESSION_HPP

#include "Connection.hpp"
#include "config/HubsConfig.hpp"

class Session {
private:
    oatpp::String m_id;
    oatpp::Object<HubConfigDto> m_config;
    std::atomic<v_int64> m_connectionIdCounter;
    v_int64 m_synchronizedEventId;  // synchronized by m_connectionsMutex
    std::unordered_map<v_int64, std::shared_ptr<Connection>> m_connections;
    std::shared_ptr<Connection> m_host;
    std::mutex m_connectionsMutex;

private:
    v_int64 m_pingCurrentTimestamp;
    v_int64 m_pingBestTime;
    v_int64 m_pingBestConnectionId;
    v_int64 m_pingBestConnectionSinceTimestamp;
    std::mutex m_pingMutex;

public:
    Session(const oatpp::String& id,
            const oatpp::Object<HubConfigDto>& config);

    oatpp::String getId();
    oatpp::Object<HubConfigDto> getConfig();

    void addConnection(const std::shared_ptr<Connection>& connection, bool isHost = false);
    void setHost(const std::shared_ptr<Connection>& connection);
    std::shared_ptr<Connection> getHost();

    bool isHostConnection(v_int64 connectionId);

    void removeConnectionById(v_int64 connectionId, bool& isEmpty);

    std::vector<std::shared_ptr<Connection>> getAllConnections();
    std::vector<std::shared_ptr<Connection>> getConnections(
        const oatpp::Vector<oatpp::Int64>& connectionIds);

    void broadcastSynchronizedEvent(v_int64 senderId,
                                    const oatpp::String& eventData);

    v_int64 generateNewConnectionId();

    void checkAllConnectionsPings();

    void pingAllConnections();

    /**
     * Report pong from connection.
     * @param connectionId
     * @param timestamp - timestamp reported in the pong (payload). If timestamp
     * doesn't equal to the latest ping timestamp - ping considered to be
     * failed.
     * @return - connection's ping in microseconds or `-1` if ping failed.
     */
    v_int64 reportConnectionPong(v_int64 connectionId, v_int64 timestamp);
};

#endif  // LITHIUM_WEBSOCKET_SESSION_HPP
