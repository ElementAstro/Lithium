#ifndef ATOM_CONNECTION_ASYNC_TCPCLIENT_HPP
#define ATOM_CONNECTION_ASYNC_TCPCLIENT_HPP

#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <vector>

namespace atom::async::connection {

class TcpClient {
public:
    using OnConnectedCallback = std::function<void()>;
    using OnDisconnectedCallback = std::function<void()>;
    using OnDataReceivedCallback =
        std::function<void(const std::vector<char>&)>;
    using OnErrorCallback = std::function<void(const std::string&)>;

    TcpClient(bool use_ssl = false);
    ~TcpClient();

    bool connect(
        const std::string& host, int port,
        std::chrono::milliseconds timeout = std::chrono::milliseconds::zero());

    void disconnect();

    void enableReconnection(int attempts);
    void setHeartbeatInterval(std::chrono::milliseconds interval);

    bool send(const std::vector<char>& data);

    std::future<std::vector<char>> receive(
        size_t size,
        std::chrono::milliseconds timeout = std::chrono::milliseconds::zero());

    [[nodiscard]] bool isConnected() const;
    [[nodiscard]] std::string getErrorMessage() const;

    void setOnConnectedCallback(const OnConnectedCallback& callback);
    void setOnDisconnectedCallback(const OnDisconnectedCallback& callback);
    void setOnDataReceivedCallback(const OnDataReceivedCallback& callback);
    void setOnErrorCallback(const OnErrorCallback& callback);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace atom::async::connection

#endif  // ATOM_CONNECTION_ASYNC_TCPCLIENT_HPP
