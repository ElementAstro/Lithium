#ifndef LITHIUM_ADDON_REMOTE_STANDALONE_HPP
#define LITHIUM_ADDON_REMOTE_STANDALONE_HPP

#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "atom/async/future.hpp"
#include "atom/components/component.hpp"
#include "atom/function/concept.hpp"

enum class ProtocolType { TCP, UDP };

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

    template <String T>
    void sendMessageToDriver(T&& message);

    template <typename T>
    auto sendMessageAsync(T&& message)
        -> atom::async::EnhancedFuture<std::pair<std::error_code, std::size_t>>;

    void setOnMessageReceivedCallback(
        std::function<void(std::string_view)> callback);

    void setOnDisconnectedCallback(std::function<void()> callback);

    void setOnConnectedCallback(std::function<void()> callback);

    void enableHeartbeat(std::chrono::milliseconds interval,
                         std::string_view pingMessage);

    void disableHeartbeat();

    void printDriver() const;

    void toggleDriverListening();

    template <String T>
    auto executeCommand(T&& command)
        -> atom::async::EnhancedFuture<std::string>;

    void setReconnectionStrategy(std::chrono::milliseconds initialDelay,
                                 std::chrono::milliseconds maxDelay,
                                 int maxAttempts);

    void enableSSL(const std::string& certFile, const std::string& keyFile);

    void disableSSL();

    void enableCompression();

    void disableCompression();

    void authenticate(const std::string& username, const std::string& password);

    atom::async::EnhancedFuture<std::string> GetStatus();

    atom::async::EnhancedFuture<bool> RestartDriver();

    atom::async::EnhancedFuture<bool> UpdateConfig(const std::string& config);

    void initializeRPC();

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
