/*
 * _script.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Carbon Binding of ConfigManager

**************************************************/

#ifndef LITHIUM_CONFIG_SCRIPT_HPP
#define LITHIUM_CONFIG_SCRIPT_HPP

#include "carbon/carbon.hpp"

#include "configor.hpp"

#include "atom/log/loguru.hpp"
#include "atom/function/global_ptr.hpp"

namespace lithium::_Script::Config {
/**
 * Adds the String Methods to the given Carbon m.
 */
Carbon::ModulePtr bootstrap(
    Carbon::ModulePtr m = std::make_shared<Carbon::Module>()) {
    try {
        auto m_configManager = GetWeakPtr<ConfigManager>("lithium.config");
        if (m_configManager.expired()) {
            LOG_F(ERROR, "config manager pointer is expired!");
            return nullptr;
        }

        auto get_config_func = [](std::weak_ptr<ConfigManager> manager,
                                  const std::string &sv) -> json {
            if (!sv.empty() && !manager.expired()) {
                LOG_F(ERROR, "no key path found");
                return {};
            }
            if (auto value = manager.lock()->getValue(sv); value.has_value()) {
                return value.value();
            }
            return {};
        };

        m->add(Carbon::fun([m_configManager,
                            get_config_func](const std::string &key) -> double {
                   if (!key.empty()) {
                       try {
                           return get_config_func(m_configManager, key)
                               .get<double>();
                       } catch (const json::exception &e) {
                       }
                   }
                   return -1;
               }),
               "get_number_config");

        m->add(Carbon::fun([m_configManager, get_config_func](
                               const std::string &key) -> std::string {
                   if (!key.empty()) {
                       try {
                           return get_config_func(m_configManager, key)
                               .get<std::string>();
                       } catch (const json::exception &e) {
                       }
                   }
                   return "";
               }),
               "get_string_config");

        m->add(Carbon::fun([m_configManager,
                            get_config_func](const std::string &key) -> bool {
                   if (!key.empty()) {
                       try {
                           return get_config_func(m_configManager, key)
                               .get<bool>();
                       } catch (const json::exception &e) {
                       }
                   }
                   return false;
               }),
               "get_boolean_config");

        m->add(Carbon::fun([m_configManager](const std::string &key,
                                             double number) {
                   if (!key.empty() && !m_configManager.expired()) [[likely]] {
                       if (m_configManager.lock()->setValue(key, number)) {
                           return true;
                       }
                   }
                   return false;
               }),
               "set_number_config");

        m->add(Carbon::fun([m_configManager](const std::string &key,
                                             const std::string &text) {
                   if (!key.empty() && !m_configManager.expired()) [[likely]] {
                       if (m_configManager.lock()->setValue(key, text)) {
                           return true;
                       }
                   }
                   return false;
               }),
               "set_string_config");

        m->add(
            Carbon::fun([m_configManager](const std::string &key, bool value) {
                if (!key.empty() && !m_configManager.expired()) [[likely]] {
                    if (m_configManager.lock()->setValue(key, value)) {
                        return true;
                    }
                }
                return false;
            }),
            "set_bool_config");

        m->add(Carbon::fun([m_configManager](const std::string &key) {
                   if (!key.empty() && !m_configManager.expired()) [[likely]] {
                       if (m_configManager.lock()->hasValue(key)) {
                           return true;
                       }
                   }
                   return false;
               }),
               "has_config");

        m->add(Carbon::fun([m_configManager](const std::string &key) {
                   if (!key.empty() && !m_configManager.expired()) [[likely]] {
                       if (m_configManager.lock()->deleteValue(key)) {
                           return true;
                       }
                   }
                   return false;
               }),
               "delete_config");

        m->add(Carbon::fun([m_configManager](const std::string &file_path) {
                   if (!file_path.empty() && !m_configManager.expired())
                       [[likely]] {
                       if (m_configManager.lock()->loadFromFile(file_path)) {
                           return true;
                       }
                   }
                   return false;
               }),
               "load_config");

        m->add(Carbon::fun([m_configManager](const std::string &folder_path) {
                   if (!folder_path.empty() && !m_configManager.expired())
                       [[likely]] {
                       if (m_configManager.lock()->loadFromDir(folder_path)) {
                           return true;
                       }
                   }
                   return false;
               }),
               "load_configs");

        m->add(Carbon::fun([m_configManager](const std::string &file_path) {
                   if (!file_path.empty() && !m_configManager.expired())
                       [[likely]] {
                       if (m_configManager.lock()->saveToFile(file_path)) {
                           return true;
                       }
                   }
                   return false;
               }),
               "save_config");

        m->add(Carbon::fun([m_configManager]() {
                   if (!m_configManager.expired()) [[likely]] {
                       m_configManager.lock()->tidyConfig();
                   }
               }),
               "tidy_config");

        return m;
    } catch (const std::bad_any_cast &e) {
        LOG_F(ERROR, "Failed to load config manager");
        return nullptr;
    }
}
}  // namespace lithium::_Script::Config

#endif
