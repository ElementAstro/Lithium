#include "filterwheel.hpp"

#include <chrono>
#include <thread>

AlpacaFilterWheel::AlpacaFilterWheel(std::string_view address,
                                     int device_number,
                                     std::string_view protocol)
    : AlpacaDevice(std::string(address), "filterwheel", device_number,
                   std::string(protocol)) {}

auto AlpacaFilterWheel::getFocusOffsets() -> std::vector<int> {
    return getArrayProperty<int>("focusoffsets");
}

auto AlpacaFilterWheel::getNames() -> std::vector<std::string> {
    return getArrayProperty<std::string>("names");
}

auto AlpacaFilterWheel::getPosition() -> int {
    return getNumericProperty<int>("position");
}

auto AlpacaFilterWheel::setPosition(int position) -> std::future<void> {
    put("position", {{"Position", std::to_string(position)}});

    return std::async(std::launch::async, [this]() { waitForFilterChange(); });
}

void AlpacaFilterWheel::waitForFilterChange() {
    static constexpr int filterMoving = -1;
    static constexpr int pollIntervalMs = 100;

    while (getPosition() == filterMoving) {
        std::this_thread::sleep_for(std::chrono::milliseconds(pollIntervalMs));
    }
}