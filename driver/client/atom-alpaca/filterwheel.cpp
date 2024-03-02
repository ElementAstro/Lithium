#include "filterwheel.hpp"
#include "exception.hpp"

Filterwheel::Filterwheel(const std::string &address int device_number,
                         const std::string &protocol)
    : Device(address, "filterwheel", device_number, protocol) {}

std::vector<int> Filterwheel::get_FocusOffsets() const {
    std::vector<int> focus_offsets;
    json j = _get("focusoffsets");
    for (auto it = j.begin(); it != j.end(); ++it) {
        focus_offsets.push_back(it.value());
    }
    return focus_offsets;
}

std::vector<std::string> Filterwheel::get_Names() const {
    std::vector<std::string> names;
    json j = _get("names");
    for (auto it = j.begin(); it != j.end(); ++it) {
        names.push_back(it.value());
    }
    return names;
}

int Filterwheel::get_Positions() const {
    int position = -1;
    try {
        position = _get("position").get<int>();
    } catch (const std::exception &e) {
        throw InvalidValueException("position")
    }
    return position;
}

void Filterwheel::set_Position(int position) { _put("position", position); }