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

Component::Component(const std::string& name)
    : m_name(name),
      m_ConfigManager(std::make_unique<ConfigManager>()),
      m_CommandDispatcher(std::make_unique<CommandDispatcher>()),
      m_VariableManager(std::make_unique<VariableManager>()),
      m_typeInfo(user_type<Component>()) {
    // Empty
}

Component::~Component() {
    // Empty
}

bool Component::initialize() { return true; }

bool Component::destroy() { return true; }

std::string Component::getName() const { return m_name; }

std::optional<json> Component::getValue(const std::string& key_path) const {
    return m_ConfigManager->getValue(key_path);
}

bool Component::setValue(const std::string& key_path, const json& value) {
    return m_ConfigManager->setValue(key_path, value);
}

bool Component::hasValue(const std::string& key_path) const {
    return getValue(key_path).has_value();
}

bool Component::loadFromFile(const fs::path& path) {
    return m_ConfigManager->loadFromFile(path);
}

bool Component::saveToFile(const fs::path& file_path) const {
    return m_ConfigManager->saveToFile(file_path);
}

void Component::addAlias(const std::string& name, const std::string& alias) {
    m_CommandDispatcher->addAlias(name, alias);
}

void Component::addGroup(const std::string& name, const std::string& group) {
    m_CommandDispatcher->addGroup(name, group);
}

void Component::setTimeout(const std::string& name,
                           std::chrono::milliseconds timeout) {
    m_CommandDispatcher->setTimeout(name, timeout);
}

void Component::clearCache() { m_CommandDispatcher->clearCache(); }

void Component::removeCommand(const std::string& name) {
    m_CommandDispatcher->removeCommand(name);
}

std::vector<std::string> Component::getCommandsInGroup(
    const std::string& group) const {
    return m_CommandDispatcher->getCommandsInGroup(group);
}

std::string Component::getCommandDescription(const std::string& name) const

{
    return m_CommandDispatcher->getCommandDescription(name);
}

#if ENABLE_FASTHASH
emhash::HashSet<std::string> Component::getCommandAliases(
    const std::string& name) const
#else
std::unordered_set<std::string> Component::getCommandAliases(
    const std::string& name) const
#endif

{
    return m_CommandDispatcher->getCommandAliases(name);
}