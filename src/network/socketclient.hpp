#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <string>
#include <memory>
#include <boost/asio.hpp>
#include "json.hpp"
#include "spdlog/spdlog.h"

using boost::asio::ip::tcp;
using json = nlohmann::json;

/**
 * @brief TCP客户端类，用于向TCP服务器发送数据并接收响应。
 */
class TcpClient
{
public:
    /**
     * @brief 构造函数，创建TCP客户端对象。
     * @param host 服务器主机名或IP地址。
     * @param port 服务器端口号。
     */
    TcpClient(const std::string &host, const std::string &port);

    /**
     * @brief 析构函数，释放资源。
     */
    ~TcpClient();

    /**
     * @brief 连接到服务器。
     * @return 连接是否成功。
     */
    bool Connect();

    /**
     * @brief 发送数据到服务器。
     * @param data 要发送的数据。
     * @return 发送是否成功。
     *
     * 发送数据到服务器。如果发送成功，返回 true，否则返回 false。
     * 调用示例：
     * @code{.cpp}
     * TcpClient client("127.0.0.1", "12345");
     * if (client.Connect()) {
     *     json data = {{"msg_type", "hello"}, {"name", "world"}};
     *     if (client.Send(data)) {
     *         // 发送成功
     *     }
     * }
     * @endcode
     */
    bool Send(const json &data);

    /**
     * @brief 从服务器接收响应数据。
     * @param response 存储接收到的响应数据。
     * @param timeout 超时时间，单位为秒，默认为-1表示没有超时限制。
     * @return 接收是否成功。
     *
     * 从服务器接收响应数据。如果接收成功，将响应数据存储在 response 中并返回 true，
     * 否则返回 false。如果指定了超时时间，则在超时后函数会返回 false。
     * 调用示例：
     * @code{.cpp}
     * TcpClient client("127.0.0.1", "12345");
     * if (client.Connect()) {
     *     json data = {{"msg_type", "hello"}, {"name", "world"}};
     *     if (client.Send(data)) {
     *         json response;
     *         if (client.Receive(response, 10)) {
     *             // 接收成功
     *         }
     *     }
     * }
     * @endcode
     */
    bool Receive(json &response, int timeout = -1);

    /**
     * @brief 断开与服务器的连接。
     *
     * 断开与服务器的连接。
     * 调用示例：
     * @code{.cpp}
     * TcpClient client("127.0.0.1", "12345");
     * if (client.IsConnected()) {
     *     client.Disconnect();
     * }
     * @endcode
     */
    void Disconnect();

    /**
     * @brief 检查服务器是否存在。
     * @return 服务器是否存在。
     *
     * 检查服务器是否存在。如果服务器存在，返回 true，否则返回 false。
     * 调用示例：
     * @code{.cpp}
     * TcpClient client("127.0.0.1", "12345");
     * if (client.CheckServerExistence()) {
     *     // 服务器存在
     * }
     * @endcode
     */
    bool CheckServerExistence();

    /**
     * @brief 判断是否连接到服务器。
     * @return 是否连接到服务器。
     *
     * 判断是否连接到服务器。如果连接到服务器，返回 true，否则返回 false。
     * 调用示例：
     * @code{.cpp}
     * TcpClient client("127.0.0.1", "12345");
     * if (client.Connect()) {
     *     if (client.IsConnected()) {
     *         // 已连接到服务器
     *     }
     * }
     * @endcode
     */
    bool IsConnected();

    /**
     * @brief 设置操作超时时间。
     * @param timeout 超时时间，单位为秒。
     *
     * 设置操作超时时间，如果在指定时间内操作未完成，则函数会返回。
     * 调用示例：
     * @code{.cpp}
     * TcpClient client("127.0.0.1", "12345");
     * client.SetTimeout(10);
     * @endcode
     */
    void SetTimeout(int timeout);

    /**
     * @brief 清空socket缓存区。
     *
     * 清空socket缓存区。
     * 调用示例：
     * @code{.cpp}
     * TcpClient client("127.0.0.1", "12345");
     * client.ClearSocket();
     * @endcode
     */
    void ClearSocket();

    /**
     * @brief 设置接收缓存区大小。
     * @param size 缓存区大小，单位为字节。
     *
     * 设置接收缓存区大小。
     * 调用示例：
     * @code{.cpp}
     * TcpClient client("127.0.0.1", "12345");
     * client.SetRecvBufferSize(1024);
     * @endcode
     */
    void SetRecvBufferSize(int size);

    /**
     * @brief 设置发送缓存区大小。
     * @param size 缓存区大小，单位为字节。
     *
     * 设置发送缓存区大小。
     * 调用示例：
     * @code{.cpp}
     * TcpClient client("127.0.0.1", "12345");
     * client.SetSendBufferSize(1024);
     * @endcode
     */
    void SetSendBufferSize(int size);

    /**
     * @brief 设置是否启用SO_KEEPALIVE功能。
     * @param enable 是否启用。
     *
     * 设置是否启用SO_KEEPALIVE功能。
     * 调用示例：
     * @code{.cpp}
     * TcpClient client("127.0.0.1", "12345");
     * client.SetKeepAlive(true);
     * @endcode
     */
    void SetKeepAlive(bool enable);

    /**
     * @brief 设置是否启用SO_LINGER功能。
     * @param enable 是否启用。
     * @param linger_time LINGER时间，单位为秒，默认为0表示立即关闭连接。
     *
     * 设置是否启用SO_LINGER功能。
     * 调用示例：
     * @code{.cpp}
     * TcpClient client("127.0.0.1", "12345");
     * client.SetLinger(true);
     * @endcode
     */
    void SetLinger(bool enable, int linger_time = 0);

    /**
     * @brief 设置是否启用TCP_NODELAY功能。
     * @param enable 是否启用。
     *
     * 设置是否启用TCP_NODELAY功能。
     * 调用示例：
     * @code{.cpp}
     * TcpClient client("127.0.0.1", "12345");
     * client.SetNoDelay(true);
     * @endcode
     */
    void SetNoDelay(bool enable);

    /**
     * @brief 获取本地绑定的端口信息。
     * @return 本地绑定的端口信息。
     *
     * 获取本地绑定的端口信息。
     * 调用示例：
     * @code{.cpp}
     * TcpClient client("127.0.0.1", "12345");
     * client.Connect();
     * tcp::endpoint local_ep = client.GetLocalEndpoint();
     * std::cout << "Local endpoint: " << local_ep.address().to_string() << ":" << local_ep.port() << std::endl;
     * @endcode
     */
    tcp::endpoint GetLocalEndpoint();

private:
    boost::asio::io_context io_context_;
    tcp::socket socket_;
    std::string host_;
    std::string port_;
    std::shared_ptr<spdlog::logger> logger_;
};

#endif // TCP_CLIENT_H
