#pragma once

#include <future>
#include <string>
#include <vector>
#include "device.hpp"

class AlpacaFilterWheel : public AlpacaDevice {
public:
    AlpacaFilterWheel(std::string_view address, int device_number,
                      std::string_view protocol = "http");
    ~AlpacaFilterWheel() override = default;

    // Properties
    auto getFocusOffsets() -> std::vector<int>;
    auto getNames() -> std::vector<std::string>;
    auto getPosition() -> int;
    auto setPosition(int position) -> std::future<void>;

    // Disable copy and move constructors and assignment operators
    AlpacaFilterWheel(const AlpacaFilterWheel&) = delete;
    AlpacaFilterWheel& operator=(const AlpacaFilterWheel&) = delete;
    AlpacaFilterWheel(AlpacaFilterWheel&&) = delete;
    AlpacaFilterWheel& operator=(AlpacaFilterWheel&&) = delete;

private:
    static constexpr int filterMoving = -1;

    std::future<void> positionChangeFuture_;

    void waitForFilterChange();
};