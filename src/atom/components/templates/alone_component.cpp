/*
 * alone_component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Alone component(run in different process)

**************************************************/

#include "alone_component.hpp"
#include "atom/components/macro.hpp"

#include "atom/log/loguru.hpp"
#include "atom/utils/random.hpp"

#include <fstream>
#include <sstream>

AloneComponent::AloneComponent(const std::string &name)
    : Component(name)
{

}