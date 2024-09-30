#include "async_sockethub.hpp"
#include <iostream>
#include <mutex>
#include <thread>

namespace atom::async::connection {

class SocketHub::Impl {
public:
    Impl(bool use_ssl)
        : io_context_(),
          acceptor_(io_context_),
          ssl_context_(asio::ssl::context::sslv23),
          use_ssl_(use_ssl),
          is_running_(false) {}

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
    void doAccept();
    void handleNewConnection(std::shared_ptr<asio::ip::tcp::socket> socket);
    void doRead(std::shared_ptr<asio::ip::tcp::socket> socket);
    void handleIncomingMessage(const std::string& message,
                               std::shared_ptr<asio::ip::tcp::socket> socket);
    void handleDisconnect(std::shared_ptr<asio::ip::tcp::socket> socket);
    void disconnectAllClients();
    size_t getClientId(const std::shared_ptr<asio::ip::tcp::socket>& socket);
    void log(const std::string& message);

    asio::io_context io_context_;
    asio::ip::tcp::acceptor acceptor_;
    asio::ssl::context ssl_context_;
    bool use_ssl_;
    bool is_running_;
    std::unordered_map<size_t, std::shared_ptr<asio::ip::tcp::socket>> clients_;
    std::mutex client_mutex_;
    std::vector<std::function<void(const std::string&, size_t)>> handlers_;
    std::mutex handler_mutex_;
    std::vector<std::function<void(size_t)>> connect_handlers_;
    std::mutex connect_handler_mutex_;
    std::vector<std::function<void(size_t)>> disconnect_handlers_;
    std::mutex disconnect_handler_mutex_;
    size_t next_client_id_ = 1;
    std::thread io_thread_;
};

SocketHub::SocketHub(bool use_ssl) : impl_(std::make_unique<Impl>(use_ssl)) {}

SocketHub::~SocketHub() = default;

void SocketHub::start(int port) { impl_->start(port); }

void SocketHub::stop() { impl_->stop(); }

void SocketHub::addHandler(
    const std::function<void(const std::string&, size_t)>& handler) {
    impl_->addHandler(handler);
}

void SocketHub::addConnectHandler(const std::function<void(size_t)>& handler) {
    impl_->addConnectHandler(handler);
}

void SocketHub::addDisconnectHandler(
    const std::function<void(size_t)>& handler) {
    impl_->addDisconnectHandler(handler);
}

void SocketHub::broadcastMessage(const std::string& message) {
    impl_->broadcastMessage(message);
}

void SocketHub::sendMessageToClient(size_t client_id,
                                    const std::string& message) {
    impl_->sendMessageToClient(client_id, message);
}

auto SocketHub::isRunning() const -> bool { return impl_->isRunning(); }

// Definitions for Impl
void SocketHub::Impl::start(int port) {
    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    is_running_ = true;
    doAccept();

    io_thread_ = std::thread([this]() { io_context_.run(); });
    log("SocketHub started.");
}

void SocketHub::Impl::stop() {
    if (is_running_) {
        is_running_ = false;
        io_context_.stop();
        disconnectAllClients();
        if (io_thread_.joinable())
            io_thread_.join();
        log("SocketHub stopped.");
    }
}

void SocketHub::Impl::addHandler(
    const std::function<void(const std::string&, size_t)>& handler) {
    std::lock_guard<std::mutex> lock(handler_mutex_);
    handlers_.push_back(handler);
}

void SocketHub::Impl::addConnectHandler(
    const std::function<void(size_t)>& handler) {
    std::lock_guard<std::mutex> lock(connect_handler_mutex_);
    connect_handlers_.push_back(handler);
}

void SocketHub::Impl::addDisconnectHandler(
    const std::function<void(size_t)>& handler) {
    std::lock_guard<std::mutex> lock(disconnect_handler_mutex_);
    disconnect_handlers_.push_back(handler);
}

void SocketHub::Impl::broadcastMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(client_mutex_);
    for (const auto& [id, socket] : clients_) {
        asio::async_write(*socket, asio::buffer(message),
                          [](std::error_code ec, std::size_t) {
                              if (ec) {
                                  std::cerr
                                      << "Broadcast error: " << ec.message()
                                      << std::endl;
                              }
                          });
    }
    log("Broadcasted message: " + message);
}

void SocketHub::Impl::sendMessageToClient(size_t client_id,
                                          const std::string& message) {
    std::lock_guard<std::mutex> lock(client_mutex_);
    auto it = clients_.find(client_id);
    if (it != clients_.end()) {
        asio::async_write(*it->second, asio::buffer(message),
                          [](std::error_code ec, std::size_t) {
                              if (ec) {
                                  std::cerr << "Send error: " << ec.message()
                                            << std::endl;
                              }
                          });
        log("Sent message to client " + std::to_string(client_id) + ": " +
            message);
    }
}

[[nodiscard]] auto SocketHub::Impl::isRunning() const -> bool {
    return is_running_;
}

// Private members and methods
void SocketHub::Impl::doAccept() {
    auto socket = std::make_shared<asio::ip::tcp::socket>(io_context_);
    acceptor_.async_accept(*socket, [this, socket](std::error_code ec) {
        if (!ec) {
            handleNewConnection(socket);
            doRead(socket);
            log("New client connected.");
        }
        if (is_running_) {
            doAccept();
        }
    });
}

void SocketHub::Impl::handleNewConnection(
    std::shared_ptr<asio::ip::tcp::socket> socket) {
    std::lock_guard<std::mutex> lock(client_mutex_);
    size_t client_id = next_client_id_++;
    clients_[client_id] = socket;
    for (const auto& handler : connect_handlers_) {
        handler(client_id);
    }
}

void SocketHub::Impl::doRead(std::shared_ptr<asio::ip::tcp::socket> socket) {
    auto buffer = std::make_shared<std::vector<char>>(1024);
    socket->async_read_some(
        asio::buffer(*buffer),
        [this, socket, buffer](std::error_code ec, std::size_t length) {
            if (!ec) {
                std::string message(buffer->data(), length);
                handleIncomingMessage(message, socket);
                doRead(socket);
            } else {
                handleDisconnect(socket);
            }
        });
}

void SocketHub::Impl::handleIncomingMessage(
    const std::string& message, std::shared_ptr<asio::ip::tcp::socket> socket) {
    size_t client_id = getClientId(socket);
    std::lock_guard<std::mutex> lock(handler_mutex_);
    for (const auto& handler : handlers_) {
        handler(message, client_id);
    }
    log("Received message from client " + std::to_string(client_id) + ": " +
        message);
}

void SocketHub::Impl::handleDisconnect(
    std::shared_ptr<asio::ip::tcp::socket> socket) {
    size_t client_id = getClientId(socket);
    {
        std::lock_guard<std::mutex> lock(client_mutex_);
        clients_.erase(client_id);
    }
    for (const auto& handler : disconnect_handlers_) {
        handler(client_id);
    }
    log("Client " + std::to_string(client_id) + " disconnected.");
}

void SocketHub::Impl::disconnectAllClients() {
    std::lock_guard<std::mutex> lock(client_mutex_);
    for (auto& [id, socket] : clients_) {
        socket->close();
    }
    clients_.clear();
}

size_t SocketHub::Impl::getClientId(
    const std::shared_ptr<asio::ip::tcp::socket>& socket) {
    std::lock_guard<std::mutex> lock(client_mutex_);
    for (const auto& [id, sock] : clients_) {
        if (sock == socket) {
            return id;
        }
    }
    return 0;  // Should not happen unless the socket is not tracked (edge case)
}

void SocketHub::Impl::log(const std::string& message) {
    // Simple console logging
    std::cout << "[SocketHub] " << message << std::endl;
}

}  // namespace atom::async::connection
