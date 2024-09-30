#include "async_tcpclient.hpp"

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace atom::async::connection {

class TcpClient::Impl {
public:
    Impl(bool use_ssl)
        : io_context_(),
          ssl_context_(asio::ssl::context::sslv23),
          socket_(use_ssl ? (asio::ip::tcp::socket(io_context_))
                          : (ssl_socket_t(io_context_, ssl_context_))),
          use_ssl_(use_ssl),
          connected_(false),
          reconnect_attempts_(0),
          heartbeat_interval_(5000),
          total_bytes_sent_(0),
          total_bytes_received_(0) {
        if (use_ssl_) {
            ssl_context_.set_verify_mode(asio::ssl::verify_peer);
        }
    }

    ~Impl() { disconnect(); }

    bool connect(
        const std::string& host, int port,
        std::chrono::milliseconds timeout = std::chrono::milliseconds::zero()) {
        last_host_ = host;
        last_port_ = port;

        try {
            asio::ip::tcp::resolver resolver(io_context_);
            auto endpoints = resolver.resolve(host, std::to_string(port));

            asio::error_code ec;
            asio::connect(socket_, endpoints, ec);

            if (ec) {
                logError(ec.message());
                return false;
            }

            if (use_ssl_) {
                asio::error_code ssl_ec;
                socket_.handshake(asio::ssl::stream_base::client, ssl_ec);
                if (ssl_ec) {
                    logError(ssl_ec.message());
                    return false;
                }
            }

            connected_ = true;
            if (on_connected_)
                on_connected_();

            startReceiving(1024);
            startHeartbeat();

            io_thread_ = std::thread([this]() { io_context_.run(); });

            logInfo("Connected to server.");
            return true;
        } catch (const std::exception& e) {
            logError(e.what());
            return false;
        }
    }

    void disconnect() {
        if (connected_) {
            if (use_ssl_) {
                socket_.lowest_layer().close();
            } else {
                socket_.lowest_layer().close();
            }
            connected_ = false;
            if (on_disconnected_)
                on_disconnected_();
            logInfo("Disconnected from server.");
        }

        if (io_thread_.joinable()) {
            io_context_.stop();
            io_thread_.join();
        }
    }

    void enableReconnection(int attempts) { reconnect_attempts_ = attempts; }

    void setHeartbeatInterval(std::chrono::milliseconds interval) {
        heartbeat_interval_ = interval;
    }

    bool send(const std::vector<char>& data) {
        if (!connected_) {
            logError("Not connected to any server.");
            return false;
        }

        try {
            auto bytes_written = asio::write(socket_, asio::buffer(data));
            total_bytes_sent_ += bytes_written;
            logInfo("Sent data of size: " + std::to_string(bytes_written));
            return true;
        } catch (const std::exception& e) {
            logError(e.what());
            return false;
        }
    }

    std::future<std::vector<char>> receive(
        size_t size,
        std::chrono::milliseconds timeout = std::chrono::milliseconds::zero()) {
        return std::async(std::launch::async, [=, this]() {
            std::vector<char> data(size);
            try {
                auto bytes_read = asio::read(socket_, asio::buffer(data, size));
                total_bytes_received_ += bytes_read;
                logInfo("Received data of size: " + std::to_string(bytes_read));
                return data;
            } catch (const std::exception& e) {
                logError(e.what());
            }
            return data;
        });
    }

    [[nodiscard]] bool isConnected() const { return connected_; }

    [[nodiscard]] std::string getErrorMessage() const { return last_error_; }

    void setOnConnectedCallback(const OnConnectedCallback& callback) {
        on_connected_ = callback;
    }

    void setOnDisconnectedCallback(const OnDisconnectedCallback& callback) {
        on_disconnected_ = callback;
    }

    void setOnDataReceivedCallback(const OnDataReceivedCallback& callback) {
        on_data_received_ = callback;
    }

    void setOnErrorCallback(const OnErrorCallback& callback) {
        on_error_ = callback;
    }

private:
    using ssl_socket_t = asio::ssl::stream<asio::ip::tcp::socket>;

    void startReceiving(size_t bufferSize) {
        receive_buffer_.resize(bufferSize);
        doReceive();
    }

    void doReceive() {
        socket_.async_read_some(
            asio::buffer(receive_buffer_),
            [this](std::error_code ec, std::size_t length) {
                if (!ec) {
                    total_bytes_received_ += length;
                    if (on_data_received_) {
                        on_data_received_(std::vector<char>(
                            receive_buffer_.begin(),
                            receive_buffer_.begin() + length));
                    }
                    doReceive();
                } else {
                    handleDisconnect(ec.message());
                }
            });
    }

    void startHeartbeat() {
        heartbeat_timer_.expires_after(heartbeat_interval_);
        heartbeat_timer_.async_wait([this](const std::error_code& ec) {
            if (!ec && connected_) {
                send(std::vector<char>{'P'});  // Example ping message
                startHeartbeat();              // Re-schedule the heartbeat
            }
        });
    }

    void handleDisconnect(const std::string& error) {
        connected_ = false;
        if (on_disconnected_)
            on_disconnected_();

        logError("Disconnected due to: " + error);

        reconnect();
    }

    void reconnect() {
        int attempts = 0;
        while (attempts < reconnect_attempts_ && !connected_) {
            attempts++;
            if (connect(last_host_, last_port_)) {
                logInfo("Reconnected after " + std::to_string(attempts) +
                        " attempts.");
                return;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1) *
                                        attempts);
        }

        if (!connected_ && on_error_) {
            on_error_("Reconnection failed after " +
                      std::to_string(reconnect_attempts_) + " attempts.");
        }
    }

    void logInfo(const std::string& message) {
        std::cout << "[INFO] " << message << std::endl;
    }

    void logError(const std::string& message) {
        std::cerr << "[ERROR] " << message << std::endl;
        last_error_ = message;
    }

    asio::io_context io_context_;
    asio::ssl::context ssl_context_;
    ssl_socket_t socket_;
    asio::steady_timer heartbeat_timer_{io_context_};
    std::thread io_thread_;

    bool use_ssl_;
    bool connected_;
    std::string last_error_;
    std::vector<char> receive_buffer_;

    std::string last_host_;
    int last_port_;

    OnConnectedCallback on_connected_;
    OnDisconnectedCallback on_disconnected_;
    OnDataReceivedCallback on_data_received_;
    OnErrorCallback on_error_;

    int reconnect_attempts_;
    std::chrono::milliseconds heartbeat_interval_;

    std::atomic<size_t> total_bytes_sent_;
    std::atomic<size_t> total_bytes_received_;
};

TcpClient::TcpClient(bool use_ssl) : impl_(std::make_unique<Impl>(use_ssl)) {}

TcpClient::~TcpClient() = default;

bool TcpClient::connect(const std::string& host, int port,
                        std::chrono::milliseconds timeout) {
    return impl_->connect(host, port, timeout);
}

void TcpClient::disconnect() { impl_->disconnect(); }

void TcpClient::enableReconnection(int attempts) {
    impl_->enableReconnection(attempts);
}

void TcpClient::setHeartbeatInterval(std::chrono::milliseconds interval) {
    impl_->setHeartbeatInterval(interval);
}

bool TcpClient::send(const std::vector<char>& data) {
    return impl_->send(data);
}

std::future<std::vector<char>> TcpClient::receive(
    size_t size, std::chrono::milliseconds timeout) {
    return impl_->receive(size, timeout);
}

bool TcpClient::isConnected() const { return impl_->isConnected(); }

std::string TcpClient::getErrorMessage() const {
    return impl_->getErrorMessage();
}

void TcpClient::setOnConnectedCallback(const OnConnectedCallback& callback) {
    impl_->setOnConnectedCallback(callback);
}

void TcpClient::setOnDisconnectedCallback(
    const OnDisconnectedCallback& callback) {
    impl_->setOnDisconnectedCallback(callback);
}

void TcpClient::setOnDataReceivedCallback(
    const OnDataReceivedCallback& callback) {
    impl_->setOnDataReceivedCallback(callback);
}

void TcpClient::setOnErrorCallback(const OnErrorCallback& callback) {
    impl_->setOnErrorCallback(callback);
}

}  // namespace atom::async::connection
