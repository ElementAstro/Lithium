#include "database.hpp"

#include <algorithm>
#include <fstream>

#include "atom/log/loguru.hpp"

Database::Database(const std::string& filename) : filepath_(filename) {
    LOG_F(INFO, "Initializing Database with file: {}", filename);
    auto dir = filepath_.parent_path();
    if (!std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
        LOG_F(INFO, "Created directory {}", dir.string());
    }

    if (std::filesystem::exists(filepath_)) {
        LOG_F(INFO, "Loading existing database from file: {}",
              filepath_.string());
        load();
    } else {
        LOG_F(INFO, "Creating new database file: {}", filepath_.string());
        db_ = {{"version", CURRENT_VERSION},
               {"profiles", json::array()},
               {"custom_drivers", json::array()},
               {"remote_drivers", json::array()}};
        create();
    }
    update();
}

void Database::load() {
    try {
        std::ifstream file(filepath_);
        db_ = json::parse(file);
        LOG_F(INFO, "Database loaded successfully from file: {}",
              filepath_.string());
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to load database: {}", e.what());
        throw;
    }
}

void Database::save() const {
    try {
        std::ofstream file(filepath_);
        file << db_.dump(2);
        LOG_F(INFO, "Database saved successfully to file: {}",
              filepath_.string());
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to save database: {}", e.what());
        throw;
    }
}

void Database::update() {
    std::string version = db_["version"];
    if (version < CURRENT_VERSION) {
        LOG_F(INFO, "Updating database from version {} to {}", version,
              CURRENT_VERSION);
        if (version < "0.1.6") {
            LOG_F(INFO, "Updating profiles to add autoconnect field");
            for (auto& profile : db_["profiles"]) {
                if (!profile.contains("autoconnect")) {
                    profile["autoconnect"] = false;
                }
            }
        }
        db_["version"] = CURRENT_VERSION;
        save();
    }
}

void Database::create() {
    if (db_["profiles"].empty()) {
        LOG_F(INFO, "Creating default simulator profile");
        json simulator = {{"name", "Simulators"},
                          {"port", 7624},
                          {"autostart", false},
                          {"autoconnect", false},
                          {"drivers", json::array({{"Telescope Simulator"},
                                                   {"CCD Simulator"},
                                                   {"Focuser Simulator"}})}};
        db_["profiles"].push_back(simulator);
        save();
    }
}

std::optional<std::string> Database::getAutoProfile() const {
    LOG_F(INFO, "Fetching auto-start profile");
    for (const auto& profile : db_["profiles"]) {
        if (profile["autostart"].get<bool>()) {
            LOG_F(INFO, "Auto-start profile found: {}", profile["name"]);
            return profile["name"];
        }
    }
    LOG_F(INFO, "No auto-start profile found");
    return std::nullopt;
}

std::vector<json> Database::getProfiles() const {
    LOG_F(INFO, "Fetching all profiles");
    std::vector<json> profiles;
    for (const auto& profile : db_["profiles"]) {
        profiles.push_back(profile);
    }
    return profiles;
}

std::vector<json> Database::getCustomDrivers() const {
    LOG_F(INFO, "Fetching all custom drivers");
    std::vector<json> custom_drivers;
    for (const auto& driver : db_["custom_drivers"]) {
        custom_drivers.push_back(driver);
    }
    return custom_drivers;
}

std::vector<std::string> Database::getProfileDriversLabels(
    const std::string& name) const {
    LOG_F(INFO, "Fetching drivers labels for profile: {}", name);
    std::vector<std::string> labels;
    for (const auto& profile : db_["profiles"]) {
        if (profile["name"] == name) {
            for (const auto& driver : profile["drivers"]) {
                labels.push_back(driver);
            }
            break;
        }
    }
    return labels;
}

std::optional<std::string> Database::getProfileRemoteDrivers(
    const std::string& name) const {
    LOG_F(INFO, "Fetching remote drivers for profile: {}", name);
    for (const auto& remote : db_["remote_drivers"]) {
        if (remote["profile"] == name) {
            return remote["drivers"];
        }
    }
    return std::nullopt;
}

void Database::deleteProfile(const std::string& name) {
    LOG_F(INFO, "Deleting profile: {}", name);
    auto& profiles = db_["profiles"];
    profiles.erase(std::remove_if(profiles.begin(), profiles.end(),
                                  [&name](const json& profile) {
                                      return profile["name"] == name;
                                  }),
                   profiles.end());
    save();
}

int Database::addProfile(const std::string& name) {
    LOG_F(INFO, "Adding new profile: {}", name);
    json new_profile = {{"name", name},
                        {"port", 7624},
                        {"autostart", false},
                        {"autoconnect", false},
                        {"drivers", json::array()}};
    db_["profiles"].push_back(new_profile);
    save();
    return db_["profiles"].size() - 1;
}

std::optional<json> Database::getProfile(const std::string& name) const {
    LOG_F(INFO, "Fetching profile: {}", name);
    for (const auto& profile : db_["profiles"]) {
        if (profile["name"] == name) {
            return profile;
        }
    }
    return std::nullopt;
}

void Database::updateProfile(const std::string& name, int port, bool autostart,
                             bool autoconnect) {
    LOG_F(INFO, "Updating profile: {}", name);
    for (auto& profile : db_["profiles"]) {
        if (profile["name"] == name) {
            profile["port"] = port;
            profile["autostart"] = autostart;
            profile["autoconnect"] = autoconnect;
            if (autostart) {
                LOG_F(INFO,
                      "Setting autostart for profile: {} and disabling for "
                      "others",
                      name);
                for (auto& other_profile : db_["profiles"]) {
                    if (other_profile["name"] != name) {
                        other_profile["autostart"] = false;
                    }
                }
            }
            save();
            return;
        }
    }
}

void Database::saveProfileDrivers(const std::string& name,
                                  const std::vector<json>& drivers) {
    LOG_F(INFO, "Saving drivers for profile: {}", name);
    for (auto& profile : db_["profiles"]) {
        if (profile["name"] == name) {
            profile["drivers"] = drivers;
            save();
            return;
        }
    }
    json new_profile = {{"name", name},
                        {"port", 7624},
                        {"autostart", false},
                        {"autoconnect", false},
                        {"drivers", drivers}};
    db_["profiles"].push_back(new_profile);
    save();
}

void Database::saveProfileCustomDriver(const json& driver) {
    LOG_F(INFO, "Saving custom driver: {}", driver.dump());
    db_["custom_drivers"].push_back(driver);
    save();
}