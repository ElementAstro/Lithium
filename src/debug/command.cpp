#include "command.hpp"

#include "addon/manager.hpp"

#include "atom/components/registry.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/system/user.hpp"
#include "atom/type/json.hpp"

#include "utils/constant.hpp"

#include <cstdlib>  // Include for std::exit
#include <iostream>

void quit() { std::exit(0); }  // Note: This function is not thread-safe

void loadSharedComponent(const std::string &component_name,
                         const std::string &module_name) {
    if (component_name.empty() || module_name.empty()) {
        std::cout << "Usage: loadSharedComponent <component name> <module name>"
                  << '\n';
        return;
    }
    auto manager = GetWeakPtr<lithium::ComponentManager>(
        Constants::COMPONENT_MANAGER);  // Fixed typo
    if (manager.expired()) {
        std::cout << "Component manager not found" << '\n';
        return;
    }
    if (!manager.lock()->loadComponent(
            {{"component_name", component_name},
             {"module_name", module_name},
             {"module_path", atom::system::getCurrentWorkingDirectory() +
                                 Constants::MODULE_LOADER}})) {  // Fixed typo
        std::cout << "Failed to load component" << '\n';
        return;
    }
    std::cout << "Component loaded" << '\n';
}

void unloadSharedComponent(const std::string &component_name) {
    if (component_name.empty()) {
        std::cout << "Usage: unloadSharedComponent <component name>" << '\n';
        return;
    }
    if (!GetWeakPtr<lithium::ComponentManager>(
             Constants::COMPONENT_MANAGER)  // Fixed typo
             .lock()
             ->unloadComponent({{"component_name", component_name}})) {
        std::cout << "Failed to unload component" << '\n';
        return;
    }
    std::cout << "Component unloaded" << '\n';
}

void reloadSharedComponent(const std::string &component_name) {
    if (component_name.empty()) {
        std::cout << "Usage: reloadSharedComponent <component name>" << '\n';
        return;
    }
    if (!GetWeakPtr<lithium::ComponentManager>(
             Constants::COMPONENT_MANAGER)  // Fixed typo
             .lock()
             ->reloadComponent({{"component_name", component_name}})) {
        std::cout << "Failed to reload component" << '\n';
        return;
    }
    std::cout << "Component reloaded" << '\n';
}

void reloadAllComponents() {
    if (!GetWeakPtr<lithium::ComponentManager>(
             Constants::COMPONENT_MANAGER)  // Fixed typo
             .lock()
             ->reloadAllComponents()) {
        std::cout << "Failed to reload all components" << '\n';
        return;
    }
    std::cout << "All components reloaded" << '\n';
}

void scanComponents(const std::string &path) {
    if (path.empty()) {
        std::cout << "Usage: scanComponents <path>" << '\n';
        return;
    }
    if (auto vec = GetWeakPtr<lithium::ComponentManager>(
                       Constants::COMPONENT_MANAGER)  // Fixed typo
                       .lock()
                       ->scanComponents(path);
        vec.empty()) {
        std::cout << "No components found" << '\n';
        return;
    } else {
        std::cout << "Components found:" << '\n';
        for (auto &component : vec) {
            std::cout << component << '\n';
        }
    }
}

void getComponentInfo(const std::string &name) {
    if (name.empty()) {
        std::cout << "Usage: getComponentInfo <component name>" << '\n';
        return;
    }
    auto manager = GetWeakPtr<lithium::ComponentManager>(
        Constants::COMPONENT_MANAGER);  // Fixed typo
    if (manager.expired()) {
        std::cout << "Component manager not found" << '\n';
        return;
    }
    auto info = manager.lock()->getComponentInfo(name);
    if (!info.has_value()) {
        std::cout << "Component not found" << '\n';
        return;
    }
    std::cout << "Component info:" << '\n';
    std::cout << info.value().dump(4) << '\n';
}

void getComponentList() {
    auto manager = GetWeakPtr<lithium::ComponentManager>(
        Constants::COMPONENT_MANAGER);  // Fixed typo
    if (manager.expired()) {
        std::cout << "Component manager not found" << std::endl;
        return;
    }
    auto list = manager.lock()->getComponentList();
    std::cout << "Component list:" << std::endl;
    for (auto &component : list) {
        std::cout << component << std::endl;
    }
}

void getEmbedComponentList() {
    std::cout << "Component list:" << std::endl;
    for (auto &component : Registry::instance().getAllComponentNames()) {
        std::cout << component << std::endl;
    }
}
