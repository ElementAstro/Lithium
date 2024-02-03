/*
 * generator.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-21

Description: Task Generator

**************************************************/

#ifndef LITHIUM_TASK_GENERATOR_HPP
#define LITHIUM_TASK_GENERATOR_HPP

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif
#include <fstream>
#include <optional>

#include "device/manager.hpp"
#include "atom/task/task.hpp"

using json = nlohmann::json;

namespace Lithium
{
    class TaskGenerator
    {
    public:
        explicit TaskGenerator(std::shared_ptr<DeviceManager> deviceManager);
        ~TaskGenerator() = default;

        // -------------------------------------------------------------------
        // Common methods
        // -------------------------------------------------------------------

        static std::shared_ptr<TaskGenerator> createShared(std::shared_ptr<DeviceManager> deviceManager);

        // -------------------------------------------------------------------
        // Macro methods
        // -------------------------------------------------------------------

        bool loadMacros(const std::string &macroFileName);
        bool loadMacrosFromFolder(const std::string &folderPath);
        bool addMacro(const std::string &name, const std::string &content);
        bool deleteMacro(const std::string &name);
        std::optional<std::string> getMacroContent(const std::string &name);

        // -------------------------------------------------------------------
        // Task methods
        // -------------------------------------------------------------------

        bool generateTasks(const std::string &jsonFileName);

    private:

        // -------------------------------------------------------------------
        // Task methods
        // -------------------------------------------------------------------

        bool parseJsonFile(const std::string &jsonFileName, json &jsonTasks);
        void saveTasksToJson(const std::string &jsonFileName, const json &jsonTasks);

        // -------------------------------------------------------------------
        // Macro methods
        // -------------------------------------------------------------------

        void processMacroFile(const std::string &sfilePath);

    private:
#if ENABLE_FASTHASH
        emhash8::HashMap<std::string, std::string> m_MacroMap;
#else
        std::unordered_map<std::string, std::string> m_MacroMap;
#endif

        std::mutex m_Mutex;

        std::shared_ptr<DeviceManager> m_DeviceManager;
    };

}
#endif
