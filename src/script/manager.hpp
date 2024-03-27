/*
 * python.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Lithium Python scripting engine

**************************************************/

#ifndef LITHIUM_SCRIPT_PYTHON_HPP
#define LITHIUM_SCRIPT_PYTHON_HPP

#include "pocketpy/include/pocketpy.h"

#include "config/configor.hpp"

namespace Lithium {

class PyManagerImpl;

class PyScriptManager {
public:
    PyScriptManager(/* args */);
    ~PyScriptManager();

private:
    std::unique_ptr<PyManagerImpl> m_impl;
};

}  // namespace Lithium

#endif
