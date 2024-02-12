#pragma once

enum class DeviceType
{
    Camera,
    Telescope,
    Focuser,
    FilterWheel,
    Solver,
    Guider,
    NumDeviceTypes
};

inline const char *DeviceTypeToString(DeviceType type)
{
    switch (type)
    {
    case DeviceType::Camera:
        return "Camera";
        break;
    case DeviceType::Telescope:
        return "Telescope";
        break;
    case DeviceType::Focuser:
        return "Focuser";
        break;
    case DeviceType::FilterWheel:
        return "FilterWheel";
        break;
    case DeviceType::Solver:
        return "Solver";
        break;
    case DeviceType::Guider:
        return "Guider";
        break;
    default:
        return "Unknown";
        break;
    }
    return "Unknown";
}

inline DeviceType StringToDeviceType(const std::string &type)
{
    if (type == "Camera")
        return DeviceType::Camera;
    else if (type == "Telescope")
        return DeviceType::Telescope;
    else if (type == "Focuser")
        return DeviceType::Focuser;
    else if (type == "FilterWheel")
        return DeviceType::FilterWheel;
    else if (type == "Solver")
        return DeviceType::Solver;
    else if (type == "Guider")
        return DeviceType::Guider;
    else
        return DeviceType::NumDeviceTypes;
}