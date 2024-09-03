#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct InterfacePHD2Profile {
    std::string name;
    std::string camera;
    std::string camera_ccd;
    double pixel_size;
    std::string telescope;
    double focal_length;
    double mass_change_threshold;
    bool mass_change_flag;
    double calibration_distance;
    double calibration_duration;
};

class PHD2ProfileSettingHandler {
public:
    PHD2ProfileSettingHandler();
    ~PHD2ProfileSettingHandler();

    // Disable copy operations
    PHD2ProfileSettingHandler(const PHD2ProfileSettingHandler&) = delete;
    PHD2ProfileSettingHandler& operator=(const PHD2ProfileSettingHandler&) =
        delete;

    // Enable move operations
    PHD2ProfileSettingHandler(PHD2ProfileSettingHandler&&) noexcept;
    PHD2ProfileSettingHandler& operator=(PHD2ProfileSettingHandler&&) noexcept;

    std::optional<InterfacePHD2Profile> load_profile_file();
    bool load_profile(const std::string& profile_name);
    bool new_profile_setting(const std::string& new_profile_name);
    bool update_profile(const InterfacePHD2Profile& phd2_profile_setting);
    bool delete_profile(const std::string& to_delete_profile);
    void save_profile(const std::string& profile_name);
    bool restore_profile(const std::string& to_restore_profile);

    // New functionality
    std::vector<std::string> list_profiles() const;
    bool export_profile(const std::string& profile_name,
                        const std::filesystem::path& export_path) const;
    bool import_profile(const std::filesystem::path& import_path,
                        const std::string& new_profile_name);
    bool compare_profiles(const std::string& profile1,
                          const std::string& profile2) const;
    void print_profile_details(const std::string& profile_name) const;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
