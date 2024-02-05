/*
 * guider.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Guider Simulator and Basic Definition

**************************************************/

#include "guider.hpp"

Guider::Guider(const std::string &name) : AtomDriver(name)
{
}

Guider::~Guider()
{
}

bool Guider::connect(const nlohmann::json &params)
{
    return true;
}

bool Guider::disconnect(const nlohmann::json &params)
{
    return true;
}

bool Guider::reconnect(const nlohmann::json &params)
{
    return true;
}