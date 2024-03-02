/*
 * generator.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-21

Description: Task Generator

**************************************************/

#include "generator.hpp"

#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"

#include "atom/server/global_ptr.hpp"

#include <fstream>
#include <future>

namespace Lithium {
TaskGenerator::TaskGenerator() {
    m_DeviceManager = GetPtr<DeviceManager>("lithium.device");
}

std::shared_ptr<TaskGenerator> TaskGenerator::createShared() {
    return std::make_shared<TaskGenerator>();
}

bool TaskGenerator::loadMacros(const std::string &macroFileName) {
    try {
        if (!Atom::IO::isFileExists(macroFileName)) {
            LOG_F(ERROR, "Macro file not found: {}", macroFileName);
            return false;
        }

        json macros;
        std::ifstream file(macroFileName);
        if (!file) {
            LOG_F(ERROR, "Failed to open macro file: {}", macroFileName);
            return false;
        }
        file >> macros;
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to parse file {}, error: {}", macroFileName,
              e.what());
        return false;
    }

    return true;
}

bool TaskGenerator::loadMacrosFromFolder(const std::string &folderPath) {
    if (!Atom::IO::isFolderExists(folderPath)) {
        LOG_F(ERROR, "Invalid folder path: {}", folderPath);
        return false;
    }

    std::vector<std::future<void>> futures;
    for (const auto &entry : fs::directory_iterator(folderPath)) {
        futures.push_back(std::async(std::launch::async,
                                     &TaskGenerator::processMacroFile, this,
                                     entry.path().string()));
    }

    for (auto &fut : futures) {
        fut.wait();
    }

    return true;
}

bool TaskGenerator::addMacro(const std::string &name,
                             const std::string &content) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_MacroMap[name] = content;
    return true;
}

bool TaskGenerator::deleteMacro(const std::string &name) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return (m_MacroMap.erase(name) > 0);
}

std::optional<std::string> TaskGenerator::getMacroContent(
    const std::string &name) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    const auto it = m_MacroMap.find(name);
    if (it != m_MacroMap.end()) {
        return it->second;
    } else {
        LOG_F(ERROR, "Macro with name {} not found.", name);
        return std::nullopt;
    }
}

void TaskGenerator::processMacroFile(const std::string &sfilePath) {
    if (!Atom::IO::isFileExists(sfilePath)) {
        LOG_F(ERROR, "Macro file not found: {}", sfilePath);
        return;
    }
    fs::path filePath = sfilePath;
    if (fs::is_regular_file(filePath) && filePath.extension() == ".json") {
        std::ifstream file(filePath);
        if (!file) {
            LOG_F(ERROR, "Failed to open macro file: {}", filePath.string());
            return;
        }

        json jsonMacro;
        file >> jsonMacro;

        if (jsonMacro.is_object()) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            for (const auto &[name, content] : jsonMacro.items()) {
                if (content.is_string()) {
                    m_MacroMap[name] = content.get<std::string>();
                }
            }
        } else {
            LOG_F(ERROR, "Invalid macro file format: {}", filePath.string());
        }
    }
}

bool TaskGenerator::generateTasks(const std::string &jsonFileName) {
    json jsonTasks;
    if (!parseJsonFile(jsonFileName, jsonTasks)) {
        return false;
    }

    // 从 DeviceManager 和 PluginManager
    // 获取任务（仅提供接口，需要实现该部分逻辑）

    // 将解析得到的任务添加到 TaskManager 中
    // 将任务清单保存为 JSON 格式
    std::string outputJsonFileName = jsonFileName + ".json";
    saveTasksToJson(outputJsonFileName, jsonTasks);

    return true;
}

bool TaskGenerator::parseJsonFile(const std::string &jsonFileName,
                                  json &jsonTasks) {
    try {
        std::ifstream file(jsonFileName);
        if (!file) {
            LOG_F(ERROR, "Failed to open JSON file: {}", jsonFileName);
            return false;
        }
        file >> jsonTasks;
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Error while parsing JSON file: {}", e.what());
        return false;
    }

    return true;
}

void TaskGenerator::saveTasksToJson(const std::string &jsonFileName,
                                    const json &jsonTasks) {
    try {
        std::ofstream jsonFile(jsonFileName);
        if (!jsonFile) {
            LOG_F(ERROR, "Failed to open JSON file: {}", jsonFileName);
            return;
        }
        jsonFile << jsonTasks.dump(4);  // 使用四个空格缩进
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Error while saving JSON file: {}", e.what());
        return;
    }
}
}  // namespace Lithium
