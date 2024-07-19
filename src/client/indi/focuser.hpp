#ifndef LITHIUM_CLIENT_INDI_FOCUSER_HPP
#define LITHIUM_CLIENT_INDI_FOCUSER_HPP

#include <libindi/baseclient.h>
#include <libindi/basedevice.h>

#include <atomic>
#include <optional>
#include <string_view>

class INDIFocuser : public INDI::BaseClient {
public:
    explicit INDIFocuser(std::string name);
    ~INDIFocuser() override = default;

    auto connect(const std::string &deviceName) -> bool;
    auto disconnect() -> void;
    auto reconnect() -> bool;

    virtual auto watchAdditionalProperty() -> bool;

    void setPropertyNumber(std::string_view propertyName, double value);

    auto getFocuserSpeed() -> std::optional<std::tuple<double, double, double>>;
    auto setFocuserSpeed(int value) -> bool;

    auto getFocuserMoveDiretion() -> bool;
    auto setFocuserMoveDiretion(bool isDirectionIn) -> bool;

    auto getFocuserMaxLimit() -> std::optional<int>;
    auto setFocuserMaxLimit(int maxlimit) -> bool;

    auto getFocuserReverse() -> std::optional<bool>;
    auto setFocuserReverse(bool isReversed) -> bool;

    auto moveFocuserSteps(int steps) -> bool;
    auto moveFocuserToAbsolutePosition(int position) -> bool;
    auto getFocuserAbsolutePosition() -> std::optional<double>;
    auto moveFocuserWithTime(int msec) -> bool;
    auto abortFocuserMove() -> bool;
    auto syncFocuserPosition(int position) -> bool;
    auto getFocuserOutTemperature() -> std::optional<double>;
    auto getFocuserChipTemperature() -> std::optional<double>;

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