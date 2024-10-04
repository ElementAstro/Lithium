/*
 * _component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Config Component for Atom Addon

**************************************************/

#include "atom/components/component.hpp"
#include "atom/components/registry.hpp"

#include "config/configor.hpp"

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

static auto mConfigManager = lithium::ConfigManager::createShared();

ATOM_MODULE(lithium_config, [](Component &component) {
    DLOG_F(INFO, "Loading module {}", component.getName());

    component.def("getConfig", &lithium::ConfigManager::getValue,
                  mConfigManager);
    component.def("setConfig", &lithium::ConfigManager::setValue,
                  mConfigManager);
    component.def("hasConfig", &lithium::ConfigManager::hasValue,
                  mConfigManager);
    component.def("deleteConfig", &lithium::ConfigManager::deleteValue,
                  mConfigManager);
    component.def("loadConfig", &lithium::ConfigManager::loadFromFile,
                  mConfigManager);
    component.def("loadConfigs", &lithium::ConfigManager::loadFromDir,
                  mConfigManager);
    component.def("saveConfig", &lithium::ConfigManager::saveToFile,
                  mConfigManager);
    component.def("tidyConfig", &lithium::ConfigManager::tidyConfig,
                  mConfigManager);
    component.def("clearConfig", &lithium::ConfigManager::clearConfig,
                  mConfigManager);
    component.def("asyncLoadConfig", &lithium::ConfigManager::asyncLoadFromFile,
                  mConfigManager);
    component.def("asyncSaveConfig", &lithium::ConfigManager::asyncSaveToFile,
                  mConfigManager);

    component.addVariable("config.instance", mConfigManager,
                          "ConfigManager Instance");

    LOG_F(INFO, "Loaded module {}", component.getName());
});

ATOM_MODULE_TEST(lithium_config, [](const std::shared_ptr<Component>& component) {
    
});