/*
 * lregister.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: A self-contained registry manager.

**************************************************/

#ifndef ATOM_SYSTEM_REGISTRY_HPP
#define ATOM_SYSTEM_REGISTRY_HPP

#include <memory>
#include <string>
#include <vector>

namespace Atom::System {
/**
 * @brief The Registry class handles registry operations.
 */
class Registry {
public:
    Registry();
    /**
     * @brief Loads registry data from a file.
     */
    void loadRegistryFromFile();

    /**
     * @brief Creates a new key in the registry.
     *
     * @param keyName The name of the key to create.
     */
    void createKey(std::string keyName);

    /**
     * @brief Deletes a key from the registry.
     *
     * @param keyName The name of the key to delete.
     */
    void deleteKey(std::string keyName);

    /**
     * @brief Sets a value for a key in the registry.
     *
     * @param keyName The name of the key.
     * @param valueName The name of the value to set.
     * @param data The data to set for the value.
     */
    void setValue(std::string keyName, std::string valueName, std::string data);

    /**
     * @brief Gets the value associated with a key and value name from the
     * registry.
     *
     * @param keyName The name of the key.
     * @param valueName The name of the value to retrieve.
     * @return The value associated with the key and value name.
     */
    std::string getValue(std::string keyName, std::string valueName);

    /**
     * @brief Deletes a value from a key in the registry.
     *
     * @param keyName The name of the key.
     * @param valueName The name of the value to delete.
     */
    void deleteValue(std::string keyName, std::string valueName);

    /**
     * @brief Backs up the registry data.
     */
    void backupRegistryData();

    /**
     * @brief Restores the registry data from a backup file.
     *
     * @param backupFile The backup file to restore data from.
     */
    void restoreRegistryData(std::string backupFile);

    /**
     * @brief Checks if a key exists in the registry.
     *
     * @param keyName The name of the key to check for existence.
     * @return true if the key exists, false otherwise.
     */
    bool keyExists(std::string keyName) const;

    /**
     * @brief Checks if a value exists for a key in the registry.
     *
     * @param keyName The name of the key.
     * @param valueName The name of the value to check for existence.
     * @return true if the value exists, false otherwise.
     */
    bool valueExists(std::string keyName, std::string valueName) const;

    /**
     * @brief Retrieves all value names for a given key from the registry.
     *
     * @param keyName The name of the key.
     * @return A vector of value names associated with the given key.
     */
    std::vector<std::string> getValueNames(std::string keyName) const;

private:
    class RegistryImpl;  // Forward declaration of the implementation class
    std::unique_ptr<RegistryImpl> pImpl;  // Pointer to the implementation class
};

}  // namespace Atom::System

#endif
