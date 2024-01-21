#include "ws_server.hpp"

#include "config.h"

#include <chrono>
#include <sstream>
#include <ostream>

#include "atom/log/loguru.hpp"
#include "atom/server/global_ptr.hpp"
#include "config/configor.hpp"

#include "LithiumApp.hpp"

WsContext::WsContext() : timerID(INVALID_TIMER_ID)
{
}

WsContext::~WsContext()
{
}

int WsContext::handleMessage(const std::string &msg, enum ws_opcode opcode)
{
    // text, just parse into json and process it
    if (opcode == WS_OPCODE_TEXT) {
        try
        {
            json data = json::parse(msg);
            DLOG_F(INFO, "onmessage(type=text len={}): {}", data.size(), data.dump());

        }
        catch(const json::exception &e)
        {
            LOG_F(ERROR, "parse json error: {}", e.what());
        }
        catch(const std::exception& e)
        {
            LOG_F(ERROR, "handle message error: {}", e.what());
        }
    }
    // binary, just print it
    else if (opcode == WS_OPCODE_BINARY) {
        DLOG_F(INFO, "onmessage(type=binary len={}): {}", (int)msg.size(), msg.data());
        return msg.size();
    }
    // unknown
    else 
    {
        DLOG_F(ERROR, "onmessage(type={} len={}): {}\n", opcode == WS_OPCODE_TEXT ? "text" : "binary",
               (int)msg.size(), msg.data())
    }
    return msg.size();
}

WebSocketServer::WebSocketServer() : m_host("127.0.0.1"), m_port(8080), m_isSSL(false)
{
    m_ws.onopen = [](const WebSocketChannelPtr &channel, const HttpRequestPtr &req)
    {
        DLOG_F(INFO, "onopen: {}", req->getRemoteAddr().c_str());
        auto ctx = channel->newContextPtr<WsContext>();
        // send(heartbeat) every 5s
        ctx->timerID = setInterval(5000, [channel](TimerID id)
                                   {
            if (channel->isConnected() && channel->isWriteComplete()) {
                channel->send(getHeartbeatString());
            } });
    };
    m_ws.onmessage = [](const WebSocketChannelPtr &channel, const std::string &msg)
    {
        auto ctx = channel->getContextPtr<MyContext>();
        ctx->handleMessage(msg, channel->opcode);
    };
    m_ws.onclose = [](const WebSocketChannelPtr &channel)
    {
        DLOG_F(INFO, "onclose");
        auto ctx = channel->getContextPtr<MyContext>();
        if (ctx->timerID != INVALID_TIMER_ID)
        {
            killTimer(ctx->timerID);
            ctx->timerID = INVALID_TIMER_ID;
        }
    };
}

WebSocketServer::~WebSocketServer()
{
}

bool WebSocketServer::start(const std::string &host, int port, bool isSSL)
{
    m_host = host;
    m_port = port;
    m_isSSL = isSSL;

    m_server.port = port;
#if ENABLE_SSL
    m_server.https_port = port + 1;
    hssl_ctx_opt_t param;
    memset(&param, 0, sizeof(param));
    param.crt_file = "cert/server.crt";
    param.key_file = "cert/server.key";
    param.endpoint = HSSL_SERVER;
    if (server.newSslCtx(&param) != 0)
    {
        LOG_F(ERROR, "SSL server start failed!");
        return false;
    }
#endif
    m_server.registerWebSocketService(&ws);

    m_server.start();
    return true;
}

bool WebSocketServer::stop()
{
    return true;
}

bool WebSocketServer::isRunning() const
{
    return true;
}

bool WebSocketServer::sendMessage(const std::string &msg, const std::string &clientID)
{
    return true;
}

std::string WebSocketServer::getHeartbeatString()
{
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    std::stringstream ss;
    ss << "{\"type\":\"heartbeat\",\"timestamp\":" << timestamp << "}" << std::endl;
    return ss.str();
}