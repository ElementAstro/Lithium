#include "remote.hpp"

#include <asio.hpp>
#include <atomic>
#include <chrono>
#include <random>
#include <string>
#include <string_view>
#include <thread>

#include "atom/log/loguru.hpp"

using asio::ip::tcp;
using asio::ip::udp;

class RemoteStandAloneComponentImpl {
public:
    std::string driverName;
    std::atomic<bool> shouldExit{false};
    std::jthread driverThread;
    std::variant<std::optional<tcp::socket>, std::optional<udp::socket>> socket;
    std::optional<asio::ip::basic_endpoint<asio::ip::tcp>> tcpEndpoint;
    std::optional<asio::ip::basic_endpoint<asio::ip::udp>> udpEndpoint;
    asio::io_context ioContext;
    asio::steady_timer heartbeatTimer{ioContext};
    std::atomic<bool> isListening{false};
    std::function<void(std::string_view)> onMessageReceived;
    std::function<void()> onDisconnected;
    std::function<void()> onConnected;
    std::chrono::milliseconds heartbeatInterval{0};
    std::string heartbeatMessage;
    std::atomic<bool> heartbeatEnabled{false};
    ProtocolType protocol{ProtocolType::TCP};

    // Reconnection strategy
    std::chrono::milliseconds initialReconnectDelay{1000};
    std::chrono::milliseconds maxReconnectDelay{30000};
    int maxReconnectAttempts{5};
    int currentReconnectAttempts{0};

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
        "TCP or UDP");
    def("connect", &RemoteStandAloneComponent::connectToRemoteDriver);
    def("disconnect", &RemoteStandAloneComponent::disconnectRemoteDriver);
    def("send", &RemoteStandAloneComponent::sendMessageToDriver<std::string>);
    def("send_async",
        &RemoteStandAloneComponent::sendMessageAsync<std::string>);
    def("listen", &RemoteStandAloneComponent::toggleDriverListening);
    def("print", &RemoteStandAloneComponent::printDriver);
    def("heartbeat_on", &RemoteStandAloneComponent::enableHeartbeat);
    def("heartbeat_off", &RemoteStandAloneComponent::disableHeartbeat);
    def("execute", &RemoteStandAloneComponent::executeCommand<std::string>);
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
    const std::string& address, uint16_t port, ProtocolType protocol,
    std::chrono::milliseconds timeout) {
    impl_->protocol = protocol;
    try {
        switch (protocol) {
            case ProtocolType::TCP: {
                tcp::resolver resolver(impl_->ioContext);
                auto endpoints =
                    resolver.resolve(address, std::to_string(port));
                impl_->socket.emplace<std::optional<tcp::socket>>(
                    impl_->ioContext);
                auto& tcpSocket =
                    std::get<std::optional<tcp::socket>>(impl_->socket).value();

                asio::steady_timer timer(impl_->ioContext);
                timer.expires_after(timeout);
                asio::error_code ec = asio::error::operation_aborted;

                tcpSocket.async_connect(*endpoints,
                                        [&](const asio::error_code& error) {
                                            ec = error;
                                            timer.cancel();
                                        });

                timer.async_wait([&](const asio::error_code& error) {
                    if (!error) {
                        tcpSocket.cancel();
                    }
                });

                impl_->ioContext.run_one();

                if (ec) {
                    throw asio::system_error(ec);
                }

                impl_->tcpEndpoint = *endpoints;
                break;
            }
            case ProtocolType::UDP: {
                udp::resolver resolver(impl_->ioContext);
                auto endpoint =
                    *resolver.resolve(udp::v4(), address, std::to_string(port))
                         .begin();
                impl_->socket.emplace<std::optional<udp::socket>>(
                    impl_->ioContext, udp::endpoint(udp::v4(), 0));
                auto& udpSocket =
                    std::get<std::optional<udp::socket>>(impl_->socket).value();
                udpSocket.connect(endpoint);
                impl_->udpEndpoint = endpoint;
                break;
            }
        }

        if (impl_->onConnected)
            impl_->onConnected();

        LOG_F(INFO, "Connected to remote driver at {}:{} using {}", address,
              port, protocol == ProtocolType::TCP ? "TCP" : "UDP");

        impl_->driverThread = std::jthread(
            &RemoteStandAloneComponent::backgroundProcessing, this);
    } catch (std::exception& e) {
        LOG_F(ERROR, "Failed to connect to remote driver: {}", e.what());
        if (impl_->onDisconnected)
            impl_->onDisconnected();
    }
}

void RemoteStandAloneComponent::disconnectRemoteDriver() {
    std::visit(
        [](auto&& socket) {
            if (socket && socket->is_open()) {
                asio::error_code ec;
                socket->shutdown(std::decay_t<decltype(*socket)>::shutdown_both,
                                 ec);
                socket->close(ec);
            }
        },
        impl_->socket);

    LOG_F(INFO, "Disconnected from remote driver");
    if (impl_->onDisconnected)
        impl_->onDisconnected();

    impl_->shouldExit = true;
}

template <Stringlike T>
void RemoteStandAloneComponent::sendMessageToDriver(T&& message) {
    std::visit(
        [&](auto&& socket) {
            if (socket && socket->is_open()) {
                asio::write(*socket, asio::buffer(std::forward<T>(message)));
            } else {
                LOG_F(ERROR, "No active connection to send message");
            }
        },
        impl_->socket);
}

template <Stringlike T>
std::future<std::pair<std::error_code, std::size_t>>
RemoteStandAloneComponent::sendMessageAsync(T&& message) {
    auto promise = std::make_shared<
        std::promise<std::pair<std::error_code, std::size_t>>>();
    auto future = promise->get_future();

    std::visit(
        [&](auto&& socket) {
            if (socket && socket->is_open()) {
                asio::async_write(
                    *socket, asio::buffer(std::forward<T>(message)),
                    [promise](const asio::error_code& ec,
                              std::size_t bytes_transferred) {
                        promise->set_value({ec, bytes_transferred});
                    });
            } else {
                promise->set_value({asio::error::not_connected, 0});
            }
        },
        impl_->socket);

    return future;
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

void RemoteStandAloneComponent::enableHeartbeat(
    std::chrono::milliseconds interval, std::string_view pingMessage) {
    impl_->heartbeatInterval = interval;
    impl_->heartbeatMessage = pingMessage;
    impl_->heartbeatEnabled = true;
    startHeartbeat();
}

void RemoteStandAloneComponent::disableHeartbeat() {
    impl_->heartbeatEnabled = false;
    stopHeartbeat();
}

void RemoteStandAloneComponent::printDriver() const {
    if (impl_->tcpEndpoint) {
        LOG_F(INFO, "Remote Driver (TCP): {}:{}",
              impl_->tcpEndpoint->address().to_string(),
              impl_->tcpEndpoint->port());
    } else if (impl_->udpEndpoint) {
        LOG_F(INFO, "Remote Driver (UDP): {}:{}",
              impl_->udpEndpoint->address().to_string(),
              impl_->udpEndpoint->port());
    } else {
        LOG_F(INFO, "No remote driver connected");
    }
}

void RemoteStandAloneComponent::toggleDriverListening() {
    impl_->isListening = !impl_->isListening;
    LOG_F(INFO, "Driver listening status: {}",
          impl_->isListening ? "ON" : "OFF");
}

template <Stringlike T>
std::future<std::string> RemoteStandAloneComponent::executeCommand(
    T&& command) {
    auto promise = std::make_shared<std::promise<std::string>>();
    auto future = promise->get_future();

    sendMessageAsync(std::forward<T>(command))
        .then([this, promise](auto&& result) {
            auto [ec, _] = result.get();
            if (!ec) {
                std::array<char, 1024> buffer;
                std::size_t len = 0;

                std::visit(
                    [&](auto&& socket) {
                        if constexpr (std::is_same_v<
                                          std::decay_t<decltype(socket)>,
                                          std::optional<tcp::socket>>) {
                            asio::error_code error;
                            len =
                                socket->read_some(asio::buffer(buffer), error);
                            if (error) {
                                promise->set_exception(std::make_exception_ptr(
                                    std::runtime_error(error.message())));
                                return;
                            }
                        } else if constexpr (std::is_same_v<
                                                 std::decay_t<decltype(socket)>,
                                                 std::optional<udp::socket>>) {
                            udp::endpoint sender_endpoint;
                            asio::error_code error;
                            len =
                                socket->receive_from(asio::buffer(buffer),
                                                     sender_endpoint, 0, error);
                            if (error) {
                                promise->set_exception(std::make_exception_ptr(
                                    std::runtime_error(error.message())));
                                return;
                            }
                        }
                    },
                    impl_->socket);

                promise->set_value(std::string(buffer.data(), len));
            } else {
                promise->set_exception(
                    std::make_exception_ptr(std::runtime_error(ec.message())));
            }
        });

    return future;
}

void RemoteStandAloneComponent::setReconnectionStrategy(
    std::chrono::milliseconds initialDelay, std::chrono::milliseconds maxDelay,
    int maxAttempts) {
    impl_->initialReconnectDelay = initialDelay;
    impl_->maxReconnectDelay = maxDelay;
    impl_->maxReconnectAttempts = maxAttempts;
}

void RemoteStandAloneComponent::backgroundProcessing() {
    while (!impl_->shouldExit) {
        monitorConnection();
        processMessages();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void RemoteStandAloneComponent::monitorConnection() {
    bool isConnected =
        std::visit([](auto&& socket) { return socket && socket->is_open(); },
                   impl_->socket);

    if (!isConnected && !impl_->shouldExit) {
        LOG_F(INFO, "Connection lost. Attempting to reconnect...");
        attemptReconnection();
    }
}

void RemoteStandAloneComponent::processMessages() {
    if (!impl_->isListening)
        return;

    std::visit(
        [this](auto&& socket) {
            if constexpr (std::is_same_v<std::decay_t<decltype(socket)>,
                                         std::optional<tcp::socket>>) {
                if (socket && socket->is_open()) {
                    std::array<char, 1024> buffer;
                    socket->async_read_some(
                        asio::buffer(buffer),
                        [this, buffer](const asio::error_code& error,
                                       std::size_t bytes_transferred) {
                            if (!error) {
                                impl_->handleDriverOutput(std::string_view(
                                    buffer.data(), bytes_transferred));
                            } else if (error == asio::error::eof) {
                                LOG_F(INFO,
                                      "Connection closed by remote driver");
                                disconnectRemoteDriver();
                            } else {
                                LOG_F(ERROR, "Read error: {}", error.message());
                            }
                        });
                }
            } else if constexpr (std::is_same_v<std::decay_t<decltype(socket)>,
                                                std::optional<udp::socket>>) {
                if (socket && socket->is_open()) {
                    std::array<char, 1024> buffer;
                    udp::endpoint sender_endpoint;
                    socket->async_receive_from(
                        asio::buffer(buffer), sender_endpoint,
                        [this, buffer](const asio::error_code& error,
                                       std::size_t bytes_transferred) {
                            if (!error) {
                                impl_->handleDriverOutput(std::string_view(
                                    buffer.data(), bytes_transferred));
                            } else {
                                LOG_F(ERROR, "Read error: {}", error.message());
                            }
                        });
                }
            }
        },
        impl_->socket);
}

void RemoteStandAloneComponent::startHeartbeat() {
    if (!impl_->heartbeatEnabled)
        return;

    impl_->heartbeatTimer.expires_after(impl_->heartbeatInterval);
    impl_->heartbeatTimer.async_wait([this](const asio::error_code& error) {
        if (!error && impl_->heartbeatEnabled) {
            sendMessageAsync(impl_->heartbeatMessage).then([this](auto future) {
                auto [ec, _] = future.get();
                if (ec) {
                    LOG_F(ERROR, "Failed to send heartbeat: {}", ec.message());
                    attemptReconnection();
                } else {
                    startHeartbeat();
                }
            });
        }
    });
}

void RemoteStandAloneComponent::stopHeartbeat() {
    impl_->heartbeatEnabled = false;
    impl_->heartbeatTimer.cancel();
}

void RemoteStandAloneComponent::attemptReconnection() {
    if (impl_->currentReconnectAttempts >= impl_->maxReconnectAttempts) {
        LOG_F(ERROR, "Max reconnection attempts reached. Giving up.");
        return;
    }

    std::chrono::milliseconds delay = std::min(
        impl_->initialReconnectDelay * (1 << impl_->currentReconnectAttempts),
        impl_->maxReconnectDelay);

    LOG_F(INFO, "Attempting to reconnect in {} ms", delay.count());

    std::this_thread::sleep_for(delay);

    std::visit(
        [this](auto&& socket) {
            using SocketType = std::decay_t<decltype(*socket)>;
            if constexpr (std::is_same_v<SocketType, tcp::socket>) {
                connectToRemoteDriver(impl_->tcpEndpoint->address().to_string(),
                                      impl_->tcpEndpoint->port(),
                                      ProtocolType::TCP);
            } else if constexpr (std::is_same_v<SocketType, udp::socket>) {
                connectToRemoteDriver(impl_->udpEndpoint->address().to_string(),
                                      impl_->udpEndpoint->port(),
                                      ProtocolType::UDP);
            }
        },
        impl_->socket);

    impl_->currentReconnectAttempts++;
}

// Explicit template instantiations
template void RemoteStandAloneComponent::sendMessageToDriver<std::string>(
    std::string&&);
template void RemoteStandAloneComponent::sendMessageToDriver<std::string_view>(
    std::string_view&&);
template std::future<std::pair<std::error_code, std::size_t>>
RemoteStandAloneComponent::sendMessageAsync<std::string>(std::string&&);
template std::future<std::pair<std::error_code, std::size_t>>
RemoteStandAloneComponent::sendMessageAsync<std::string_view>(
    std::string_view&&);
template std::future<std::string>
RemoteStandAloneComponent::executeCommand<std::string>(std::string&&);
template std::future<std::string>
RemoteStandAloneComponent::executeCommand<std::string_view>(std::string_view&&);
