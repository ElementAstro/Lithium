/*
 * python.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Lithium Python scripting engine

**************************************************/

#pragma once

#include "pocketpy/include/pocketpy.h"

#include "config/configor.hpp"

namespace Lithium {
class PyScriptManager {
public:
    PyScriptManager(/* args */);
    ~PyScriptManager();

    void InjectSystemModule();
    void InjectDeviceModule();
    void InjectConfigModule();

private:
    pkpy::VM* vm;
    pkpy::PyObject* m_deviceModule;
    pkpy::PyObject* m_systemModule;
    pkpy::PyObject* m_configModule;

    std::shared_ptr<ConfigManager> m_coofigManager;
};

}  // namespace Lithium
