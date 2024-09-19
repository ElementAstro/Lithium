#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct InterfacePHD2Profile {
    std::string name;
    std::string camera;
    std::string cameraCCD;  // Changed to camelCase
    double pixelSize;       // Changed to camelCase
    std::string telescope;
    double focalLength;           // Changed to camelCase
    double massChangeThreshold;   // Changed to camelCase
    bool massChangeFlag;          // Changed to camelCase
    double calibrationDistance;   // Changed to camelCase
    double calibrationDuration;   // Changed to camelCase
} __attribute__((aligned(128)));  // Align to 128 bytes

class PHD2ProfileSettingHandler {
public:
    PHD2ProfileSettingHandler();
    ~PHD2ProfileSettingHandler();

    // Disable copy operations
    PHD2ProfileSettingHandler(const PHD2ProfileSettingHandler&) = delete;
    auto operator=(const PHD2ProfileSettingHandler&)
        -> PHD2ProfileSettingHandler& = delete;

    // Enable move operations
    PHD2ProfileSettingHandler(PHD2ProfileSettingHandler&&) noexcept;
    auto operator=(PHD2ProfileSettingHandler&&) noexcept
        -> PHD2ProfileSettingHandler&;

    [[nodiscard]] auto loadProfileFile()
        -> std::optional<InterfacePHD2Profile>;  // Changed to camelCase and
                                                 // added [[nodiscard]]
    auto loadProfile(const std::string& profileName)
        -> bool;  // Changed to camelCase
    auto newProfileSetting(const std::string& newProfileName)
        -> bool;  // Changed to camelCase
    auto updateProfile(const InterfacePHD2Profile& phd2ProfileSetting)
        -> bool;  // Changed to camelCase
    auto deleteProfile(const std::string& toDeleteProfile)
        -> bool;                                       // Changed to camelCase
    void saveProfile(const std::string& profileName);  // Changed to camelCase
    auto restoreProfile(const std::string& toRestoreProfile)
        -> bool;  // Changed to camelCase

    // New functionality
    [[nodiscard]] auto listProfiles() const
        -> std::vector<std::string>;  // Changed to camelCase and added
                                      // [[nodiscard]]
    [[nodiscard]] auto exportProfile(const std::string& profileName,
                                     const std::filesystem::path& exportPath)
        const -> bool;  // Changed to camelCase and added [[nodiscard]]
    auto importProfile(const std::filesystem::path& importPath,
                       const std::string& newProfileName)
        -> bool;  // Changed to camelCase
    [[nodiscard]] auto compareProfiles(const std::string& profile1,
                                       const std::string& profile2) const
        -> bool;  // Changed to camelCase and added [[nodiscard]]
    void printProfileDetails(
        const std::string& profileName) const;  // Changed to camelCase

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;  // Changed to camelCase
};