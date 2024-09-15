#include "command.hpp"

#include "addon/manager.hpp"

#include "atom/function/global_ptr.hpp"
#include "atom/system/user.hpp"
#include "atom/type/json.hpp"

#include "components/registry.hpp"
#include "system/process.hpp"
#include "utils/constant.hpp"

#include <iostream>

void quit() { std::exit(0); }

void loadSharedCompoennt(const std::string &compoennt_name,
                         const std::string &module_name) {
    if (compoennt_name.empty() || module_name.empty()) {
        std::cout << "Usage: loadSharedCompoennt <component name> <module name>"
                  << '\n';
        return;
    }
    auto manager = GetWeakPtr<lithium::ComponentManager>(
        Constants::LITHIUM_COMPONENT_MANAGER);
    if (manager.expired()) {
        std::cout << "Component manager not found" << '\n';
        return;
    }
    if (!manager.lock()->loadComponent(
            {{"component_name", compoennt_name},
             {"module_name", module_name},
             {"module_path", atom::system::getCurrentWorkingDirectory() +
                                 Constants::MODULE_FOLDER}})) {
        std::cout << "Failed to load component" << '\n';
        return;
    }
    std::cout << "Component loaded" << '\n';
}

void unloadSharedCompoennt(const std::string &compoennt_name) {
    if (compoennt_name.empty()) {
        std::cout << "Usage: unloadSharedCompoennt <component name>" << '\n';
        return;
    }
    if (!GetWeakPtr<lithium::ComponentManager>(
             Constants::LITHIUM_COMPONENT_MANAGER)
             .lock()
             ->unloadComponent({{"component_name", compoennt_name}})) {
        std::cout << "Failed to unload component" << '\n';
        return;
    }
    std::cout << "Component unloaded" << '\n';
}

void reloadSharedCompoennt(const std::string &compoennt_name) {
    if (compoennt_name.empty()) {
        std::cout << "Usage: reloadSharedCompoennt <component name>" << '\n';
        return;
    }
    if (!GetWeakPtr<lithium::ComponentManager>(
             Constants::LITHIUM_COMPONENT_MANAGER)
             .lock()
             ->reloadComponent({{"component_name", compoennt_name}})) {
        std::cout << "Failed to reload component" << '\n';
        return;
    }
    std::cout << "Component reloaded" << '\n';
}

void reloadAllComponents() {
    if (!GetWeakPtr<lithium::ComponentManager>(
             Constants::LITHIUM_COMPONENT_MANAGER)
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
                       Constants::LITHIUM_COMPONENT_MANAGER)
                       .lock()
                       ->scanComponents(path);
        vec.empty()) {
        std::cout << "No components found";
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
        Constants::LITHIUM_COMPONENT_MANAGER);
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
        Constants::LITHIUM_COMPONENT_MANAGER);
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
