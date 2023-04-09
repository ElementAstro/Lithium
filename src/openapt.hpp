/*
 * openapt.hpp
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
 
Date: 2023-3-27
 
Description: Main 
 
**************************************************/

#ifndef _OPENAPT_HPP_
#define _OPENAPT_HPP_

#include "crow.h"

#include "plugins/thread.hpp"
#include "task/runner.hpp"
#include "device/manager.hpp"
#include "module/modloader.hpp"
#include "module/lualoader.hpp"
#include "module/pythonloader.hpp"
#include "config/configor.hpp"
#include "package/packageloader.hpp"

extern crow::SimpleApp app;

extern OpenAPT::ThreadManager m_ThreadManager;
extern OpenAPT::TaskManager m_TaskManager;
extern OpenAPT::DeviceManager m_DeviceManager;
extern OpenAPT::ModuleLoader m_ModuleLoader;
extern OpenAPT::ConfigManager m_ConfigManager;
extern OpenAPT::PackageManager m_PackageManager;
extern OpenAPT::PyModuleLoader m_PythonLoader;
extern OpenAPT::LuaScriptLoader m_LuaLoader;

extern bool DEBUG;

#endif