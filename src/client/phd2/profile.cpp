#include "profile.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>

#include "atom/type/json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

struct ServerConfigData {
    static inline const fs::path PHD2_HIDDEN_CONFIG_FILE =
        "./phd2_hidden_config.json";
    static inline const fs::path DEFAULT_PHD2_CONFIG_FILE =
        "./default_phd2_config.json";
};

class PHD2ProfileSettingHandler::Impl {
public:
    std::optional<InterfacePHD2Profile> loaded_config_status;
    const fs::path phd2_profile_save_path = "./server/data/phd2";

    static void replace_double_marker(const fs::path& file_path) {
        std::ifstream input_file(file_path);
        std::string content((std::istreambuf_iterator<char>(input_file)),
                            std::istreambuf_iterator<char>());
        input_file.close();

        size_t pos = content.find("\"\"#");
        while (pos != std::string::npos) {
            content.replace(pos, 3, "#");
            pos = content.find("\"\"#", pos + 1);
        }

        std::ofstream output_file(file_path);
        output_file << content;
        output_file.close();
    }

    json load_json_file(const fs::path& file_path) const {
        std::ifstream file(file_path);
        json config;
        file >> config;
        return config;
    }

    void save_json_file(const fs::path& file_path, const json& config) const {
        std::ofstream file(file_path);
        file << config.dump(4);
        file.close();
        replace_double_marker(file_path);
    }
};

PHD2ProfileSettingHandler::PHD2ProfileSettingHandler()
    : pImpl(std::make_unique<Impl>()) {}
PHD2ProfileSettingHandler::~PHD2ProfileSettingHandler() = default;
PHD2ProfileSettingHandler::PHD2ProfileSettingHandler(
    PHD2ProfileSettingHandler&&) noexcept = default;
PHD2ProfileSettingHandler& PHD2ProfileSettingHandler::operator=(
    PHD2ProfileSettingHandler&&) noexcept = default;

std::optional<InterfacePHD2Profile>
PHD2ProfileSettingHandler::loadProfileFile() {
    if (!fs::exists(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE)) {
        fs::copy_file(ServerConfigData::DEFAULT_PHD2_CONFIG_FILE,
                      ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                      fs::copy_options::overwrite_existing);
    }

    try {
        json phd2_config =
            pImpl->load_json_file(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE);
        pImpl->loaded_config_status = InterfacePHD2Profile{
            .name = phd2_config["profile"]["1"]["name"],
            .camera = phd2_config["profile"]["1"]["indi"]["INDIcam"],
            .cameraCCD = phd2_config["profile"]["1"]["indi"]["INDIcam_ccd"],
            .pixelSize = phd2_config["profile"]["1"]["camera"]["pixelsize"],
            .telescope = phd2_config["profile"]["1"]["indi"]["INDImount"],
            .focalLength = phd2_config["profile"]["1"]["frame"]["focalLength"],
            .massChangeThreshold =
                phd2_config["profile"]["1"]["guider"]["onestar"]
                           ["MassChangeThreshold"],
            .massChangeFlag = phd2_config["profile"]["1"]["guider"]["onestar"]
                                         ["MassChangeThresholdEnabled"],
            .calibrationDistance =
                phd2_config["profile"]["1"]["scope"]["CalibrationDistance"],
            .calibrationDuration =
                phd2_config["profile"]["1"]["scope"]["CalibrationDuration"]};
    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        fs::remove(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE);
        fs::copy_file(ServerConfigData::DEFAULT_PHD2_CONFIG_FILE,
                      ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                      fs::copy_options::overwrite_existing);
        return loadProfileFile();  // Recursive call with default config
    }

    return pImpl->loaded_config_status;
}

bool PHD2ProfileSettingHandler::loadProfile(const std::string& profileName) {
    fs::path profile_file =
        pImpl->phd2_profile_save_path / (profileName + ".json");

    if (fs::exists(profile_file)) {
        fs::copy_file(profile_file, ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                      fs::copy_options::overwrite_existing);
        loadProfileFile();
        return true;
    } else {
        fs::copy_file(ServerConfigData::DEFAULT_PHD2_CONFIG_FILE,
                      ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                      fs::copy_options::overwrite_existing);
        loadProfileFile();
        return false;
    }
}

bool PHD2ProfileSettingHandler::newProfileSetting(
    const std::string& newProfileName) {
    fs::path new_profile_file =
        pImpl->phd2_profile_save_path / (newProfileName + ".json");

    if (fs::exists(new_profile_file)) {
        restoreProfile(newProfileName);
        return false;
    } else {
        fs::copy_file(ServerConfigData::DEFAULT_PHD2_CONFIG_FILE,
                      ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                      fs::copy_options::overwrite_existing);
        loadProfileFile();
        return true;
    }
}

bool PHD2ProfileSettingHandler::updateProfile(
    const InterfacePHD2Profile& phd2ProfileSetting) {
    json phd2_config =
        pImpl->load_json_file(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE);

    phd2_config["profile"]["1"]["name"] = phd2ProfileSetting.name;
    phd2_config["profile"]["1"]["indi"]["INDIcam"] = phd2ProfileSetting.camera;
    phd2_config["profile"]["1"]["indi"]["INDIcam_ccd"] =
        phd2ProfileSetting.cameraCCD;
    phd2_config["profile"]["1"]["camera"]["pixelsize"] =
        phd2ProfileSetting.pixelSize;
    phd2_config["profile"]["1"]["indi"]["INDImount"] =
        phd2ProfileSetting.telescope;
    phd2_config["profile"]["1"]["frame"]["focalLength"] =
        phd2ProfileSetting.focalLength;
    phd2_config["profile"]["1"]["guider"]["onestar"]["MassChangeThreshold"] =
        phd2ProfileSetting.massChangeThreshold;
    phd2_config["profile"]["1"]["guider"]["onestar"]
               ["MassChangeThresholdEnabled"] =
                   phd2ProfileSetting.massChangeFlag;
    phd2_config["profile"]["1"]["scope"]["CalibrationDistance"] =
        phd2ProfileSetting.calibrationDistance;
    phd2_config["profile"]["1"]["scope"]["CalibrationDuration"] =
        phd2ProfileSetting.calibrationDuration;

    pImpl->save_json_file(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                          phd2_config);
    return true;
}

bool PHD2ProfileSettingHandler::deleteProfile(
    const std::string& toDeleteProfile) {
    fs::path to_delete_profile_file =
        pImpl->phd2_profile_save_path / (toDeleteProfile + ".json");
    if (fs::exists(to_delete_profile_file)) {
        fs::remove(to_delete_profile_file);
        return true;
    }
    return false;
}

void PHD2ProfileSettingHandler::saveProfile(const std::string& profileName) {
    fs::path profile_file =
        pImpl->phd2_profile_save_path / (profileName + ".json");
    if (fs::exists(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE)) {
        if (fs::exists(profile_file)) {
            fs::remove(profile_file);
        }
        fs::copy_file(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE, profile_file,
                      fs::copy_options::overwrite_existing);
    }
}

bool PHD2ProfileSettingHandler::restoreProfile(
    const std::string& toRestoreProfile) {
    fs::path to_restore_file =
        pImpl->phd2_profile_save_path / (toRestoreProfile + ".json");
    if (fs::exists(to_restore_file)) {
        fs::copy_file(to_restore_file,
                      ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                      fs::copy_options::overwrite_existing);
        loadProfileFile();
        return true;
    } else {
        newProfileSetting(toRestoreProfile);
        return false;
    }
}

// New functionality implementations

std::vector<std::string> PHD2ProfileSettingHandler::listProfiles() const {
    std::vector<std::string> profiles;
    for (const auto& entry :
         fs::directory_iterator(pImpl->phd2_profile_save_path)) {
        if (entry.path().extension() == ".json") {
            profiles.push_back(entry.path().stem().string());
        }
    }
    return profiles;
}

bool PHD2ProfileSettingHandler::exportProfile(
    const std::string& profileName, const fs::path& exportPath) const {
    fs::path source_file =
        pImpl->phd2_profile_save_path / (profileName + ".json");
    if (fs::exists(source_file)) {
        fs::copy_file(source_file, exportPath,
                      fs::copy_options::overwrite_existing);
        return true;
    }
    return false;
}

bool PHD2ProfileSettingHandler::importProfile(
    const fs::path& importPath, const std::string& newProfileName) {
    if (fs::exists(importPath)) {
        fs::path destination_file =
            pImpl->phd2_profile_save_path / (newProfileName + ".json");
        fs::copy_file(importPath, destination_file,
                      fs::copy_options::overwrite_existing);
        return true;
    }
    return false;
}

bool PHD2ProfileSettingHandler::compareProfiles(
    const std::string& profile1, const std::string& profile2) const {
    fs::path file1 = pImpl->phd2_profile_save_path / (profile1 + ".json");
    fs::path file2 = pImpl->phd2_profile_save_path / (profile2 + ".json");

    if (!fs::exists(file1) || !fs::exists(file2)) {
        return false;
    }

    json config1 = pImpl->load_json_file(file1);
    json config2 = pImpl->load_json_file(file2);

    std::cout << "Comparing profiles: " << profile1 << " and " << profile2
              << std::endl;
    std::cout << "Differences:" << std::endl;

    for (auto it = config1.begin(); it != config1.end(); ++it) {
        if (config2.find(it.key()) == config2.end() ||
            config2[it.key()] != it.value()) {
            std::cout << it.key() << ": " << it.value() << " vs "
                      << config2[it.key()] << std::endl;
        }
    }

    for (auto it = config2.begin(); it != config2.end(); ++it) {
        if (config1.find(it.key()) == config1.end()) {
            std::cout << it.key() << ": missing in " << profile1 << std::endl;
        }
    }

    return true;
}

void PHD2ProfileSettingHandler::printProfileDetails(
    const std::string& profileName) const {
    fs::path profile_file =
        pImpl->phd2_profile_save_path / (profileName + ".json");
    if (fs::exists(profile_file)) {
        json config = pImpl->load_json_file(profile_file);
        std::cout << "Profile: " << profileName << std::endl;
        std::cout << "Details:" << std::endl;
        std::cout << config.dump(4) << std::endl;
    } else {
        std::cout << "Profile " << profileName << " does not exist."
                  << std::endl;
    }
}
