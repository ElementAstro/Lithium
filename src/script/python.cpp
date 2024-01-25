/*
 * python.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Lithium Python scripting engine

**************************************************/

#include "python.hpp"

#include "atom/log/loguru.hpp"

#include "atom/system/system.hpp"

#include "atom/io/io.hpp"

namespace Lithium
{
    PyScriptManager::PyScriptManager(/* args */)
        : vm(new VM()),
          m_deviceModule(vm->new_module("lithium_device")),
          m_systemModule(vm->new_module("lithium_system")),
          m_configModule(vm->new_module("lithium_config"))
    {
    }

    PyScriptManager::~PyScriptManager()
    {
    }

} // namespace Lithium
