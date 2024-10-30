#ifndef LITHIUM_CLIENT_SAMBA_MANAGER_HPP
#define LITHIUM_CLIENT_SAMBA_MANAGER_HPP

#include <string>

namespace lithium {
/**
 * @class SambaManager
 * @brief Manages Samba users and shared directories.
 */
class SambaManager {
public:
    /**
     * @brief Adds a new Samba user.
     * @param username The username of the new Samba user.
     * @return True if the user was added successfully, false otherwise.
     */
    auto addUser(const std::string& username) -> bool;

    /**
     * @brief Deletes an existing Samba user.
     * @param username The username of the Samba user to delete.
     * @return True if the user was deleted successfully, false otherwise.
     */
    auto deleteUser(const std::string& username) -> bool;

    /**
     * @brief Changes the password of an existing Samba user.
     * @param username The username of the Samba user whose password is to be
     * changed.
     * @return True if the password was changed successfully, false otherwise.
     */
    auto changeUserPassword(const std::string& username) -> bool;

    /**
     * @brief Enables an existing Samba user.
     * @param username The username of the Samba user to enable.
     * @return True if the user was enabled successfully, false otherwise.
     */
    auto enableUser(const std::string& username) -> bool;

    /**
     * @brief Disables an existing Samba user.
     * @param username The username of the Samba user to disable.
     * @return True if the user was disabled successfully, false otherwise.
     */
    auto disableUser(const std::string& username) -> bool;

    /**
     * @brief Creates a new shared directory.
     * @param path The path of the new shared directory.
     * @return True if the directory was created successfully, false otherwise.
     */
    auto createSharedDirectory(const std::string& path) -> bool;

    /**
     * @brief Deletes an existing shared directory.
     * @param path The path of the shared directory to delete.
     * @return True if the directory was deleted successfully, false otherwise.
     */
    auto deleteSharedDirectory(const std::string& path) -> bool;

    /**
     * @brief Adds a new shared directory configuration to the Samba
     * configuration file.
     * @param name The name of the shared directory.
     * @param path The path of the shared directory.
     * @return True if the configuration was added successfully, false
     * otherwise.
     */
    auto addSharedDirectoryConfig(const std::string& name,
                                  const std::string& path) -> bool;

    /**
     * @brief Modifies an existing shared directory configuration in the Samba
     * configuration file.
     * @param name The name of the shared directory.
     * @param path The current path of the shared directory.
     * @param newPath The new path of the shared directory.
     * @return True if the configuration was modified successfully, false
     * otherwise.
     */
    auto modifySharedDirectoryConfig(const std::string& name,
                                     const std::string& path,
                                     const std::string& newPath) -> bool;

    /**
     * @brief Deletes an existing shared directory configuration from the Samba
     * configuration file.
     * @param name The name of the shared directory.
     * @return True if the configuration was deleted successfully, false
     * otherwise.
     */
    auto deleteSharedDirectoryConfig(const std::string& name) -> bool;

    /**
     * @brief Lists all Samba users.
     * @return True if the users were listed successfully, false otherwise.
     */
    auto listSambaUsers() -> bool;

    /**
     * @brief Lists all shared directories.
     * @return True if the directories were listed successfully, false
     * otherwise.
     */
    auto listSharedDirectories() -> bool;

private:
    /**
     * @brief Restarts the Samba service.
     * @return True if the service was restarted successfully, false otherwise.
     */
    auto restartSamba() -> bool;
};

}  // namespace lithium

#endif  // LITHIUM_CLIENT_SAMBA_MANAGER_HPP