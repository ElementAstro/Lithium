#include "dome.hpp"
#include <chrono>
#include <stdexcept>
#include <thread>

AlpacaDome::AlpacaDome(std::string_view address, int device_number,
                       std::string_view protocol)
    : AlpacaDevice(std::string(address), "dome", device_number,
                   std::string(protocol)) {}

double AlpacaDome::GetAltitude() { return GetProperty<double>("altitude"); }
bool AlpacaDome::GetAtHome() { return GetProperty<bool>("athome"); }
bool AlpacaDome::GetAtPark() { return GetProperty<bool>("atpark"); }
double AlpacaDome::GetAzimuth() { return GetProperty<double>("azimuth"); }
bool AlpacaDome::GetCanFindHome() { return GetProperty<bool>("canfindhome"); }
bool AlpacaDome::GetCanPark() { return GetProperty<bool>("canpark"); }
bool AlpacaDome::GetCanSetAltitude() {
    return GetProperty<bool>("cansetaltitude");
}
bool AlpacaDome::GetCanSetAzimuth() {
    return GetProperty<bool>("cansetazimuth");
}
bool AlpacaDome::GetCanSetPark() { return GetProperty<bool>("cansetpark"); }
bool AlpacaDome::GetCanSetShutter() {
    return GetProperty<bool>("cansetshutter");
}
bool AlpacaDome::GetCanSlave() { return GetProperty<bool>("canslave"); }
bool AlpacaDome::GetCanSyncAzimuth() {
    return GetProperty<bool>("cansyncazimuth");
}
AlpacaDome::ShutterState AlpacaDome::GetShutterStatus() {
    return static_cast<ShutterState>(GetProperty<int>("shutterstatus"));
}
bool AlpacaDome::GetSlaved() { return GetProperty<bool>("slaved"); }
void AlpacaDome::SetSlaved(bool SlavedState) {
    Put("slaved", {{"Slaved", SlavedState ? "true" : "false"}});
}
bool AlpacaDome::GetSlewing() { return GetProperty<bool>("slewing"); }

void AlpacaDome::AbortSlew() { Put("abortslew"); }

template <typename Func>
std::future<void> AlpacaDome::AsyncOperation(Func&& func,
                                             const std::string& operationName) {
    return std::async(
        std::launch::async,
        [this, func = std::forward<Func>(func), operationName]() {
            func();
            while (GetSlewing() ||
                   (operationName == "shutter" &&
                    (GetShutterStatus() == ShutterState::ShutterOpening ||
                     GetShutterStatus() == ShutterState::ShutterClosing))) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
}

std::future<void> AlpacaDome::CloseShutter() {
    return AsyncOperation([this]() { Put("closeshutter"); }, "shutter");
}

std::future<void> AlpacaDome::FindHome() {
    return AsyncOperation([this]() { Put("findhome"); }, "home");
}

std::future<void> AlpacaDome::OpenShutter() {
    return AsyncOperation([this]() { Put("openshutter"); }, "shutter");
}

std::future<void> AlpacaDome::Park() {
    return AsyncOperation([this]() { Put("park"); }, "park");
}

void AlpacaDome::SetPark() { Put("setpark"); }

std::future<void> AlpacaDome::SlewToAltitude(double Altitude) {
    return AsyncOperation(
        [this, Altitude]() {
            Put("slewtoaltitude", {{"Altitude", std::to_string(Altitude)}});
        },
        "slew");
}

std::future<void> AlpacaDome::SlewToAzimuth(double Azimuth) {
    return AsyncOperation(
        [this, Azimuth]() {
            Put("slewtoazimuth", {{"Azimuth", std::to_string(Azimuth)}});
        },
        "slew");
}

void AlpacaDome::SyncToAzimuth(double Azimuth) {
    Put("synctoazimuth", {{"Azimuth", std::to_string(Azimuth)}});
}