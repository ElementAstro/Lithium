/*
 * lithiumapp.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Lithium App Enter

**************************************************/

#pragma once

#define LITHIUM_APP_MAIN

#include <memory>

#include "atom/components/dispatch.hpp"
#include "atom/server/message_bus.hpp"
#include "atom/type/message.hpp"
#include "atom/type/json.hpp"
using json = nlohmann::json;

// -------------------------------------------------------------------
// About the LithiumApp
// This is the main class of the Lithium App. All of the functions can be
// executed here. NOTE: No wrapper functions needed, just use the functions
// directly.
//       A json object is used to pass parameters to the functions.
//       And the return value is a json object.
//       Sometimes I think it is unnecessary to use json object to pass
//       parameters. However, It is more convenient to use json object.
// -------------------------------------------------------------------

namespace atom
{
    namespace error
    {
        class ErrorStack;
    }

    namespace system
    {
        class ProcessManager;
    }
}
namespace lithium {
class PyScriptManager;  // FWD

class CarbonScript;

class ComponentManager; // FWD

class ConfigManager;

class TaskPool;

class TaskManager;

class LithiumApp {
public:
    LithiumApp();
    ~LithiumApp();

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    static std::shared_ptr<LithiumApp> createShared();

private:
    std::weak_ptr<TaskPool> m_TaskPool;
    std::weak_ptr<atom::server::MessageBus> m_MessageBus;
    std::weak_ptr<atom::error::ErrorStack> m_ErrorStack;
    std::weak_ptr<ComponentManager> m_ComponentManager;
    std::weak_ptr<TaskManager> m_TaskManager;

    std::weak_ptr<PyScriptManager> m_PyScriptManager;
    std::weak_ptr<CarbonScript> m_CarbonScript;
};
extern std::shared_ptr<LithiumApp> MyApp;

void InitLithiumApp(int argc, char **argv);
}  // namespace lithium
