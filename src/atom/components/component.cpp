/*
 * component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-26

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
      m_CommandDispatcher(std::make_unique<CommandDispatcher>()),
      m_VariableManager(std::make_unique<VariableManager>()),
      m_typeInfo(atom::meta::user_type<Component>()),
      m_TypeCaster(atom::meta::TypeCaster::createShared()),
      m_TypeConverter(atom::meta::TypeConversions::createShared()) {
    // Empty
}

Component::~Component() {
    // Empty
}

std::weak_ptr<const Component> Component::getInstance() const {
    return shared_from_this();
}

bool Component::initialize() {
    LOG_F(INFO, "Initializing component: {}", m_name);
    return true;
}

bool Component::destroy() {
    LOG_F(INFO, "Destroying component: {}", m_name);
    return true;
}

std::string Component::getName() const { return m_name; }

atom::meta::Type_Info Component::getTypeInfo() const { return m_typeInfo; }

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

std::string Component::getCommandDescription(const std::string& name) const {
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

std::vector<std::string> Component::getNeededComponents() const { return {}; }

void Component::addOtherComponent(const std::string& name,
                                  const std::weak_ptr<Component>& component) {
    if (m_OtherComponents.contains(name)) {
        THROW_EXCEPTION(
#if __cplusplus >= 202002L
            std::format("Other component with name {} already exists",
#else
            fmt::format("Other component with name {} already exists",
#endif
                        name));
    }
    m_OtherComponents[name] = std::move(component);
}

void Component::removeOtherComponent(const std::string& name) {
    m_OtherComponents.erase(name);
}

void Component::clearOtherComponents() { m_OtherComponents.clear(); }

std::weak_ptr<Component> Component::getOtherComponent(const std::string& name) {
    if (m_OtherComponents.contains(name)) {
        return m_OtherComponents[name];
    }
    return {};
}

bool Component::has(const std::string& name) const {
    return m_CommandDispatcher->has(name);
}

std::vector<std::string> Component::getAllCommands() const {
    return m_CommandDispatcher->getAllCommands();
}

std::any Component::runCommand(const std::string& name,
                               const std::vector<std::any>& args) {
    auto _cmd = getAllCommands();
    auto it = std::find(_cmd.begin(), _cmd.end(), name);

    if (it != _cmd.end()) {
        return m_CommandDispatcher->dispatch(name, args);
    } else {
        for (auto& [key, value] : m_OtherComponents) {
            if (!value.expired()) {
                if (value.lock()->has(name)) {
                    return value.lock()->dispatch(name, args);
                }
            } else {
                LOG_F(ERROR, "Component {} has expired", key);
                m_OtherComponents.erase(key);
            }
        }
    }
    THROW_EXCEPTION(
#if __cplusplus >= 202002L
        std::format("Command with name {} not found",
#else
        fmt::format("Command with name {} not found",
#endif
                    name));
}

void Component::def(const atom::meta::Type_Info& ti, const std::string& group,
                    const std::string& description) {
    m_classes.push_back(ti);
}
