/*
 * python.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Lithium Python scripting engine

**************************************************/

#include "pocketpy/pocketpy.h"

#include "config/configor.hpp"

using namespace pkpy;

namespace Lithium
{
    class PyScriptManager
    {
    public:
        PyScriptManager(/* args */);
        ~PyScriptManager();

        void InjectSystemModule();
        void InjectDeviceModule();
        void InjectConfigModule();
    

    private:
        VM* vm;
        PyObject* m_deviceModule;
        PyObject* m_systemModule;
        PyObject* m_configModule;

        std::shared_ptr<ConfigManager> m_coofigManager;
    };

} // namespace Lithium
