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

Registry::Registry() : pImpl(std::make_unique<RegistryImpl>()) {}

void Registry::loadRegistryFromFile() { pImpl->saveRegistryToFile(); }

void Registry::createKey(const std::string& keyName) {
    pImpl->saveRegistryToFile();
    pImpl->notifyEvent("KeyCreated", keyName);
}

void Registry::deleteKey(const std::string& keyName) {
    pImpl->saveRegistryToFile();
    pImpl->notifyEvent("KeyDeleted", keyName);
}

void Registry::setValue(const std::string& keyName,
                        const std::string& valueName, const std::string& data) {
    pImpl->registryData[keyName][valueName] = data;
    pImpl->saveRegistryToFile();
    pImpl->notifyEvent("ValueSet", keyName);
}

std::string Registry::getValue(const std::string& keyName,
                               const std::string& valueName) {
    if (pImpl->registryData.find(keyName) != pImpl->registryData.end() &&
        pImpl->registryData[keyName].find(valueName) !=
            pImpl->registryData[keyName].end()) {
        return pImpl->registryData[keyName][valueName];
    } else {
        return "Value not found";
    }
}

void Registry::deleteValue(const std::string& keyName,
                           const std::string& valueName) {
    if (pImpl->registryData.find(keyName) != pImpl->registryData.end()) {
        pImpl->registryData[keyName].erase(valueName);
        pImpl->saveRegistryToFile();
        pImpl->notifyEvent("ValueDeleted", keyName);
    }
}

void Registry::backupRegistryData() {
    std::time_t currentTime = std::time(nullptr);
    const std::string& backupFileName =
        "registry_backup_" + std::to_string(currentTime) + ".txt";

    std::ofstream backupFile(backupFileName);
    if (backupFile.is_open()) {
        for (const auto& key : pImpl->registryData) {
            backupFile << key.first << std::endl;
            for (const auto& value : key.second) {
                backupFile << value.first << "=" << value.second << std::endl;
            }
            backupFile << std::endl;
        }
        backupFile.close();
        DLOG_F(INFO, "Registry data backed up successfully to file: {}",
               backupFileName);
    } else {
        LOG_F(ERROR, "Error: Unable to create backup file");
    }
}

void Registry::restoreRegistryData(const std::string& backupFile) {
    std::ifstream backup(backupFile);
    if (backup.is_open()) {
        pImpl->registryData.clear();  // Clear existing data before restoring

        std::string line;
        std::string currentKey;
        while (std::getline(backup, line)) {
            if (!line.empty() && line.find('=') == std::string::npos) {
                currentKey = line;
                pImpl->registryData[currentKey] =
                    std::unordered_map<std::string, std::string>();
            } else if (line.find('=') != std::string::npos) {
                size_t splitPos = line.find('=');
                std::string valueName = line.substr(0, splitPos);
                std::string data = line.substr(splitPos + 1);
                pImpl->registryData[currentKey][valueName] = data;
            }
        }

        backup.close();
        DLOG_F(INFO, "Registry data restored successfully from backup file: {}",
               backupFile);
    } else {
        LOG_F(ERROR, "Error: Unable to open backup file for restore");
    }
}

bool Registry::keyExists(const std::string& keyName) const {
    return pImpl->registryData.find(keyName) != pImpl->registryData.end();
}

bool Registry::valueExists(const std::string& keyName,
                           const std::string& valueName) const {
    auto keyIter = pImpl->registryData.find(keyName);
    return keyIter != pImpl->registryData.end() &&
           keyIter->second.find(valueName) != keyIter->second.end();
}

std::vector<std::string> Registry::getValueNames(
    const std::string& keyName) const {
    std::vector<std::string> valueNames;
    if (pImpl->registryData.find(keyName) != pImpl->registryData.end()) {
        for (const auto& pair : pImpl->registryData.at(keyName)) {
            valueNames.push_back(pair.first);
        }
    }
    return valueNames;
}
void Registry::RegistryImpl::saveRegistryToFile() {
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
    } else {
        LOG_F(ERROR, "Error: Unable to save registry data to file");
    }
}

void Registry::RegistryImpl::notifyEvent(const std::string& eventType,
                                         const std::string& keyName) {
    DLOG_F(INFO, "Event: {} occurred for key: {}", eventType, keyName);
}

}  // namespace atom::system
