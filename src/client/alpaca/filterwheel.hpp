#pragma once



#include "device.hpp"

class Filterwheel : public Device
{
public:
    Filterwheel(const std::string &address, int device_number, const std::string &protocol);
    
    std::vector<int> get_FocusOffsets() const;

    std::vector<std::string> get_Names() const;

    int get_Position() const;

    void set_Position(int position);
};