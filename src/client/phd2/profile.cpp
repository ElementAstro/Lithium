#include "profile.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

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
    : pimpl(std::make_unique<Impl>()) {}
PHD2ProfileSettingHandler::~PHD2ProfileSettingHandler() = default;
PHD2ProfileSettingHandler::PHD2ProfileSettingHandler(
    PHD2ProfileSettingHandler&&) noexcept = default;
PHD2ProfileSettingHandler& PHD2ProfileSettingHandler::operator=(
    PHD2ProfileSettingHandler&&) noexcept = default;

std::optional<InterfacePHD2Profile>
PHD2ProfileSettingHandler::load_profile_file() {
    if (!fs::exists(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE)) {
        fs::copy_file(ServerConfigData::DEFAULT_PHD2_CONFIG_FILE,
                      ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                      fs::copy_options::overwrite_existing);
    }

    try {
        json phd2_config =
            pimpl->load_json_file(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE);
        pimpl->loaded_config_status = InterfacePHD2Profile{
            .name = phd2_config["profile"]["1"]["name"],
            .camera = phd2_config["profile"]["1"]["indi"]["INDIcam"],
            .camera_ccd = phd2_config["profile"]["1"]["indi"]["INDIcam_ccd"],
            .pixel_size = phd2_config["profile"]["1"]["camera"]["pixelsize"],
            .telescope = phd2_config["profile"]["1"]["indi"]["INDImount"],
            .focal_length = phd2_config["profile"]["1"]["frame"]["focalLength"],
            .mass_change_threshold =
                phd2_config["profile"]["1"]["guider"]["onestar"]
                           ["MassChangeThreshold"],
            .mass_change_flag = phd2_config["profile"]["1"]["guider"]["onestar"]
                                           ["MassChangeThresholdEnabled"],
            .calibration_distance =
                phd2_config["profile"]["1"]["scope"]["CalibrationDistance"],
            .calibration_duration =
                phd2_config["profile"]["1"]["scope"]["CalibrationDuration"]};
    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        fs::remove(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE);
        fs::copy_file(ServerConfigData::DEFAULT_PHD2_CONFIG_FILE,
                      ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                      fs::copy_options::overwrite_existing);
        return load_profile_file();  // Recursive call with default config
    }

    return pimpl->loaded_config_status;
}

bool PHD2ProfileSettingHandler::load_profile(const std::string& profile_name) {
    fs::path profile_file =
        pimpl->phd2_profile_save_path / (profile_name + ".json");

    if (fs::exists(profile_file)) {
        fs::copy_file(profile_file, ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                      fs::copy_options::overwrite_existing);
        load_profile_file();
        return true;
    } else {
        fs::copy_file(ServerConfigData::DEFAULT_PHD2_CONFIG_FILE,
                      ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                      fs::copy_options::overwrite_existing);
        load_profile_file();
        return false;
    }
}

bool PHD2ProfileSettingHandler::new_profile_setting(
    const std::string& new_profile_name) {
    fs::path new_profile_file =
        pimpl->phd2_profile_save_path / (new_profile_name + ".json");

    if (fs::exists(new_profile_file)) {
        restore_profile(new_profile_name);
        return false;
    } else {
        fs::copy_file(ServerConfigData::DEFAULT_PHD2_CONFIG_FILE,
                      ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                      fs::copy_options::overwrite_existing);
        load_profile_file();
        return true;
    }
}

bool PHD2ProfileSettingHandler::update_profile(
    const InterfacePHD2Profile& phd2_profile_setting) {
    json phd2_config =
        pimpl->load_json_file(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE);

    phd2_config["profile"]["1"]["name"] = phd2_profile_setting.name;
    phd2_config["profile"]["1"]["indi"]["INDIcam"] =
        phd2_profile_setting.camera;
    phd2_config["profile"]["1"]["indi"]["INDIcam_ccd"] =
        phd2_profile_setting.camera_ccd;
    phd2_config["profile"]["1"]["camera"]["pixelsize"] =
        phd2_profile_setting.pixel_size;
    phd2_config["profile"]["1"]["indi"]["INDImount"] =
        phd2_profile_setting.telescope;
    phd2_config["profile"]["1"]["frame"]["focalLength"] =
        phd2_profile_setting.focal_length;
    phd2_config["profile"]["1"]["guider"]["onestar"]["MassChangeThreshold"] =
        phd2_profile_setting.mass_change_threshold;
    phd2_config["profile"]["1"]["guider"]["onestar"]
               ["MassChangeThresholdEnabled"] =
                   phd2_profile_setting.mass_change_flag;
    phd2_config["profile"]["1"]["scope"]["CalibrationDistance"] =
        phd2_profile_setting.calibration_distance;
    phd2_config["profile"]["1"]["scope"]["CalibrationDuration"] =
        phd2_profile_setting.calibration_duration;

    pimpl->save_json_file(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                          phd2_config);
    return true;
}

bool PHD2ProfileSettingHandler::delete_profile(
    const std::string& to_delete_profile) {
    fs::path to_delete_profile_file =
        pimpl->phd2_profile_save_path / (to_delete_profile + ".json");
    if (fs::exists(to_delete_profile_file)) {
        fs::remove(to_delete_profile_file);
        return true;
    }
    return false;
}

void PHD2ProfileSettingHandler::save_profile(const std::string& profile_name) {
    fs::path profile_file =
        pimpl->phd2_profile_save_path / (profile_name + ".json");
    if (fs::exists(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE)) {
        if (fs::exists(profile_file)) {
            fs::remove(profile_file);
        }
        fs::copy_file(ServerConfigData::PHD2_HIDDEN_CONFIG_FILE, profile_file,
                      fs::copy_options::overwrite_existing);
    }
}

bool PHD2ProfileSettingHandler::restore_profile(
    const std::string& to_restore_profile) {
    fs::path to_restore_file =
        pimpl->phd2_profile_save_path / (to_restore_profile + ".json");
    if (fs::exists(to_restore_file)) {
        fs::copy_file(to_restore_file,
                      ServerConfigData::PHD2_HIDDEN_CONFIG_FILE,
                      fs::copy_options::overwrite_existing);
        load_profile_file();
        return true;
    } else {
        new_profile_setting(to_restore_profile);
        return false;
    }
}

// New functionality implementations

std::vector<std::string> PHD2ProfileSettingHandler::list_profiles() const {
    std::vector<std::string> profiles;
    for (const auto& entry :
         fs::directory_iterator(pimpl->phd2_profile_save_path)) {
        if (entry.path().extension() == ".json") {
            profiles.push_back(entry.path().stem().string());
        }
    }
    return profiles;
}

bool PHD2ProfileSettingHandler::export_profile(
    const std::string& profile_name, const fs::path& export_path) const {
    fs::path source_file =
        pimpl->phd2_profile_save_path / (profile_name + ".json");
    if (fs::exists(source_file)) {
        fs::copy_file(source_file, export_path,
                      fs::copy_options::overwrite_existing);
        return true;
    }
    return false;
}

bool PHD2ProfileSettingHandler::import_profile(
    const fs::path& import_path, const std::string& new_profile_name) {
    if (fs::exists(import_path)) {
        fs::path destination_file =
            pimpl->phd2_profile_save_path / (new_profile_name + ".json");
        fs::copy_file(import_path, destination_file,
                      fs::copy_options::overwrite_existing);
        return true;
    }
    return false;
}

bool PHD2ProfileSettingHandler::compare_profiles(
    const std::string& profile1, const std::string& profile2) const {
    fs::path file1 = pimpl->phd2_profile_save_path / (profile1 + ".json");
    fs::path file2 = pimpl->phd2_profile_save_path / (profile2 + ".json");

    if (!fs::exists(file1) || !fs::exists(file2)) {
        return false;
    }

    json config1 = pimpl->load_json_file(file1);
    json config2 = pimpl->load_json_file(file2);

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

void PHD2ProfileSettingHandler::print_profile_details(
    const std::string& profile_name) const {
    fs::path profile_file =
        pimpl->phd2_profile_save_path / (profile_name + ".json");
    if (fs::exists(profile_file)) {
        json config = pimpl->load_json_file(profile_file);
        std::cout << "Profile: " << profile_name << std::endl;
        std::cout << "Details:" << std::endl;
        std::cout << config.dump(4) << std::endl;
    } else {
        std::cout << "Profile " << profile_name << " does not exist."
                  << std::endl;
    }
}