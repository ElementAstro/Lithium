#ifndef LITHIUM_ADDON_REMOTE_STANDALONE_HPP
#define LITHIUM_ADDON_REMOTE_STANDALONE_HPP

#include <chrono>
#include <concepts>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <string>

#include "atom/components/component.hpp"

enum class ProtocolType { TCP, UDP };

template <typename T>
concept Stringlike = std::is_convertible_v<T, std::string_view>;

class RemoteStandAloneComponentImpl;

class RemoteStandAloneComponent : public Component {
public:
    explicit RemoteStandAloneComponent(std::string name);
    ~RemoteStandAloneComponent() override;

    void connectToRemoteDriver(
        const std::string& address, uint16_t port,
        ProtocolType protocol = ProtocolType::TCP,
        std::chrono::milliseconds timeout = std::chrono::seconds(5));

    void disconnectRemoteDriver();

    template <Stringlike T>
    void sendMessageToDriver(T&& message);

    template <Stringlike T>
    std::future<std::pair<std::error_code, std::size_t>> sendMessageAsync(
        T&& message);

    void setOnMessageReceivedCallback(
        std::function<void(std::string_view)> callback);

    void setOnDisconnectedCallback(std::function<void()> callback);

    void setOnConnectedCallback(std::function<void()> callback);

    void enableHeartbeat(std::chrono::milliseconds interval,
                         std::string_view pingMessage);

    void disableHeartbeat();

    void printDriver() const;

    void toggleDriverListening();

    template <Stringlike T>
    std::future<std::string> executeCommand(T&& command);

    void setReconnectionStrategy(std::chrono::milliseconds initialDelay,
                                 std::chrono::milliseconds maxDelay,
                                 int maxAttempts);

private:
    void backgroundProcessing();
    void monitorConnection();
    void processMessages();
    void startHeartbeat();
    void stopHeartbeat();
    void attemptReconnection();

    std::unique_ptr<RemoteStandAloneComponentImpl> impl_;
};

#endif  // LITHIUM_ADDON_REMOTE_STANDALONE_HPP