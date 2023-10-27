/*
 * solver.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

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