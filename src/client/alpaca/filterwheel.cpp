#include <chrono>
#include <stdexcept>
#include <thread>

#include "filterwheel.hpp"

AlpacaFilterWheel::AlpacaFilterWheel(std::string_view address,
                                     int device_number,
                                     std::string_view protocol)
    : AlpacaDevice(std::string(address), "filterwheel", device_number,
                   std::string(protocol)) {}

std::vector<int> AlpacaFilterWheel::GetFocusOffsets() {
    return GetArrayProperty<int>("focusoffsets");
}

std::vector<std::string> AlpacaFilterWheel::GetNames() {
    return GetArrayProperty<std::string>("names");
}

int AlpacaFilterWheel::GetPosition() {
    return GetNumericProperty<int>("position");
}

std::future<void> AlpacaFilterWheel::SetPosition(int Position) {
    Put("position", {{"Position", std::to_string(Position)}});

    return std::async(std::launch::async,
                      [this, Position]() { WaitForFilterChange(); });
}

void AlpacaFilterWheel::WaitForFilterChange() {
    while (GetPosition() == FILTER_MOVING) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}