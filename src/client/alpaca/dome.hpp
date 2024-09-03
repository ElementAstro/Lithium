#pragma once

#include <future>
#include <optional>
#include <string>
#include "device.hpp"

class AlpacaDome : public AlpacaDevice {
public:
    enum class ShutterState {
        ShutterOpen = 0,
        ShutterClosed = 1,
        ShutterOpening = 2,
        ShutterClosing = 3,
        ShutterError = 4
    };

    AlpacaDome(std::string_view address, int device_number,
               std::string_view protocol = "http");
    virtual ~AlpacaDome() = default;

    // Properties
    double GetAltitude();
    bool GetAtHome();
    bool GetAtPark();
    double GetAzimuth();
    bool GetCanFindHome();
    bool GetCanPark();
    bool GetCanSetAltitude();
    bool GetCanSetAzimuth();
    bool GetCanSetPark();
    bool GetCanSetShutter();
    bool GetCanSlave();
    bool GetCanSyncAzimuth();
    ShutterState GetShutterStatus();
    bool GetSlaved();
    void SetSlaved(bool SlavedState);
    bool GetSlewing();

    // Methods
    void AbortSlew();
    std::future<void> CloseShutter();
    std::future<void> FindHome();
    std::future<void> OpenShutter();
    std::future<void> Park();
    void SetPark();
    std::future<void> SlewToAltitude(double Altitude);
    std::future<void> SlewToAzimuth(double Azimuth);
    void SyncToAzimuth(double Azimuth);

private:
    template <typename T>
    T GetProperty(const std::string& property) const {
        return GetNumericProperty<T>(property);
    }

    template <typename Func>
    std::future<void> AsyncOperation(Func&& func,
                                     const std::string& operationName);
};