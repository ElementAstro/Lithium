#include "dome.hpp"

#include <chrono>
#include <thread>

AlpacaDome::AlpacaDome(std::string_view address, int device_number,
                       std::string_view protocol)
    : AlpacaDevice(std::string(address), "dome", device_number,
                   std::string(protocol)) {}

auto AlpacaDome::getAltitude() -> double {
    return getProperty<double>("altitude");
}

auto AlpacaDome::getAtHome() -> bool { return getProperty<bool>("athome"); }

auto AlpacaDome::getAtPark() -> bool { return getProperty<bool>("atpark"); }

auto AlpacaDome::getAzimuth() -> double {
    return getProperty<double>("azimuth");
}

auto AlpacaDome::getCanFindHome() -> bool {
    return getProperty<bool>("canfindhome");
}

auto AlpacaDome::getCanPark() -> bool { return getProperty<bool>("canpark"); }

auto AlpacaDome::getCanSetAltitude() -> bool {
    return getProperty<bool>("cansetaltitude");
}

auto AlpacaDome::getCanSetAzimuth() -> bool {
    return getProperty<bool>("cansetazimuth");
}

auto AlpacaDome::getCanSetPark() -> bool {
    return getProperty<bool>("cansetpark");
}

auto AlpacaDome::getCanSetShutter() -> bool {
    return getProperty<bool>("cansetshutter");
}

auto AlpacaDome::getCanSlave() -> bool { return getProperty<bool>("canslave"); }

auto AlpacaDome::getCanSyncAzimuth() -> bool {
    return getProperty<bool>("cansyncazimuth");
}

auto AlpacaDome::getShutterStatus() -> ShutterState {
    return static_cast<ShutterState>(getProperty<int>("shutterstatus"));
}

auto AlpacaDome::getSlaved() -> bool { return getProperty<bool>("slaved"); }

void AlpacaDome::setSlaved(bool slavedState) {
    put("slaved", {{"Slaved", slavedState ? "true" : "false"}});
}

auto AlpacaDome::getSlewing() -> bool { return getProperty<bool>("slewing"); }

void AlpacaDome::abortSlew() { put("abortslew"); }

template <typename Func>
auto AlpacaDome::asyncOperation(Func&& func, const std::string& operationName)
    -> std::future<void> {
    return std::async(
        std::launch::async,
        [this, func = std::forward<Func>(func), operationName]() {
            func();
            while (getSlewing() ||
                   (operationName == "shutter" &&
                    (getShutterStatus() == ShutterState::shutterOpening ||
                     getShutterStatus() == ShutterState::shutterClosing))) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
}

auto AlpacaDome::closeShutter() -> std::future<void> {
    return asyncOperation([this]() { put("closeshutter"); }, "shutter");
}

auto AlpacaDome::findHome() -> std::future<void> {
    return asyncOperation([this]() { put("findhome"); }, "home");
}

auto AlpacaDome::openShutter() -> std::future<void> {
    return asyncOperation([this]() { put("openshutter"); }, "shutter");
}

auto AlpacaDome::park() -> std::future<void> {
    return asyncOperation([this]() { put("park"); }, "park");
}

void AlpacaDome::setPark() { put("setpark"); }

auto AlpacaDome::slewToAltitude(double altitude) -> std::future<void> {
    return asyncOperation(
        [this, altitude]() {
            put("slewtoaltitude", {{"Altitude", std::to_string(altitude)}});
        },
        "slew");
}

auto AlpacaDome::slewToAzimuth(double azimuth) -> std::future<void> {
    return asyncOperation(
        [this, azimuth]() {
            put("slewtoazimuth", {{"Azimuth", std::to_string(azimuth)}});
        },
        "slew");
}

void AlpacaDome::syncToAzimuth(double azimuth) {
    put("synctoazimuth", {{"Azimuth", std::to_string(azimuth)}});
}
