#ifndef LITHIUM_CLIENT_INDI_FILTERWHEEL_HPP
#define LITHIUM_CLIENT_INDI_FILTERWHEEL_HPP

#include <libindi/baseclient.h>
#include <libindi/basedevice.h>

#include "atom/utils/qdatetime.hpp"

#include <atomic>
#include <optional>
#include <string_view>

class INDIDevice : public INDI::BaseClient {
public:
    explicit INDIDevice(std::string name);
    ~INDIDevice() override = default;

    auto connect(const std::string &deviceName) -> bool;
    auto disconnect() -> void;
    auto reconnect() -> bool;

    virtual auto watchAdditionalProperty() -> bool;

    void setPropertyNumber(std::string_view propertyName, double value);

    auto getDevicePort() -> std::optional<std::string>;
    auto setDevicePort(std::string_view devicePort) -> bool;

    auto setTimeUTC(atom::utils::MyDateTime datetime) -> bool;
    auto getTimeUTC(atom::utils::MyDateTime &datetime) -> bool;
    auto setLocation(double latitude_degree, double longitude_degree,
                     double elevation) -> bool;
    auto getLocation(double &latitude_degree, double &longitude_degree,
                     double &elevation) -> bool;
    auto setAtmosphere(double temperature, double pressure,
                       double humidity) -> bool;
    auto getAtmosphere(double &temperature, double &pressure,
                       double &humidity) -> bool;

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
