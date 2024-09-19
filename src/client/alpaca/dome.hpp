#pragma once

#include <future>
#include <string>
#include "device.hpp"

class AlpacaDome : public AlpacaDevice {
public:
    enum class ShutterState {
        shutterOpen = 0,
        shutterClosed = 1,
        shutterOpening = 2,
        shutterClosing = 3,
        shutterError = 4
    };

    AlpacaDome(std::string_view address, int device_number,
               std::string_view protocol = "http");
    ~AlpacaDome() override = default;

    // Properties
    auto getAltitude() -> double;
    auto getAtHome() -> bool;
    auto getAtPark() -> bool;
    auto getAzimuth() -> double;
    auto getCanFindHome() -> bool;
    auto getCanPark() -> bool;
    auto getCanSetAltitude() -> bool;
    auto getCanSetAzimuth() -> bool;
    auto getCanSetPark() -> bool;
    auto getCanSetShutter() -> bool;
    auto getCanSlave() -> bool;
    auto getCanSyncAzimuth() -> bool;
    auto getShutterStatus() -> ShutterState;
    auto getSlaved() -> bool;
    void setSlaved(bool slavedState);
    auto getSlewing() -> bool;

    // Methods
    void abortSlew();
    auto closeShutter() -> std::future<void>;
    auto findHome() -> std::future<void>;
    auto openShutter() -> std::future<void>;
    auto park() -> std::future<void>;
    void setPark();
    auto slewToAltitude(double altitude) -> std::future<void>;
    auto slewToAzimuth(double azimuth) -> std::future<void>;
    void syncToAzimuth(double azimuth);

private:
    template <typename T>
    auto getProperty(const std::string& property) const -> T {
        return getNumericProperty<T>(property);
    }

    template <typename Func>
    auto asyncOperation(Func&& func,
                        const std::string& operationName) -> std::future<void>;
};
