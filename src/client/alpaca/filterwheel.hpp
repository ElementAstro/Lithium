#pragma once

#include <future>
#include <string>
#include <vector>
#include "device.hpp"

class AlpacaFilterWheel : public AlpacaDevice {
public:
    AlpacaFilterWheel(std::string_view address, int device_number,
                      std::string_view protocol = "http");
    virtual ~AlpacaFilterWheel() = default;

    // Properties
    std::vector<int> GetFocusOffsets();
    std::vector<std::string> GetNames();
    int GetPosition();
    std::future<void> SetPosition(int Position);

private:
    static constexpr int FILTER_MOVING = -1;

    std::future<void> m_position_change_future;

    void WaitForFilterChange();
};