#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <string>
#include <memory>
#include <boost/asio.hpp>
#include "json.hpp"
#include "spdlog/spdlog.h"

using boost::asio::ip::tcp;
using json = nlohmann::json;

class TcpClient
{
public:
    TcpClient(const std::string &host, const std::string &port);
    ~TcpClient();

    bool Connect();
    bool Send(const json &data);
    bool Receive(json &response);
    void Disconnect();
    bool CheckServerExistence();
    bool IsConnected();

private:
    boost::asio::io_context io_context_;
    tcp::socket socket_;
    std::string host_;
    std::string port_;
    std::shared_ptr<spdlog::logger> logger_;
};

#endif // TCP_CLIENT_H
