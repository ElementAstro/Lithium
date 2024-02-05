/*
 * solver.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: Basic Solver Defination

*************************************************/

#pragma once

#include "device.hpp"

class Solver : public AtomDriver
{
public:
    Solver(const std::string &name);
    ~Solver();

    virtual bool connect(const nlohmann::json &params) override;

    virtual bool disconnect(const nlohmann::json &params) override;

    virtual bool reconnect(const nlohmann::json &params) override;

public:
    virtual bool solve_image(const nlohmann::json &params);

    virtual bool get_solve_result(const nlohmann::json &params);

    virtual bool get_solve_status(const nlohmann::json &params);

    virtual bool set_solve_params(const nlohmann::json &params);

    virtual bool get_solve_params(const nlohmann::json &params);
};