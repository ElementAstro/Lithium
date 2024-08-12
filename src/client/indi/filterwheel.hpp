#ifndef LITHIUM_CLIENT_INDI_FILTERWHEEL_HPP
#define LITHIUM_CLIENT_INDI_FILTERWHEEL_HPP

#include <libindi/baseclient.h>
#include <libindi/basedevice.h>

#include <atomic>
#include <optional>
#include <string_view>

class INDIFilterwheel : public INDI::BaseClient {
public:
    explicit INDIFilterwheel(std::string name);
    ~INDIFilterwheel() override = default;

    auto connect(const std::string &deviceName) -> bool;
    auto disconnect() -> void;
    auto reconnect() -> bool;

    virtual auto watchAdditionalProperty() -> bool;

    void setPropertyNumber(std::string_view propertyName, double value);

    auto getCFWPosition() -> std::optional<std::tuple<double, double, double>>;
    auto setCFWPosition(int position) -> bool;
    auto getCFWSlotName() -> std::optional<std::string>;
    auto setCFWSlotName(std::string_view name) -> bool;

protected:
    void newMessage(INDI::BaseDevice baseDevice, int messageID) override;

private:
    std::string name_;
    std::string deviceName_;

    std::string driverExec_;
    std::string driverVersion_;
    std::string driverInterface_;
    bool deviceAutoSearch_;
    bool devicePortScan_;

    std::atomic<double> currentPollingPeriod_;

    std::atomic_bool isDebug_;

    std::atomic_bool isConnected_;

    INDI::BaseDevice device_;
};

#endif
