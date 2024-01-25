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

#include "atom/log/loguru.hpp"

Guider::Guider(const std::string &name) : Device(name)
{
    DLOG_F(INFO, "Guider Simulator Loaded : %s", name.c_str());
    init();
}

Guider::~Guider()
{
    DLOG_F(INFO, "Guider Simulator Destructed");
}

bool Guider::connect(const nlohmann::json &params)
{
    DLOG_F(INFO, "%s is connected", getDeviceName());
    return true;
}

bool Guider::disconnect(const nlohmann::json &params)
{
    DLOG_F(INFO, "%s is disconnected", getDeviceName());
    return true;
}

bool Guider::reconnect(const nlohmann::json &params)
{
    return true;
}