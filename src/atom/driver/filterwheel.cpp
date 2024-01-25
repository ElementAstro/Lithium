/*
 * filterwheel.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Filterwheel Simulator and Basic Definition

**************************************************/

#include "filterwheel.hpp"

#include "atom/log/loguru.hpp"

Filterwheel::Filterwheel(const std::string &name) : Device(name)
{
    init();
}

Filterwheel::~Filterwheel()
{
}

bool Filterwheel::connect(const json &params)
{
    return true;
}

bool Filterwheel::disconnect(const json &params)
{
    return true;
}

bool Filterwheel::reconnect(const json &params)
{
    return true;
}

bool Filterwheel::isConnected()
{
    return true;
}

bool Filterwheel::moveTo(const json &params)
{
    return true;
}

bool Filterwheel::getCurrentPosition(const json &params)
{
    return true;
}