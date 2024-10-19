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

#include "atom/log/loguru.hpp"

Component::Component(std::string name) : m_name_(std::move(name)) {
    LOG_F(INFO, "Component created: {}", m_name_);
}

auto Component::getInstance() const -> std::weak_ptr<const Component> {
    LOG_SCOPE_FUNCTION(INFO);
    return shared_from_this();
}

auto Component::initialize() -> bool {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_F(INFO, "Initializing component: {}", m_name_);
    return true;
}

auto Component::destroy() -> bool {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_F(INFO, "Destroying component: {}", m_name_);
    return true;
}

auto Component::getName() const -> std::string {
    LOG_SCOPE_FUNCTION(INFO);
    return m_name_;
}

auto Component::getTypeInfo() const -> atom::meta::TypeInfo {
    LOG_SCOPE_FUNCTION(INFO);
    return m_typeInfo_;
}

void Component::setTypeInfo(atom::meta::TypeInfo typeInfo) {
    LOG_SCOPE_FUNCTION(INFO);
    m_typeInfo_ = typeInfo;
}

void Component::addAlias(const std::string& name,
                         const std::string& alias) const {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_F(INFO, "Adding alias '{}' for command '{}'", alias, name);
    m_CommandDispatcher_->addAlias(name, alias);
}

void Component::addGroup(const std::string& name,
                         const std::string& group) const {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_F(INFO, "Adding command '{}' to group '{}'", name, group);
    m_CommandDispatcher_->addGroup(name, group);
}

void Component::setTimeout(const std::string& name,
                           std::chrono::milliseconds timeout) const {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_F(INFO, "Setting timeout for command '{}': {} ms", name,
          timeout.count());
    m_CommandDispatcher_->setTimeout(name, timeout);
}

void Component::removeCommand(const std::string& name) const {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_F(INFO, "Removing command '{}'", name);
    m_CommandDispatcher_->removeCommand(name);
}

auto Component::getCommandsInGroup(const std::string& group) const
    -> std::vector<std::string> {
    LOG_SCOPE_FUNCTION(INFO);
    return m_CommandDispatcher_->getCommandsInGroup(group);
}

auto Component::getCommandDescription(const std::string& name) const
    -> std::string {
    LOG_SCOPE_FUNCTION(INFO);
    return m_CommandDispatcher_->getCommandDescription(name);
}

#if ENABLE_FASTHASH
emhash::HashSet<std::string> Component::getCommandAliases(
    const std::string& name) const
#else
auto Component::getCommandAliases(const std::string& name) const
    -> std::unordered_set<std::string>
#endif
{
    LOG_SCOPE_FUNCTION(INFO);
    return m_CommandDispatcher_->getCommandAliases(name);
}

auto Component::getCommandArgAndReturnType(const std::string& name)
    -> std::pair<std::vector<atom::meta::Arg>, std::string> {
    LOG_SCOPE_FUNCTION(INFO);
    return m_CommandDispatcher_->getCommandArgAndReturnType(name);
}

auto Component::getNeededComponents() -> std::vector<std::string> {
    LOG_SCOPE_FUNCTION(INFO);
    return {};
}

void Component::addOtherComponent(const std::string& name,
                                  const std::weak_ptr<Component>& component) {
    LOG_SCOPE_FUNCTION(INFO);
    if (m_OtherComponents_.contains(name)) {
        LOG_F(ERROR, "Component '{}' already exists", name);
        THROW_OBJ_ALREADY_EXIST(name);
    }
    LOG_F(INFO, "Adding other component '{}'", name);
    m_OtherComponents_[name] = component;
}

void Component::removeOtherComponent(const std::string& name) {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_F(INFO, "Removing other component '{}'", name);
    m_OtherComponents_.erase(name);
}

void Component::clearOtherComponents() {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_F(INFO, "Clearing all other components");
    m_OtherComponents_.clear();
}

auto Component::getOtherComponent(const std::string& name)
    -> std::weak_ptr<Component> {
    LOG_SCOPE_FUNCTION(INFO);
    if (m_OtherComponents_.contains(name)) {
        return m_OtherComponents_[name];
    }
    return {};
}

bool Component::has(const std::string& name) const {
    LOG_SCOPE_FUNCTION(INFO);
    return m_CommandDispatcher_->has(name);
}

bool Component::hasType(std::string_view name) const {
    LOG_SCOPE_FUNCTION(INFO);
    if (auto it = m_classes_.find(name); it != m_classes_.end()) {
        return true;
    }
    return false;
}

auto Component::getAllCommands() const -> std::vector<std::string> {
    LOG_SCOPE_FUNCTION(INFO);
    if (m_CommandDispatcher_ == nullptr) {
        LOG_F(ERROR, "Component command dispatch is not initialized");
        THROW_OBJ_UNINITIALIZED(
            "Component command dispatch is not initialized");
    }
    return m_CommandDispatcher_->getAllCommands();
}

auto Component::getRegisteredTypes() const -> std::vector<std::string> {
    LOG_SCOPE_FUNCTION(INFO);
    return m_TypeCaster_->getRegisteredTypes();
}

auto Component::runCommand(const std::string& name,
                           const std::vector<std::any>& args) -> std::any {
    LOG_SCOPE_FUNCTION(INFO);
    auto cmd = getAllCommands();

    if (auto it = std::ranges::find(cmd, name); it != cmd.end()) {
        LOG_F(INFO, "Running command '{}'", name);
        return m_CommandDispatcher_->dispatch(name, args);
    }
    for (const auto& [key, value] : m_OtherComponents_) {
        if (!value.expired() && value.lock()->has(name)) {
            LOG_F(INFO, "Running command '{}' in other component '{}'", name,
                  key);
            return value.lock()->dispatch(name, args);
        }
        LOG_F(ERROR, "Component '{}' has expired", key);
        m_OtherComponents_.erase(key);
    }

    LOG_F(ERROR, "Command '{}' not found", name);
    THROW_EXCEPTION("Component ", name, " not found");
}

void Component::doc(const std::string& description) {
    LOG_SCOPE_FUNCTION(INFO);
    m_doc_ = description;
}

auto Component::getDoc() const -> std::string {
    LOG_SCOPE_FUNCTION(INFO);
    return m_doc_;
}

void Component::defClassConversion(
    const std::shared_ptr<atom::meta::TypeConversionBase>& conversion) {
    LOG_SCOPE_FUNCTION(INFO);
    m_TypeConverter_->addConversion(conversion);
}

auto Component::hasVariable(const std::string& name) const -> bool {
    LOG_SCOPE_FUNCTION(INFO);
    return m_VariableManager_->has(name);
}

auto Component::getVariableDescription(const std::string& name) const
    -> std::string {
    LOG_SCOPE_FUNCTION(INFO);
    return m_VariableManager_->getDescription(name);
}

auto Component::getVariableAlias(const std::string& name) const -> std::string {
    LOG_SCOPE_FUNCTION(INFO);
    return m_VariableManager_->getAlias(name);
}

auto Component::getVariableGroup(const std::string& name) const -> std::string {
    LOG_SCOPE_FUNCTION(INFO);
    return m_VariableManager_->getGroup(name);
}