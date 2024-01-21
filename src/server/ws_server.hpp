#pragma once

#include <string>

#include "libhv/http/server/WebSocketServer.h"
#include "libhv/base/htime.h"

class WsContext
{
public:
    WsContext();
    ~WsContext();

    int handleMessage(const std::string &msg, enum ws_opcode opcode);

    TimerID timerID;
};

class WebSocketServer
{
public:
    explicit WebSocketServer();
    ~WebSocketServer();

    bool start(const std::string &host, int port, bool isSSL = false);
    bool stop();
    bool isRunning() const;

    bool sendMessage(const std::string &msg, const std::string &clientID = "");

private:

    std::string getHeartbeatString() const;

    std::string m_host;
    int m_port;
    bool m_isSSL;

    hv::WebSocketService m_ws;
    hv::WebSocketServer m_server;
};