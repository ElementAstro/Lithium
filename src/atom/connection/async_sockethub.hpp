#ifndef ATOM_CONNECTION_ASYNC_SOCKETHUB_HPP
#define ATOM_CONNECTION_ASYNC_SOCKETHUB_HPP

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <functional>
#include <memory>
#include <string>

namespace atom::async::connection {

class SocketHub {
public:
    SocketHub(bool use_ssl = false);
    ~SocketHub();

    void start(int port);
    void stop();

    void addHandler(
        const std::function<void(const std::string&, size_t)>& handler);
    void addConnectHandler(const std::function<void(size_t)>& handler);
    void addDisconnectHandler(const std::function<void(size_t)>& handler);

    void broadcastMessage(const std::string& message);
    void sendMessageToClient(size_t client_id, const std::string& message);

    [[nodiscard]] auto isRunning() const -> bool;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace atom::async::connection

#endif  // ATOM_CONNECTION_ASYNC_SOCKETHUB_HPP
