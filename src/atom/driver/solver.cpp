/*
 * solver.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: Basic Solver Defination

*************************************************/

#include "solver.hpp"

Solver::Solver(const std::string &name) : Device(name)
{
}

Solver::~Solver()
{
}

bool Solver::connect(const nlohmann::json &params)
{
    return true;
}

bool Solver::disconnect(const nlohmann::json &params)
{
    return true;
}

bool Solver::reconnect(const nlohmann::json &params)
{
    return true;
}

bool Solver::solve_image(const nlohmann::json &params)
{
    return true;
}

bool Solver::get_solve_params(const nlohmann::json &params)
{
    return true;
}

bool Solver::get_solve_result(const nlohmann::json &params)
{
    return true;
}

bool Solver::get_solve_status(const nlohmann::json &params)
{
    return true;
}

bool Solver::set_solve_params(const nlohmann::json &params)
{
    return true;
}