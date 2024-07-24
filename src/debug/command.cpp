#include "addon/manager.hpp"

#include "atom/function/global_ptr.hpp"
#include "atom/type/json.hpp"
#include "utils/constant.hpp"

#include <iostream>

void quit() {
    std::exit(0);
}

void getComponentInfo(const std::string &name) {
    if (name.empty()) {
        std::cout << "Usage: getComponentInfo <component name>" << '\n';
        return;
    }
    auto manager = GetWeakPtr<lithium::ComponentManager>(
        constants::LITHIUM_COMPONENT_MANAGER);
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
        constants::LITHIUM_COMPONENT_MANAGER);
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
