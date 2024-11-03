#include "profile.hpp"

#include <fstream>
#include <iostream>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

struct ServerConfigData {
    static inline const fs::path PHD2_HIDDEN_CONFIG_FILE =
        "./phd2_hidden_config.json";
    static inline const fs::path DEFAULT_PHD2_CONFIG_FILE =
        "./default_phd2_config.json";
    static inline const fs::path PROFILE_SAVE_PATH = "./server/data/phd2";
};

class PHD2ProfileSettingHandler::Impl {
public:
    std::optional<InterfacePHD2Profile> loadedConfigStatus;
    const fs::path PHD2_PROFILE_SAVE_PATH = ServerConfigData::PROFILE_SAVE_PATH;

    static void replaceDoubleMarker(const fs::path& file_path) {
        std::ifstream inputFile(file_path);
        if (!inputFile.is_open()) {
            LOG_F(ERROR, "Failed to open file for reading: {}",
                  file_path);
            THROW_FAIL_TO_OPEN_FILE("Failed to open file for reading.");
        }

        std::string content((std::istreambuf_iterator<char>(inputFile)),
                            std::istreambuf_iterator<char>());
        inputFile.close();

        size_t pos = content.find("\"\"#");
        while (pos != std::string::npos) {
            content.replace(pos, 3, "#");
            pos = content.find("\"\"#", pos + 1);
        }

        std::ofstream outputFile(file_path);
        if (!outputFile.is_open()) {
            LOG_F(ERROR, "Failed to open file for writing: {}",
                  file_path);
            THROW_FAIL_TO_OPEN_FILE("Failed to open file for writing.");
        }

        outputFile << content;
        outputFile.close();
    }

    [[nodiscard]] static auto loadJsonFile(const fs::path& file_path) -> json {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            LOG_F(ERROR, "Failed to open JSON file: {}", file_path);
            THROW_FAIL_TO_OPEN_FILE("Failed to open JSON file.");
        }

        json config;
        try {
            file >> config;
        } catch (const json::parse_error& e) {
            LOG_F(ERROR, "JSON parsing error in file {}: {}", file_path,
                  e.what());
            throw;
        }
        file.close();
        return config;
    }

    static void saveJsonFile(const fs::path& file_path, const json& config) {
        std::ofstream file(file_path);
        if (!file.is_open()) {
            LOG_F(ERROR, "Failed to open JSON file for writing: {}",
                  file_path);
            THROW_FAIL_TO_OPEN_FILE("Failed to open JSON file for writing.");
        }

        try {
            file << config.dump(4);
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Error writing JSON to file {}: {}", file_path,
                  e.what());
            throw;
        }
        file.close();
        replaceDoubleMarker(file_path);
    }
};

PHD2ProfileSettingHandler::PHD2ProfileSettingHandler()
    : pImpl(std::make_unique<Impl>()) {
    LOG_F(INFO, "PHD2ProfileSettingHandler initialized.");
}

PHD2ProfileSettingHandler::~PHD2ProfileSettingHandler() = default;

PHD2ProfileSettingHandler::PHD2ProfileSettingHandler(
    PHD2ProfileSettingHandler&&) noexcept = default;

auto PHD2ProfileSettingHandler::operator=(PHD2ProfileSettingHandler&&) noexcept
    -> PHD2ProfileSettingHandler& = default;

auto PHD2ProfileSettingHandler::loadProfileFile()
    -> std::optional<InterfacePHD2Profile> {
    LOG_F(INFO, "Loading profile file.");
    if (!fs::exists(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE)) {
        LOG_F(WARNING,
              "Hidden config file does not exist. Copying default config.");
        fs::copy_file(ServerConfigData::DEFAULT_PHD2_CONFIG_FILE,
                      ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                      fs::copy_options::overwrite_existing);
    }

    try {
        json phd2Config =
            pImpl->loadJsonFile(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE);
        pImpl->loadedConfigStatus = InterfacePHD2Profile{
            .name =
                phd2Config.at("profile").at("1").at("name").get<std::string>(),
            .camera = phd2Config.at("profile")
                          .at("1")
                          .at("indi")
                          .at("INDIcam")
                          .get<std::string>(),
            .cameraCCD = phd2Config.at("profile")
                             .at("1")
                             .at("indi")
                             .at("INDIcam_ccd")
                             .get<std::string>(),
            .pixelSize = phd2Config.at("profile")
                             .at("1")
                             .at("camera")
                             .at("pixelsize")
                             .get<double>(),
            .telescope = phd2Config.at("profile")
                             .at("1")
                             .at("indi")
                             .at("INDImount")
                             .get<std::string>(),
            .focalLength = phd2Config.at("profile")
                               .at("1")
                               .at("frame")
                               .at("focalLength")
                               .get<double>(),
            .massChangeThreshold = phd2Config.at("profile")
                                       .at("1")
                                       .at("guider")
                                       .at("onestar")
                                       .at("MassChangeThreshold")
                                       .get<double>(),
            .massChangeFlag = phd2Config.at("profile")
                                  .at("1")
                                  .at("guider")
                                  .at("onestar")
                                  .at("MassChangeThresholdEnabled")
                                  .get<bool>(),
            .calibrationDistance = phd2Config.at("profile")
                                       .at("1")
                                       .at("scope")
                                       .at("CalibrationDistance")
                                       .get<double>(),
            .calibrationDuration = phd2Config.at("profile")
                                       .at("1")
                                       .at("scope")
                                       .at("CalibrationDuration")
                                       .get<double>()};
        LOG_F(INFO, "Profile file loaded successfully.");
    } catch (const json::exception& e) {
        LOG_F(ERROR, "JSON parsing error: {}", e.what());
        fs::remove(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE);
        fs::copy_file(ServerConfigData::DEFAULT_PHD2_CONFIG_FILE,
                      ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                      fs::copy_options::overwrite_existing);
        return loadProfileFile();  // Recursive call with default config
    }

    return pImpl->loadedConfigStatus;
}

auto PHD2ProfileSettingHandler::loadProfile(const std::string& profileName)
    -> bool {
    LOG_F(INFO, "Loading profile: {}", profileName);
    fs::path profileFile =
        pImpl->PHD2_PROFILE_SAVE_PATH / (profileName + ".json");

    if (fs::exists(profileFile)) {
        fs::copy_file(profileFile, ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                      fs::copy_options::overwrite_existing);
        try {
            loadProfileFile();
            LOG_F(INFO, "Profile {} loaded successfully.", profileName);
            return true;
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Failed to load profile {}: {}", profileName,
                  e.what());
            return false;
        }
    } else {
        LOG_F(WARNING, "Profile {} does not exist. Loading default profile.",
              profileName);
        fs::copy_file(ServerConfigData::DEFAULT_PHD2_CONFIG_FILE,
                      ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                      fs::copy_options::overwrite_existing);
        loadProfileFile();
        return false;
    }
}

auto PHD2ProfileSettingHandler::newProfileSetting(
    const std::string& newProfileName) -> bool {
    LOG_F(INFO, "Creating new profile: {}", newProfileName);
    fs::path newProfileFile =
        pImpl->PHD2_PROFILE_SAVE_PATH / (newProfileName + ".json");

    if (fs::exists(newProfileFile)) {
        LOG_F(WARNING, "Profile {} already exists. Restoring existing profile.",
              newProfileName);
        return restoreProfile(newProfileName);
    }
    fs::copy_file(ServerConfigData::DEFAULT_PHD2_CONFIG_FILE,
                  ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                  fs::copy_options::overwrite_existing);
    loadProfileFile();
    saveProfile(newProfileName);
    LOG_F(INFO, "New profile {} created successfully.", newProfileName);
    return true;
}

auto PHD2ProfileSettingHandler::updateProfile(
    const InterfacePHD2Profile& phd2ProfileSetting) -> bool {
    LOG_F(INFO, "Updating profile: {}", phd2ProfileSetting.name);
    json phd2Config =
        pImpl->loadJsonFile(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE);

    try {
        phd2Config["profile"]["1"]["name"] = phd2ProfileSetting.name;
        phd2Config["profile"]["1"]["indi"]["INDIcam"] =
            phd2ProfileSetting.camera;
        phd2Config["profile"]["1"]["indi"]["INDIcam_ccd"] =
            phd2ProfileSetting.cameraCCD;
        phd2Config["profile"]["1"]["camera"]["pixelsize"] =
            phd2ProfileSetting.pixelSize;
        phd2Config["profile"]["1"]["indi"]["INDImount"] =
            phd2ProfileSetting.telescope;
        phd2Config["profile"]["1"]["frame"]["focalLength"] =
            phd2ProfileSetting.focalLength;
        phd2Config["profile"]["1"]["guider"]["onestar"]["MassChangeThreshold"] =
            phd2ProfileSetting.massChangeThreshold;
        phd2Config["profile"]["1"]["guider"]["onestar"]
                  ["MassChangeThresholdEnabled"] =
                      phd2ProfileSetting.massChangeFlag;
        phd2Config["profile"]["1"]["scope"]["CalibrationDistance"] =
            phd2ProfileSetting.calibrationDistance;
        phd2Config["profile"]["1"]["scope"]["CalibrationDuration"] =
            phd2ProfileSetting.calibrationDuration;
    } catch (const json::exception& e) {
        LOG_F(ERROR, "Error updating profile: {}", e.what());
        throw std::runtime_error("Error updating profile: " +
                                 std::string(e.what()));
    }

    pImpl->saveJsonFile(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE, phd2Config);
    LOG_F(INFO, "Profile {} updated successfully.",
          phd2ProfileSetting.name);
    return true;
}

auto PHD2ProfileSettingHandler::deleteProfile(
    const std::string& toDeleteProfile) -> bool {
    LOG_F(INFO, "Deleting profile: {}", toDeleteProfile);
    fs::path toDeleteProfileFile =
        pImpl->PHD2_PROFILE_SAVE_PATH / (toDeleteProfile + ".json");
    if (fs::exists(toDeleteProfileFile)) {
        try {
            fs::remove(toDeleteProfileFile);
            LOG_F(INFO, "Profile {} deleted successfully.",
                  toDeleteProfile);
            return true;
        } catch (const fs::filesystem_error& e) {
            LOG_F(ERROR, "Failed to delete profile {}: {}",
                  toDeleteProfile, e.what());
            return false;
        }
    }
    LOG_F(WARNING, "Profile {} does not exist.", toDeleteProfile);
    return false;
}

void PHD2ProfileSettingHandler::saveProfile(const std::string& profileName) {
    LOG_F(INFO, "Saving current profile as: {}", profileName);
    fs::path profileFile =
        pImpl->PHD2_PROFILE_SAVE_PATH / (profileName + ".json");
    if (fs::exists(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE)) {
        try {
            if (fs::exists(profileFile)) {
                fs::remove(profileFile);
                LOG_F(INFO, "Existing profile file {} removed.",
                      profileFile);
            }
            fs::copy_file(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                          profileFile, fs::copy_options::overwrite_existing);
            LOG_F(INFO, "Profile saved successfully as {}.",
                  profileName);
        } catch (const fs::filesystem_error& e) {
            LOG_F(ERROR, "Failed to save profile {}: {}", profileName,
                  e.what());
            throw std::runtime_error("Failed to save profile: " +
                                     std::string(e.what()));
        }
    } else {
        LOG_F(ERROR, "Hidden config file does not exist. Cannot save profile.");
        throw std::runtime_error("Hidden config file does not exist.");
    }
}

auto PHD2ProfileSettingHandler::restoreProfile(
    const std::string& toRestoreProfile) -> bool {
    LOG_F(INFO, "Restoring profile: {}", toRestoreProfile);
    fs::path toRestoreFile =
        pImpl->PHD2_PROFILE_SAVE_PATH / (toRestoreProfile + ".json");
    if (fs::exists(toRestoreFile)) {
        try {
            fs::copy_file(toRestoreFile,
                          ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                          fs::copy_options::overwrite_existing);
            loadProfileFile();
            LOG_F(INFO, "Profile {} restored successfully.",
                  toRestoreProfile);
            return true;
        } catch (const fs::filesystem_error& e) {
            LOG_F(ERROR, "Failed to restore profile {}: {}",
                  toRestoreProfile, e.what());
            return false;
        }
    } else {
        LOG_F(WARNING, "Profile {} does not exist. Creating new profile.",
              toRestoreProfile);
        return newProfileSetting(toRestoreProfile);
    }
}

// New functionality implementations

auto PHD2ProfileSettingHandler::listProfiles() const
    -> std::vector<std::string> {
    LOG_F(INFO, "Listing all profiles.");
    std::vector<std::string> profiles;
    try {
        for (const auto& entry :
             fs::directory_iterator(pImpl->PHD2_PROFILE_SAVE_PATH)) {
            if (entry.path().extension() == ".json") {
                profiles.push_back(entry.path().stem().string());
            }
        }
        LOG_F(INFO, "Found %zu profiles.", profiles.size());
    } catch (const fs::filesystem_error& e) {
        LOG_F(ERROR, "Error listing profiles: {}", e.what());
        throw std::runtime_error("Error listing profiles: " +
                                 std::string(e.what()));
    }
    return profiles;
}

auto PHD2ProfileSettingHandler::exportProfile(
    const std::string& profileName, const fs::path& exportPath) const -> bool {
    LOG_F(INFO, "Exporting profile {} to {}", profileName,
          exportPath);
    fs::path sourceFile =
        pImpl->PHD2_PROFILE_SAVE_PATH / (profileName + ".json");
    if (fs::exists(sourceFile)) {
        try {
            fs::copy_file(sourceFile, exportPath,
                          fs::copy_options::overwrite_existing);
            LOG_F(INFO, "Profile {} exported successfully to {}.",
                  profileName, exportPath);
            return true;
        } catch (const fs::filesystem_error& e) {
            LOG_F(ERROR, "Failed to export profile {}: {}", profileName,
                  e.what());
            return false;
        }
    }
    LOG_F(WARNING, "Profile {} does not exist. Cannot export.",
          profileName);
    return false;
}

auto PHD2ProfileSettingHandler::importProfile(
    const fs::path& importPath, const std::string& newProfileName) -> bool {
    LOG_F(INFO, "Importing profile from {} as {}", importPath,
          newProfileName);
    if (fs::exists(importPath)) {
        fs::path destinationFile =
            pImpl->PHD2_PROFILE_SAVE_PATH / (newProfileName + ".json");
        try {
            fs::copy_file(importPath, destinationFile,
                          fs::copy_options::overwrite_existing);
            LOG_F(INFO, "Profile imported successfully as {}.",
                  newProfileName);
            return true;
        } catch (const fs::filesystem_error& e) {
            LOG_F(ERROR, "Failed to import profile as {}: {}",
                  newProfileName, e.what());
            return false;
        }
    }
    LOG_F(WARNING, "Import path {} does not exist. Cannot import profile.",
          importPath);
    return false;
}

auto PHD2ProfileSettingHandler::compareProfiles(
    const std::string& profile1, const std::string& profile2) const -> bool {
    LOG_F(INFO, "Comparing profiles: {} and {}", profile1,
          profile2);
    fs::path file1 = pImpl->PHD2_PROFILE_SAVE_PATH / (profile1 + ".json");
    fs::path file2 = pImpl->PHD2_PROFILE_SAVE_PATH / (profile2 + ".json");

    if (!fs::exists(file1) || !fs::exists(file2)) {
        LOG_F(ERROR, "One or both profiles do not exist.");
        return false;
    }

    try {
        json config1 = pImpl->loadJsonFile(file1);
        json config2 = pImpl->loadJsonFile(file2);

        bool areEqual = (config1 == config2);
        if (areEqual) {
            LOG_F(INFO, "Profiles {} and {} are identical.", profile1,
                  profile2);
        } else {
            LOG_F(INFO, "Profiles {} and {} have differences.",
                  profile1, profile2);
        }
        return areEqual;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Error comparing profiles: {}", e.what());
        return false;
    }
}

void PHD2ProfileSettingHandler::printProfileDetails(
    const std::string& profileName) const {
    LOG_F(INFO, "Printing details of profile: {}", profileName);
    fs::path profileFile =
        pImpl->PHD2_PROFILE_SAVE_PATH / (profileName + ".json");
    if (fs::exists(profileFile)) {
        try {
            json config = pImpl->loadJsonFile(profileFile);
            std::cout << "Profile: " << profileName << std::endl;
            std::cout << "Details:" << std::endl;
            std::cout << config.dump(4) << std::endl;
            LOG_F(INFO, "Profile details printed successfully.");
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Failed to print profile details: {}", e.what());
            throw std::runtime_error("Failed to print profile details: " +
                                     std::string(e.what()));
        }
    } else {
        LOG_F(WARNING, "Profile {} does not exist.", profileName);
        std::cout << "Profile " << profileName << " does not exist."
                  << std::endl;
    }
}