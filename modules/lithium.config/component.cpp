/*
 * _com.cpp
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

#include "atom/function/overload.hpp"
#include "atom/log/loguru.hpp"
#include "atom/tests/test.hpp"
#include "atom/type/json.hpp"

static auto mConfigManager = lithium::ConfigManager::createShared();

ATOM_MODULE(lithium_config, [](Component& com) {
    DLOG_F(INFO, "Loading module {}", com.getName());

    com.def("getConfig", &lithium::ConfigManager::getValue, mConfigManager);
    com.def("setConfig",
            atom::meta::overload_cast<const std::string&, const json&>(
                &lithium::ConfigManager::setValue),
            mConfigManager);
    com.def("hasConfig", &lithium::ConfigManager::hasValue, mConfigManager);
    com.def("deleteConfig", &lithium::ConfigManager::deleteValue,
            mConfigManager);
    com.def("loadConfig", &lithium::ConfigManager::loadFromFile,
            mConfigManager);
    com.def("loadConfigs", &lithium::ConfigManager::loadFromDir,
            mConfigManager);
    com.def("saveConfig", &lithium::ConfigManager::saveToFile, mConfigManager);
    com.def("tidyConfig", &lithium::ConfigManager::tidyConfig, mConfigManager);
    com.def("clearConfig", &lithium::ConfigManager::clearConfig,
            mConfigManager);
    com.def("asyncLoadConfig", &lithium::ConfigManager::asyncLoadFromFile,
            mConfigManager);
    com.def("asyncSaveConfig", &lithium::ConfigManager::asyncSaveToFile,
            mConfigManager);

    com.addVariable("config.instance", mConfigManager,
                    "ConfigManager Instance");

    LOG_F(INFO, "Loaded module {}", com.getName());
});

ATOM_MODULE_TEST(
    lithium_config, [](const std::shared_ptr<Component>& component) {
        auto createShared = "createShared"_test([] {
            auto configManager = lithium::ConfigManager::createShared();
            expect(configManager != nullptr);
        });

        auto createUnique = "createUnique"_test([] {
            auto configManager = lithium::ConfigManager::createUnique();
            expect(configManager != nullptr);
        });

        // Test setValue and getValue
        auto setAndGetValue = "setValue and getValue"_test([] {
            auto configManager = lithium::ConfigManager::createShared();
            json value = 42;
            bool setResult = configManager->setValue("testKey", value);
            expect(setResult);

            auto getValueResult = configManager->getValue("testKey");
            expect(getValueResult.has_value());
            expect_eq(getValueResult.value().get<int>(), 42);
        });

        // Test deleteValue
        auto deleteValue = "deleteValue"_test([] {
            auto configManager = lithium::ConfigManager::createShared();
            json value = 42;
            configManager->setValue("testKey", value);

            bool deleteResult = configManager->deleteValue("testKey");
            expect(deleteResult);

            auto getValueResult = configManager->getValue("testKey");
            expect(!getValueResult.has_value());
        });

        // Test hasValue
        auto hasValue = "hasValue"_test([] {
            auto configManager = lithium::ConfigManager::createShared();
            json value = 42;
            configManager->setValue("testKey", value);

            bool hasValueResult = configManager->hasValue("testKey");
            expect(hasValueResult);

            configManager->deleteValue("testKey");
            hasValueResult = configManager->hasValue("testKey");
            expect(!hasValueResult);
        });

        // Test loadFromFile and saveToFile
        auto loadAndSaveFile = "loadFromFile and saveToFile"_test([] {
            auto configManager = lithium::ConfigManager::createShared();
            json value = 42;
            configManager->setValue("testKey", value);

            fs::path filePath = "testConfig.json";
            bool saveResult = configManager->saveToFile(filePath);
            expect(saveResult);

            auto newConfigManager = lithium::ConfigManager::createShared();
            bool loadResult = newConfigManager->loadFromFile(filePath);
            expect(loadResult);

            auto getValueResult = newConfigManager->getValue("testKey");
            expect(getValueResult.has_value());
            expect_eq(getValueResult.value().get<int>(), 42);

            // Clean up
            fs::remove(filePath);
        });

        // Test clearConfig
        auto clearConfig = "clearConfig"_test([] {
            auto configManager = lithium::ConfigManager::createShared();
            json value = 42;
            configManager->setValue("testKey", value);

            configManager->clearConfig();
            auto getValueResult = configManager->getValue("testKey");
            expect(!getValueResult.has_value());
        });

        auto mergeConfig = "mergeConfig"_test([] {
            auto configManager = lithium::ConfigManager::createShared();
            json value1 = 42;
            configManager->setValue("testKey1", 42);

            json value2 = 84;
            json mergeData = {"testKey2", (84)};
            configManager->mergeConfig(mergeData);

            auto getValueResult1 = configManager->getValue("testKey1");
            expect(getValueResult1.has_value());
            expect_eq(getValueResult1.value().get<int>(), 42);

            auto getValueResult2 = configManager->getValue("testKey2");
            expect(getValueResult2.has_value());
            expect_eq(getValueResult2.value().get<int>(), 84);
        });

        atom::test::registerSuite(
            "ConfigManager Tests",
            {createShared, createUnique, setAndGetValue, deleteValue, hasValue,
             loadAndSaveFile, clearConfig, mergeConfig});

        atom::test::runTests();
    });
