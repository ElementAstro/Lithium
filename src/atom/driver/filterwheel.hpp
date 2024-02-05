/*
 * filterwheel.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: Basic FilterWheel Defination

*************************************************/

#pragma once

#include "device.hpp"

class Filterwheel : public AtomDriver
{
public:
    Filterwheel(const std::string &name);
    ~Filterwheel();

    virtual bool connect(const json &params) override;

    virtual bool disconnect(const json &params) override;

    virtual bool reconnect(const json &params) override;

    virtual bool isConnected() override;

    virtual bool moveTo(const json &params);

    virtual bool getCurrentPosition(const json &params);
};