/*
 * focuser.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Focuser Simulator and Basic Definition

**************************************************/

#include "focuser.hpp"

Focuser::Focuser(const std::string &name) : AtomDriver(name)
{
}

Focuser::~Focuser()
{
}

bool Focuser::connect(const json &params)
{
    return true;
}

bool Focuser::disconnect(const json &params)
{
    return true;
}

bool Focuser::reconnect(const json &params)
{
    return true;
}

bool Focuser::isConnected()
{
    return true;
}

bool Focuser::moveTo(const json &params)
{
    return true;
}

bool Focuser::moveToAbsolute(const json &params)
{
    return true;
}

bool Focuser::moveStep(const json &params)
{
    return true;
}

bool Focuser::moveStepAbsolute(const json &params)
{
    return true;
}

bool Focuser::AbortMove(const json &params)
{
    return true;
}

int Focuser::getMaxPosition(const json &params)
{
    return 0;
}

bool Focuser::setMaxPosition(const json &params)
{
    return true;
}

bool Focuser::isGetTemperatureAvailable(const json &params)
{
    return true;
}

double Focuser::getTemperature(const json &params)
{
    return 0.0;
}

bool Focuser::isAbsoluteMoveAvailable(const json &params)
{
    return true;
}

bool Focuser::isManualMoveAvailable(const json &params)
{
    return true;
}

int Focuser::getCurrentPosition(const json &params)
{
    return 0;
}

bool Focuser::haveBacklash(const json &params)
{
    return true;
}

bool Focuser::setBacklash(const json &params)
{
    return true;
}