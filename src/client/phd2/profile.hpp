#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct InterfacePHD2Profile {
    std::string name;
    std::string camera;
    std::string cameraCCD;
    double pixelSize;
    std::string telescope;
    double focalLength;
    double massChangeThreshold;
    bool massChangeFlag;
    double calibrationDistance;
    double calibrationDuration;
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
        -> std::optional<InterfacePHD2Profile>;  // Added [[nodiscard]]
    auto loadProfile(const std::string& profileName) -> bool;
    auto newProfileSetting(const std::string& newProfileName) -> bool;
    auto updateProfile(const InterfacePHD2Profile& phd2ProfileSetting) -> bool;
    auto deleteProfile(const std::string& toDeleteProfile) -> bool;
    void saveProfile(const std::string& profileName);
    auto restoreProfile(const std::string& toRestoreProfile) -> bool;

    // New functionality
    [[nodiscard]] auto listProfiles() const
        -> std::vector<std::string>;  // Added [[nodiscard]]
    [[nodiscard]] auto exportProfile(const std::string& profileName,
                                     const std::filesystem::path& exportPath)
        const -> bool;  // Added [[nodiscard]]
    auto importProfile(const std::filesystem::path& importPath,
                       const std::string& newProfileName) -> bool;
    [[nodiscard]] auto compareProfiles(const std::string& profile1,
                                       const std::string& profile2) const
        -> bool;  // Added [[nodiscard]]
    void printProfileDetails(const std::string& profileName) const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};