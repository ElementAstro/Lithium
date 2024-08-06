#ifndef LITHIUM_ADDON_VERSION_HPP
#define LITHIUM_ADDON_VERSION_HPP

#include <string>

#include "macro.hpp"

namespace lithium {
struct Version {
    int major;
    int minor;
    int patch;
    std::string prerelease;  // alpha, beta, rc等
    std::string build;       // build metadata

    Version();

    Version(int maj, int min, int pat, std::string pre = "",
            std::string bld = "");

    static auto parse(const std::string& versionStr) -> Version;

    auto operator<(const Version& other) const -> bool;

    auto operator>(const Version& other) const -> bool;

    auto operator==(const Version& other) const -> bool;

    auto operator<=(const Version& other) const -> bool;

    auto operator>=(const Version& other) const -> bool;
} ATOM_ALIGNAS(128);

auto operator<<(std::ostream& os, const Version& version) -> std::ostream&;

// 日期版本解析
struct DateVersion {
    int year;
    int month;
    int day;

    DateVersion(int y, int m, int d);

    static auto parse(const std::string& dateStr) -> DateVersion;

    auto operator<(const DateVersion& other) const -> bool;

    auto operator>(const DateVersion& other) const -> bool;

    auto operator==(const DateVersion& other) const -> bool;

    auto operator<=(const DateVersion& other) const -> bool;

    auto operator>=(const DateVersion& other) const -> bool;
} ATOM_ALIGNAS(16);

auto operator<<(std::ostream& os, const DateVersion& version) -> std::ostream&;

auto checkVersion(const Version& actualVersion,
                  const std::string& requiredVersionStr) -> bool;

auto checkDateVersion(const DateVersion& actualVersion,
                      const std::string& requiredVersionStr) -> bool;

}  // namespace lithium

#endif
