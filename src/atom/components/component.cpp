/*
 * component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-8-6

Description: Basic Component Definition

**************************************************/

#include "component.hpp"

#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/utils/string.hpp"

#if __cplusplus >= 202002L
#include <format>
#else
#include <fmt/format.h>
#endif

Component::Component(const std::string &name)
    : m_CommandDispatcher(std::make_unique<CommandDispatcher<json, json>>()),
      m_VariableRegistry(std::make_unique<VariableRegistry>(name)),
      m_name(name) {
    if (!initialize()) {
        LOG_F(ERROR, "Failed to initialize {}", name);
    }
}

Component::~Component() { destroy(); }

bool Component::initialize() { return true; }

bool Component::destroy() {
    if (m_CommandDispatcher) {
        m_CommandDispatcher->RemoveAll();
        m_CommandDispatcher.reset();
    }
    if (m_VariableRegistry) {
        m_VariableRegistry->RemoveAll();
        m_VariableRegistry.reset();
    }
    return true;
}

std::string Component::getName() const { return m_name; }

bool Component::loadConfig(const std::string &path) {
    std::unique_lock<std::mutex> lock(m_mutex);
    try {
        std::ifstream ifs(path);
        if (!ifs.is_open()) {
            LOG_F(ERROR, "Failed to open file: {}", path);
            return false;
        }
        json j = json::parse(ifs);
        const std::string basename = fs::path(path).stem().string();
        m_config[basename] = j["config"];
        m_ConfigPath = path;
        DLOG_F(INFO, "Loaded config file {} successfully", path);
        return true;
    } catch (const json::exception &e) {
        LOG_F(ERROR, "Failed to parse file: {}, error message: {}", path,
              e.what());
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to load config file: {}, error message: {}", path,
              e.what());
    }
    return false;
}

bool Component::saveConfig() {
    if (m_ConfigPath.empty()) {
        LOG_F(ERROR, "No path provided, will not save {}'s config", m_name);
        return false;
    }
    std::ofstream ofs(m_ConfigPath);
    if (!ofs.is_open()) {
        LOG_F(ERROR, "Failed to open file: {}", m_ConfigPath);
        return false;
    }
    try {
        ofs << m_config.dump(4);
    } catch (const json::parse_error &e) {
        LOG_F(ERROR, "Failed to sace config {} for JSON error: {}",
              m_ConfigPath, e.what());
        ofs.close();
        return false;
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to save config to file: {}, error message: {}",
              m_ConfigPath, e.what());
        ofs.close();
        return false;
    }
    ofs.close();
    DLOG_F(INFO, "Save config to file: {}", m_ConfigPath);
    return true;
}

json Component::getValue(const std::string &key_path) const {
    // std::lock_guard<std::shared_mutex> lock(rw_m_mutex);
    try {
        const json *p = &m_config;
        for (const auto &key : Atom::Utils::splitString(key_path, '/')) {
            if (p->is_object() && p->contains(key)) {
                p = &(*p)[key];
            } else {
                LOG_F(ERROR, "Key not found: {}", key_path);
                return nullptr;
            }
        }
        return *p;
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to get value: {} {}", key_path, e.what());
        return nullptr;
    }
    return nullptr;
}

std::string Component::getVariableInfo(const std::string &name) const {
    if (!m_VariableRegistry->HasVariable(name)) {
        return "";
    }
    return m_VariableRegistry->GetDescription(name);
}

bool Component::runFunc(const std::string &name, const json &params) {
    if (!m_CommandDispatcher->HasHandler(name)) {
        return false;
    }
    m_CommandDispatcher->Dispatch(name, params);
    return true;
}

json Component::getFuncInfo(const std::string &name) {
    if (m_CommandDispatcher->HasHandler(name)) {
        json args;
        args = {
            {"name", name},
            {"description", m_CommandDispatcher->GetFunctionDescription(name)}};
        return args;
    }
    return {};
}

std::function<json(const json &)> Component::getFunc(const std::string &name) {
    if (!m_CommandDispatcher->HasHandler(name)) {
        throw Atom::Error::InvalidArgument("Function not found");
    }
    return m_CommandDispatcher->GetHandler(name);
}

json Component::createSuccessResponse(const std::string &command,
                                      const json &value) {
    json res;
    res["command"] = command;
    res["value"] = value;
    res["status"] = "ok";
    res["code"] = 200;
#if __cplusplus >= 202003L
    res["message"] = std::format("{} operated on success", command);
#else
    res["message"] = fmt::format("{} operated on success", command);
#endif
    return res;
}

json Component::createErrorResponse(const std::string &command,
                                    const json &error,
                                    const std::string &message = "") {
    json res;
    res["command"] = command;
    res["status"] = "error";
    res["code"] = 500;
#if __cplusplus >= 202003L
    res["message"] =
        std::format("{} operated on failure, message: {}", command, message);
#else
    res["message"] =
        std::format("{} operated on failure, message: {}", command, message);
#endif
    if (!error.empty()) {
        res["error"] = error;
    } else {
        res["error"] = "Unknown Error";
    }
    return res;
}

json Component::createWarningResponse(const std::string &command,
                                      const json &warning,
                                      const std::string &message = "") {
    json res;
    res["command"] = command;
    res["status"] = "warning";
    res["code"] = 400;
#if __cplusplus >= 202003L
    res["message"] =
        std::format("{} operated on warning, message: {}", command, message);
#else
    res["message"] =
        std::format("{} operated on warning, message: {}", command, message);
#endif
    if (!warning.empty()) {
        res["warning"] = warning;
    } else {
        res["warning"] = "Unknown Warning";
    }
    return res;
}