/*
 * focuser.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: AtomFocuser Simulator and Basic Definition

*************************************************/

#pragma once

#include "device.hpp"

#include <optional>

enum class BAUD_RATE { B9600, B19200, B38400, B57600, B115200, B230400, NONE };

enum class FocusMode { ALL, ABSOLUTE, RELATIVE, NONE };

enum class FocusDirection { IN, OUT, NONE };

class AtomFocuser : public AtomDriver {
public:
    explicit AtomFocuser(std::string name) : AtomDriver(name) {}
    virtual auto getFocuserSpeed()
        -> std::optional<std::tuple<double, double, double>> = 0;
    virtual auto setFocuserSpeed(int value) -> bool = 0;

    virtual auto getFocuserMoveDirection() -> bool = 0;
    virtual auto setFocuserMoveDirection(bool isDirectionIn) -> bool = 0;

    virtual auto getFocuserMaxLimit() -> std::optional<int> = 0;
    virtual auto setFocuserMaxLimit(int maxlimit) -> bool = 0;

    virtual auto getFocuserReverse() -> std::optional<bool> = 0;
    virtual auto setFocuserReverse(bool isReversed) -> bool = 0;

    virtual auto moveFocuserSteps(int steps) -> bool = 0;
    virtual auto moveFocuserToAbsolutePosition(int position) -> bool = 0;
    virtual auto getFocuserAbsolutePosition() -> std::optional<double> = 0;
    virtual auto moveFocuserWithTime(int msec) -> bool = 0;
    virtual auto abortFocuserMove() -> bool = 0;
    virtual auto syncFocuserPosition(int position) -> bool = 0;
    virtual auto getFocuserOutTemperature() -> std::optional<double> = 0;
    virtual auto getFocuserChipTemperature() -> std::optional<double> = 0;
};
