#include "remote.hpp"

#include <asio.hpp>

#include <string>
#include <string_view>
#include <thread>

#include "atom/log/loguru.hpp"

using asio::ip::tcp;

class RemoteStandAloneComponentImpl {
public:
    std::string driverName;
    std::atomic<bool> shouldExit{false};
    std::jthread driverThread;
    std::optional<tcp::socket> socket;
    std::optional<tcp::endpoint> endpoint;
    asio::io_context ioContext;
    asio::steady_timer heartbeatTimer{ioContext};
    std::atomic<bool> isListening{false};
    std::function<void(std::string_view)> onMessageReceived;
    std::function<void()> onDisconnected;
    std::function<void()> onConnected;
    int heartbeatInterval{0};
    std::string heartbeatMessage;
    std::atomic<bool> heartbeatEnabled{false};

    void handleDriverOutput(std::string_view buffer) {
        if (onMessageReceived) {
            onMessageReceived(buffer);
        } else {
            LOG_F(INFO, "Output from remote driver: {}", buffer);
        }
    }
};

RemoteStandAloneComponent::RemoteStandAloneComponent(std::string name)
    : Component(std::move(name)),
      impl_(std::make_unique<RemoteStandAloneComponentImpl>()) {
    doc("A remote standalone component that can connect to a remote driver via "
        "TCP");
    def("connect", &RemoteStandAloneComponent::connectToRemoteDriver);
    def("disconnect", &RemoteStandAloneComponent::disconnectRemoteDriver);
    def("send", &RemoteStandAloneComponent::sendMessageToDriver);
    def("send_async", &RemoteStandAloneComponent::sendMessageAsync);
    def("listen", &RemoteStandAloneComponent::toggleDriverListening);
    def("print", &RemoteStandAloneComponent::printDriver);
    def("heartbeat_on", &RemoteStandAloneComponent::enableHeartbeat);
    def("heartbeat_off", &RemoteStandAloneComponent::disableHeartbeat);
}

RemoteStandAloneComponent::~RemoteStandAloneComponent() {
    LOG_F(INFO, "Component {} destroyed", getName());
    disconnectRemoteDriver();
    impl_->shouldExit = true;
    if (impl_->driverThread.joinable()) {
        impl_->driverThread.join();
    }
}

void RemoteStandAloneComponent::connectToRemoteDriver(
    const std::string& address, uint16_t port, std::optional<int> timeout) {
    try {
        tcp::resolver resolver(impl_->ioContext);
        auto endpoints = resolver.resolve(address, std::to_string(port));
        impl_->socket.emplace(impl_->ioContext);

        if (timeout) {
            asio::steady_timer timer(impl_->ioContext);
            timer.expires_after(std::chrono::milliseconds(*timeout));
            asio::error_code ec = asio::error::operation_aborted;

            impl_->socket->async_connect(*endpoints,
                                         [&](const asio::error_code& error) {
                                             ec = error;
                                             timer.cancel();
                                         });

            timer.async_wait([&](const asio::error_code& error) {
                if (!error) {
                    impl_->socket->cancel();
                }
            });

            impl_->ioContext.run_one();

            if (ec) {
                throw asio::system_error(ec);
            }
        } else {
            asio::connect(*impl_->socket, endpoints);
        }

        impl_->endpoint = *endpoints;
        if (impl_->onConnected)
            impl_->onConnected();

        LOG_F(INFO, "Connected to remote driver at {}:{}", address, port);

        impl_->driverThread = std::jthread(
            &RemoteStandAloneComponent::backgroundProcessing, this);
    } catch (std::exception& e) {
        LOG_F(ERROR, "Failed to connect to remote driver: {}", e.what());
        if (impl_->onDisconnected)
            impl_->onDisconnected();
    }
}

void RemoteStandAloneComponent::disconnectRemoteDriver() {
    if (impl_->socket && impl_->socket->is_open()) {
        asio::error_code ec;
        impl_->socket->shutdown(tcp::socket::shutdown_both, ec);
        impl_->socket->close(ec);
        if (ec) {
            LOG_F(ERROR, "Error closing connection: {}", ec.message());
        } else {
            LOG_F(INFO, "Disconnected from remote driver");
            if (impl_->onDisconnected)
                impl_->onDisconnected();
        }
    }
    impl_->shouldExit = true;
}

void RemoteStandAloneComponent::sendMessageToDriver(std::string_view message) {
    if (impl_->socket && impl_->socket->is_open()) {
        asio::write(*impl_->socket, asio::buffer(message));
    } else {
        LOG_F(ERROR, "No active connection to send message");
    }
}

void RemoteStandAloneComponent::sendMessageAsync(
    std::string_view message,
    std::function<void(std::error_code, std::size_t)> callback) {
    if (impl_->socket && impl_->socket->is_open()) {
        asio::async_write(*impl_->socket, asio::buffer(message), callback);
    } else {
        LOG_F(ERROR, "No active connection to send message");
    }
}

void RemoteStandAloneComponent::setOnMessageReceivedCallback(
    std::function<void(std::string_view)> callback) {
    impl_->onMessageReceived = std::move(callback);
}

void RemoteStandAloneComponent::setOnDisconnectedCallback(
    std::function<void()> callback) {
    impl_->onDisconnected = std::move(callback);
}

void RemoteStandAloneComponent::setOnConnectedCallback(
    std::function<void()> callback) {
    impl_->onConnected = std::move(callback);
}

void RemoteStandAloneComponent::enableHeartbeat(int interval_ms,
                                                std::string_view pingMessage) {
    impl_->heartbeatInterval = interval_ms;
    impl_->heartbeatMessage = pingMessage;
    impl_->heartbeatEnabled = true;
    startHeartbeat();
}

void RemoteStandAloneComponent::disableHeartbeat() {
    impl_->heartbeatEnabled = false;
    stopHeartbeat();
}

void RemoteStandAloneComponent::printDriver() const {
    if (impl_->endpoint) {
        LOG_F(INFO, "Remote Driver: {}:{}",
              impl_->endpoint->address().to_string(), impl_->endpoint->port());
    } else {
        LOG_F(INFO, "No remote driver connected");
    }
}

void RemoteStandAloneComponent::toggleDriverListening() {
    impl_->isListening = !impl_->isListening;
    LOG_F(INFO, "Driver listening status: {}",
          impl_->isListening ? "ON" : "OFF");
}

void RemoteStandAloneComponent::executeCommand(
    std::string_view command, std::function<void(std::string_view)> callback) {
    sendMessageAsync(command, [this, callback](std::error_code ec,
                                               std::size_t) {
        if (!ec) {
            std::array<char, 1024> buffer;
            asio::error_code error;
            size_t len = impl_->socket->read_some(asio::buffer(buffer), error);
            if (!error) {
                callback(std::string_view(buffer.data(), len));
            } else {
                LOG_F(ERROR, "Command execution failed: {}", error.message());
            }
        } else {
            LOG_F(ERROR, "Failed to send command: {}", ec.message());
        }
    });
}

void RemoteStandAloneComponent::backgroundProcessing() {
    while (!impl_->shouldExit) {
        monitorConnection();
        processMessages();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void RemoteStandAloneComponent::monitorConnection() {
    if (impl_->socket && impl_->socket->is_open()) {
        // 可以在这里添加更多逻辑来监控连接状态，比如检查连接的活跃性。
    } else if (!impl_->shouldExit) {
        LOG_F(INFO, "Connection lost. Attempting to reconnect...");
        disconnectRemoteDriver();
        // 根据需要添加自动重连的逻辑。
    }
}

void RemoteStandAloneComponent::processMessages() {
    if (impl_->socket && impl_->socket->is_open() && impl_->isListening) {
        std::array<char, 1024> buffer;
        asio::error_code error;
        size_t len = impl_->socket->read_some(asio::buffer(buffer), error);

        if (error == asio::error::eof) {
            LOG_F(INFO, "Connection closed by remote driver");
            disconnectRemoteDriver();
            if (impl_->onDisconnected) {
                impl_->onDisconnected();
            }
        } else if (error) {
            LOG_F(ERROR, "Read error: {}", error.message());
        } else {
            impl_->handleDriverOutput(std::string_view(buffer.data(), len));
        }
    }
}

void RemoteStandAloneComponent::startHeartbeat() {
    if (!impl_->heartbeatEnabled)
        return;

    impl_->heartbeatTimer.expires_after(
        std::chrono::milliseconds(impl_->heartbeatInterval));
    impl_->heartbeatTimer.async_wait([this](const asio::error_code& error) {
        if (!error && impl_->heartbeatEnabled) {
            sendMessageAsync(impl_->heartbeatMessage, [](std::error_code ec,
                                                         std::size_t) {
                if (ec) {
                    LOG_F(ERROR, "Failed to send heartbeat: {}", ec.message());
                }
            });
            startHeartbeat();
        }
    });
}

void RemoteStandAloneComponent::stopHeartbeat() {
    impl_->heartbeatEnabled = false;
    impl_->heartbeatTimer.cancel();
}
