#ifndef ATOM_COMPONENT_INL
#define ATOM_COMPONENT_INL

#include "abilities.hpp"
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

template <typename Delivery>
Component<Delivery>::Component(const std::string& name)
    : m_name(name),
      m_ConfigManager(std::make_unique<ConfigManager>()),
      m_CommandDispatcher(std::make_unique<CommandDispatcher>()),
      m_VariableManager(std::make_unique<VariableManager>()),
      m_typeInfo(user_type<Delivery>()) {
    // Empty
}

template <typename Delivery>
Component<Delivery>::~Component() {
    // Empty
}
template <typename Delivery>
bool Component<Delivery>::initialize() {
    return static_cast<Delivery*>(this)->initialize();
}

template <typename Delivery>
bool Component<Delivery>::destroy() {
    LOG_F(INFO, "Destroying component: {}", m_name);
    return static_cast<Delivery*>(this)->destroy();
}

template <typename Delivery>
std::string Component<Delivery>::getName() const {
    return m_name;
}

template <typename Delivery>
Type_Info Component<Delivery>::getTypeInfo() const {
    return m_typeInfo;
}

template <typename Delivery>
std::unordered_map<std::string, bool>
Component<Delivery>::getComponentAbilities() const {
    std::unordered_map<std::string, bool> abilities;
    bool has_getValue_ = has_getValue<Delivery>::value;
    bool has_setValue_ = has_setValue<Delivery>::value;
    return abilities;
}

template <typename Delivery>
bool Component<Delivery>::hasAbility(const std::string& ability) const {
    return getComponentAbilities().contains(ability);
}

template <typename Delivery>
std::optional<json> Component<Delivery>::getValue(
    const std::string& key_path) const {
    return m_ConfigManager->getValue(key_path);
}

template <typename Delivery>
bool Component<Delivery>::setValue(const std::string& key_path,
                                   const json& value) {
    return m_ConfigManager->setValue(key_path, value);
}

template <typename Delivery>
bool Component<Delivery>::hasValue(const std::string& key_path) const {
    return getValue(key_path).has_value();
}

template <typename Delivery>
bool Component<Delivery>::loadFromFile(const fs::path& path) {
    return m_ConfigManager->loadFromFile(path);
}

template <typename Delivery>
bool Component<Delivery>::saveToFile(const fs::path& file_path) const {
    return m_ConfigManager->saveToFile(file_path);
}

template <typename Delivery>
void Component<Delivery>::addAlias(const std::string& name,
                                   const std::string& alias) {
    m_CommandDispatcher->addAlias(name, alias);
}

template <typename Delivery>
void Component<Delivery>::addGroup(const std::string& name,
                                   const std::string& group) {
    m_CommandDispatcher->addGroup(name, group);
}

template <typename Delivery>
void Component<Delivery>::setTimeout(const std::string& name,
                                     std::chrono::milliseconds timeout) {
    m_CommandDispatcher->setTimeout(name, timeout);
}

template <typename Delivery>
void Component<Delivery>::clearCache() {
    m_CommandDispatcher->clearCache();
}

template <typename Delivery>
void Component<Delivery>::removeCommand(const std::string& name) {
    m_CommandDispatcher->removeCommand(name);
}

template <typename Delivery>
std::vector<std::string> Component<Delivery>::getCommandsInGroup(
    const std::string& group) const {
    return m_CommandDispatcher->getCommandsInGroup(group);
}

template <typename Delivery>
std::string Component<Delivery>::getCommandDescription(
    const std::string& name) const {
    return m_CommandDispatcher->getCommandDescription(name);
}

template <typename Delivery>
#if ENABLE_FASTHASH
emhash::HashSet<std::string> Component<Delivery>::getCommandAliases(
    const std::string& name) const
#else
std::unordered_set<std::string> Component<Delivery>::getCommandAliases(
    const std::string& name) const
#endif
{
    return m_CommandDispatcher->getCommandAliases(name);
}
template <typename Delivery>
std::vector<std::string> Component<Delivery>::getNeededComponents() const {
    return static_cast<Delivery*>(this)->getNeededComponents();
}

template <typename Delivery>
void Component<Delivery>::addOtherComponent(
    const std::string& name, const PointerSentinel<Component>& component) {
    m_OtherComponents[name] = std::move(component);
}

template <typename Delivery>
void Component<Delivery>::removeOtherComponent(const std::string& name) {
    m_OtherComponents.erase(name);
}

template <typename Delivery>
void Component<Delivery>::clearOtherComponents() {
    m_OtherComponents.clear();
}

#endif
