/*
 * solver.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: AtomSolver Simulator and Basic Definition

*************************************************/

#pragma once

#include "device.hpp"

#include <optional>

class AtomSolver : public AtomDriver {
public:
    explicit AtomSolver(std::string name) : AtomDriver(name) {}


};
