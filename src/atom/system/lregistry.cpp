/*
 * lregister.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: A self-contained registry manager.

**************************************************/

#include "lregistry.hpp"

#include <ctime>
#include <fstream>
#include <unordered_map>

#include "atom/log/loguru.hpp"

namespace atom::system {

class Registry::RegistryImpl {
public:
    std::unordered_map<std::string,
                       std::unordered_map<std::string, std::string>>
        registryData;

    void saveRegistryToFile();
    void notifyEvent(const std::string& eventType, const std::string& keyName);
};

Registry::Registry() : pImpl_(std::make_unique<RegistryImpl>()) {
    LOG_F(INFO, "Registry constructor called");
}

void Registry::loadRegistryFromFile() {
    LOG_F(INFO, "Registry::loadRegistryFromFile called");
    pImpl_->saveRegistryToFile();
    LOG_F(INFO, "Registry::loadRegistryFromFile completed");
}

void Registry::createKey(const std::string& keyName) {
    LOG_F(INFO, "Registry::createKey called with keyName: {}", keyName);
    pImpl_->saveRegistryToFile();
    pImpl_->notifyEvent("KeyCreated", keyName);
    LOG_F(INFO, "Registry::createKey completed for keyName: {}", keyName);
}

void Registry::deleteKey(const std::string& keyName) {
    LOG_F(INFO, "Registry::deleteKey called with keyName: {}", keyName);
    pImpl_->saveRegistryToFile();
    pImpl_->notifyEvent("KeyDeleted", keyName);
    LOG_F(INFO, "Registry::deleteKey completed for keyName: {}", keyName);
}

void Registry::setValue(const std::string& keyName,
                        const std::string& valueName, const std::string& data) {
    LOG_F(INFO,
          "Registry::setValue called with keyName: {}, valueName: {}, data: {}",
          keyName, valueName, data);
    pImpl_->registryData[keyName][valueName] = data;
    pImpl_->saveRegistryToFile();
    pImpl_->notifyEvent("ValueSet", keyName);
    LOG_F(INFO, "Registry::setValue completed for keyName: {}, valueName: {}",
          keyName, valueName);
}

auto Registry::getValue(const std::string& keyName,
                        const std::string& valueName) -> std::string {
    LOG_F(INFO, "Registry::getValue called with keyName: {}, valueName: {}",
          keyName, valueName);
    if (pImpl_->registryData.find(keyName) != pImpl_->registryData.end() &&
        pImpl_->registryData[keyName].find(valueName) !=
            pImpl_->registryData[keyName].end()) {
        std::string value = pImpl_->registryData[keyName][valueName];
        LOG_F(
            INFO,
            "Registry::getValue found value: {} for keyName: {}, valueName: {}",
            value, keyName, valueName);
        return value;
    }
    LOG_F(
        WARNING,
        "Registry::getValue did not find value for keyName: {}, valueName: {}",
        keyName, valueName);
    return "Value not found";
}

void Registry::deleteValue(const std::string& keyName,
                           const std::string& valueName) {
    LOG_F(INFO, "Registry::deleteValue called with keyName: {}, valueName: {}",
          keyName, valueName);
    if (pImpl_->registryData.find(keyName) != pImpl_->registryData.end()) {
        pImpl_->registryData[keyName].erase(valueName);
        pImpl_->saveRegistryToFile();
        pImpl_->notifyEvent("ValueDeleted", keyName);
        LOG_F(INFO,
              "Registry::deleteValue completed for keyName: {}, valueName: {}",
              keyName, valueName);
    } else {
        LOG_F(WARNING, "Registry::deleteValue did not find keyName: {}",
              keyName);
    }
}

void Registry::backupRegistryData() {
    LOG_F(INFO, "Registry::backupRegistryData called");
    std::time_t currentTime = std::time(nullptr);
    std::string backupFileName =
        "registry_backup_" + std::to_string(currentTime) + ".txt";

    std::ofstream backupFile(backupFileName);
    if (backupFile.is_open()) {
        for (const auto& key : pImpl_->registryData) {
            backupFile << key.first << std::endl;
            for (const auto& value : key.second) {
                backupFile << value.first << "=" << value.second << std::endl;
            }
            backupFile << std::endl;
        }
        backupFile.close();
        LOG_F(INFO, "Registry data backed up successfully to file: {}",
              backupFileName);
    } else {
        LOG_F(ERROR, "Error: Unable to create backup file");
    }
}

void Registry::restoreRegistryData(const std::string& backupFile) {
    LOG_F(INFO, "Registry::restoreRegistryData called with backupFile: {}",
          backupFile);
    std::ifstream backup(backupFile);
    if (backup.is_open()) {
        pImpl_->registryData.clear();  // Clear existing data before restoring

        std::string line;
        std::string currentKey;
        while (std::getline(backup, line)) {
            if (!line.empty() && line.contains('=')) {
                currentKey = line;
                pImpl_->registryData[currentKey] =
                    std::unordered_map<std::string, std::string>();
            } else if (line.contains('=')) {
                size_t splitPos = line.find('=');
                std::string valueName = line.substr(0, splitPos);
                std::string data = line.substr(splitPos + 1);
                pImpl_->registryData[currentKey][valueName] = data;
            }
        }

        backup.close();
        LOG_F(INFO, "Registry data restored successfully from backup file: {}",
              backupFile);
    } else {
        LOG_F(ERROR, "Error: Unable to open backup file for restore");
    }
}

auto Registry::keyExists(const std::string& keyName) const -> bool {
    LOG_F(INFO, "Registry::keyExists called with keyName: {}", keyName);
    bool exists =
        pImpl_->registryData.find(keyName) != pImpl_->registryData.end();
    LOG_F(INFO, "Registry::keyExists returning: {}", exists);
    return exists;
}

auto Registry::valueExists(const std::string& keyName,
                           const std::string& valueName) const -> bool {
    LOG_F(INFO, "Registry::valueExists called with keyName: {}, valueName: {}",
          keyName, valueName);
    auto keyIter = pImpl_->registryData.find(keyName);
    bool exists = keyIter != pImpl_->registryData.end() &&
                  keyIter->second.find(valueName) != keyIter->second.end();
    LOG_F(INFO, "Registry::valueExists returning: {}", exists);
    return exists;
}

auto Registry::getValueNames(const std::string& keyName) const
    -> std::vector<std::string> {
    LOG_F(INFO, "Registry::getValueNames called with keyName: {}", keyName);
    std::vector<std::string> valueNames;
    if (pImpl_->registryData.find(keyName) != pImpl_->registryData.end()) {
        for (const auto& pair : pImpl_->registryData.at(keyName)) {
            valueNames.push_back(pair.first);
        }
    }
    LOG_F(INFO, "Registry::getValueNames returning {} value names",
          valueNames.size());
    return valueNames;
}

void Registry::RegistryImpl::saveRegistryToFile() {
    LOG_F(INFO, "RegistryImpl::saveRegistryToFile called");
    std::ofstream file("registry_data.txt");
    if (file.is_open()) {
        for (auto const& key : registryData) {
            file << key.first << std::endl;
            for (auto const& value : key.second) {
                file << value.first << "=" << value.second << std::endl;
            }
            file << std::endl;
        }
        file.close();
        LOG_F(INFO, "Registry data saved to file successfully");
    } else {
        LOG_F(ERROR, "Error: Unable to save registry data to file");
    }
}

void Registry::RegistryImpl::notifyEvent(const std::string& eventType,
                                         const std::string& keyName) {
    LOG_F(INFO, "Event: {} occurred for key: {}", eventType, keyName);
}

}  // namespace atom::system
