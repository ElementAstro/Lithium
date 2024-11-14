// database.hpp
#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>
#include "atom/type/json.hpp"

using json = nlohmann::json;

/**
 * @class Database
 * @brief A class to manage and store INDI profiles and drivers using JSON.
 *
 * This class provides functionality to manage profiles and drivers, including
 * adding, deleting, updating, and retrieving profiles and drivers. The data is
 * stored in a JSON file.
 */
class Database {
public:
    /**
     * @brief Constructs a Database object with the given filename.
     * @param filename The path to the JSON file used for storing the database.
     */
    explicit Database(const std::string& filename);

    /**
     * @brief Gets the auto-start profile.
     * @return The name of the auto-start profile, or std::nullopt if none is
     * set.
     */
    std::optional<std::string> getAutoProfile() const;

    /**
     * @brief Gets all profiles from the database.
     * @return A vector of JSON objects representing the profiles.
     */
    std::vector<json> getProfiles() const;

    /**
     * @brief Gets all custom drivers from the database.
     * @return A vector of JSON objects representing the custom drivers.
     */
    std::vector<json> getCustomDrivers() const;

    /**
     * @brief Gets all driver labels for a specific profile.
     * @param name The name of the profile.
     * @return A vector of strings representing the driver labels.
     */
    std::vector<std::string> getProfileDriversLabels(
        const std::string& name) const;

    /**
     * @brief Gets the remote drivers list for a specific profile.
     * @param name The name of the profile.
     * @return A string representing the remote drivers, or std::nullopt if not
     * found.
     */
    std::optional<std::string> getProfileRemoteDrivers(
        const std::string& name) const;

    /**
     * @brief Deletes a profile from the database.
     * @param name The name of the profile to delete.
     */
    void deleteProfile(const std::string& name);

    /**
     * @brief Adds a new profile to the database.
     * @param name The name of the new profile.
     * @return The ID of the newly added profile.
     */
    int addProfile(const std::string& name);

    /**
     * @brief Gets the information of a specific profile.
     * @param name The name of the profile.
     * @return A JSON object representing the profile, or std::nullopt if not
     * found.
     */
    std::optional<json> getProfile(const std::string& name) const;

    /**
     * @brief Updates the information of a specific profile.
     * @param name The name of the profile.
     * @param port The port number of the profile.
     * @param autostart Whether the profile should auto-start.
     * @param autoconnect Whether the profile should auto-connect.
     */
    void updateProfile(const std::string& name, int port,
                       bool autostart = false, bool autoconnect = false);

    /**
     * @brief Saves the drivers for a specific profile.
     * @param name The name of the profile.
     * @param drivers A vector of JSON objects representing the drivers.
     */
    void saveProfileDrivers(const std::string& name,
                            const std::vector<json>& drivers);

    /**
     * @brief Saves a custom driver to the database.
     * @param driver A JSON object representing the custom driver.
     */
    void saveProfileCustomDriver(const json& driver);

private:
    /**
     * @brief Updates the database schema if necessary.
     */
    void update();

    /**
     * @brief Creates the initial database schema.
     */
    void create();

    /**
     * @brief Saves the database to the JSON file.
     */
    void save() const;

    /**
     * @brief Loads the database from the JSON file.
     */
    void load();

    std::filesystem::path filepath_;  ///< The path to the JSON file used for
                                      ///< storing the database.
    json db_;  ///< The JSON object representing the database.
    static constexpr const char* CURRENT_VERSION =
        "0.1.6";  ///< The current version of the database schema.
};