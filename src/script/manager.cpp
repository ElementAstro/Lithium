/*
 * python.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Lithium Python scripting engine

**************************************************/

#include "manager.hpp"

#include "pocketpy/include/pocketpy.h"
using namespace pkpy;

#include "atom/log/loguru.hpp"

#include "atom/io/io.hpp"

namespace lithium {
class PyManagerImpl {
public:
    PyManagerImpl() : vm(new VM()) {}
    ~PyManagerImpl() { delete vm;}
    VM *vm;
};

PyScriptManager::PyScriptManager(/* args */)
    : m_impl(std::make_unique<PyManagerImpl>()) {}

PyScriptManager::~PyScriptManager() {  }

}  // namespace lithium
