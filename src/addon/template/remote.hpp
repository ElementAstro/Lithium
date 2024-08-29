#ifndef LITHIUM_ADDON_REMOTE_STANDALONE_HPP
#define LITHIUM_ADDON_REMOTE_STANDALONE_HPP

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "atom/components/component.hpp"

class RemoteStandAloneComponentImpl;

class RemoteStandAloneComponent : public Component {
public:
    explicit RemoteStandAloneComponent(std::string name);
    ~RemoteStandAloneComponent() override;

    void connectToRemoteDriver(const std::string& address, uint16_t port,
                               std::optional<int> timeout = std::nullopt);

    void disconnectRemoteDriver();

    void sendMessageToDriver(std::string_view message);

    void sendMessageAsync(
        std::string_view message,
        std::function<void(std::error_code, std::size_t)> callback);

    void setOnMessageReceivedCallback(
        std::function<void(std::string_view)> callback);

    void setOnDisconnectedCallback(std::function<void()> callback);

    void setOnConnectedCallback(std::function<void()> callback);

    void enableHeartbeat(int interval_ms, std::string_view pingMessage);

    void disableHeartbeat();

    void printDriver() const;

    void toggleDriverListening();

    void executeCommand(std::string_view command,
                        std::function<void(std::string_view)> callback);

private:
    void backgroundProcessing();

    void monitorConnection();

    void processMessages();

    void startHeartbeat();

    void stopHeartbeat();

    std::unique_ptr<RemoteStandAloneComponentImpl> impl_;
};

#endif  // LITHIUM_ADDON_REMOTE_STANDALONE_HPP
