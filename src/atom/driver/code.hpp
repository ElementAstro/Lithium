/*
 * code.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-8-10

Description: All of the driver error code

**************************************************/

#ifndef ATOM_DRVIER_CODE_HPP
#define ATOM_DRVIER_CODE_HPP

enum class DeviceError {
    None,
    NotSpecific,
    NotFound,
    NotSupported,
    NotConnected,
    MissingValue,
    InvalidValue,
    Busy,

    // For Telescope
    GotoError,
    ParkError,
    UnParkError,
    ParkedError,
    HomeError
};

enum class CameraError {
    // For Camera
    ExposureError,
    GainError,
    OffsetError,
    ISOError,
    CoolingError,
};

enum class DeviceWarning {
    // For Camera
    ExposureWarning,
    GainWarning,
    OffsetWarning,
    ISOWarning,
    CoolingWarning,

    // For Telescope
    GotoWarning,
    ParkWarning,
    UnParkWarning,
    ParkedWarning,
    HomeWarning
};

#endif
