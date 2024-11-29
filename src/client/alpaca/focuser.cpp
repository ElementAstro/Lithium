#include "focuser.hpp"

#include <chrono>
#include <stdexcept>
#include <thread>

AlpacaFocuser::AlpacaFocuser(const std::string& address, int device_number,
                             const std::string& protocol)
    : AlpacaDevice(address, "focuser", device_number, protocol) {}

AlpacaFocuser::~AlpacaFocuser() {
    if (moveThread_.joinable()) {
        moveThread_.join();
    }
}

auto AlpacaFocuser::getAbsolute() -> bool {
    return getNumericProperty<bool>("absolute");
}

auto AlpacaFocuser::getIsMoving() -> bool { return isMoving_.load(); }

auto AlpacaFocuser::getMaxIncrement() -> int {
    return getNumericProperty<int>("maxincrement");
}

auto AlpacaFocuser::getMaxStep() -> int {
    return getNumericProperty<int>("maxstep");
}

auto AlpacaFocuser::getPosition() -> int {
    return getNumericProperty<int>("position");
}

auto AlpacaFocuser::getStepSize() -> float {
    return getNumericProperty<float>("stepsize");
}

auto AlpacaFocuser::getTempComp() -> bool {
    return getNumericProperty<bool>("tempcomp");
}

void AlpacaFocuser::setTempComp(bool tempCompState) {
    put("tempcomp", {{"TempComp", tempCompState ? "true" : "false"}});
}

auto AlpacaFocuser::getTempCompAvailable() -> bool {
    return getNumericProperty<bool>("tempcompavailable");
}

auto AlpacaFocuser::getTemperature() -> std::optional<float> {
    try {
        return getNumericProperty<float>("temperature");
    } catch (const std::runtime_error& e) {
        // If temperature is not implemented, return nullopt
        return std::nullopt;
    }
}

void AlpacaFocuser::halt() {
    put("halt");
    isMoving_.store(false);
}

void AlpacaFocuser::startMove(int position) {
    put("move", {{"Position", std::to_string(position)}});
}

void AlpacaFocuser::moveThread(int position) {
    isMoving_.store(true);
    startMove(position);

    // Poll the isMoving property until the move is complete
    while (getNumericProperty<bool>("ismoving")) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    isMoving_.store(false);
}

auto AlpacaFocuser::move(int position) -> std::future<void> {
    // If a move is already in progress, wait for it to complete
    if (moveThread_.joinable()) {
        moveThread_.join();
    }

    // Start a new move thread
    moveThread_ = std::thread(&AlpacaFocuser::moveThread, this, position);

    // Return a future that will be ready when the move is complete
    return std::async(std::launch::deferred, [this]() {
        if (moveThread_.joinable()) {
            moveThread_.join();
        }
    });
}
