#ifndef LITHIUM_ADDON_VERSION_HPP
#define LITHIUM_ADDON_VERSION_HPP

#include <iostream>
#include <string>
#include <string_view>
#include <utility>

#include "atom/error/exception.hpp"

#include "macro.hpp"

namespace lithium {

struct Version {
    int major;
    int minor;
    int patch;
    std::string prerelease;  // alpha, beta, rc etc.
    std::string build;       // build metadata

    constexpr Version() : major(0), minor(0), patch(0) {}

    constexpr Version(int maj, int min, int pat, std::string pre = "",
                      std::string bld = "")
        : major(maj),
          minor(min),
          patch(pat),
          prerelease(std::move(pre)),
          build(std::move(bld)) {}

    static constexpr auto parse(std::string_view versionStr) -> Version;
    [[nodiscard]] auto toString() const -> std::string;

    constexpr auto operator<(const Version& other) const -> bool;
    constexpr auto operator>(const Version& other) const -> bool;
    constexpr auto operator==(const Version& other) const -> bool;
    constexpr auto operator<=(const Version& other) const -> bool;
    constexpr auto operator>=(const Version& other) const -> bool;
} ATOM_ALIGNAS(128);

auto operator<<(std::ostream& os, const Version& version) -> std::ostream&;

struct DateVersion {
    int year;
    int month;
    int day;

    constexpr DateVersion(int y, int m, int d) : year(y), month(m), day(d) {}

    static constexpr auto parse(std::string_view dateStr) -> DateVersion;

    constexpr auto operator<(const DateVersion& other) const -> bool;
    constexpr auto operator>(const DateVersion& other) const -> bool;
    constexpr auto operator==(const DateVersion& other) const -> bool;
    constexpr auto operator<=(const DateVersion& other) const -> bool;
    constexpr auto operator>=(const DateVersion& other) const -> bool;
} ATOM_ALIGNAS(16);

auto operator<<(std::ostream& os, const DateVersion& version) -> std::ostream&;

auto checkVersion(const Version& actualVersion,
                  const std::string& requiredVersionStr) -> bool;
auto checkDateVersion(const DateVersion& actualVersion,
                      const std::string& requiredVersionStr) -> bool;

constexpr auto parseInt(std::string_view str) -> int {
    int result = 0;
    auto [ptr, ec] =
        std::from_chars(str.data(), str.data() + str.size(), result);
    if (ec != std::errc()) {
        THROW_INVALID_ARGUMENT("Invalid integer format");
    }
    return result;
}

constexpr auto Version::parse(std::string_view versionStr) -> Version {
    size_t pos = 0;
    auto nextDot = versionStr.find('.', pos);
    if (nextDot == std::string_view::npos) {
        THROW_INVALID_ARGUMENT("Invalid version format");
    }

    int major = parseInt(versionStr.substr(pos, nextDot - pos));
    pos = nextDot + 1;

    nextDot = versionStr.find('.', pos);
    if (nextDot == std::string_view::npos) {
        THROW_INVALID_ARGUMENT("Invalid version format");
    }

    int minor = parseInt(versionStr.substr(pos, nextDot - pos));
    pos = nextDot + 1;

    auto nextDash = versionStr.find('-', pos);
    auto nextPlus = versionStr.find('+', pos);
    size_t endPos = std::min(nextDash, nextPlus);

    int patch = parseInt(versionStr.substr(pos, endPos - pos));

    std::string prerelease = (nextDash != std::string_view::npos)
                                 ? std::string(versionStr.substr(
                                       nextDash + 1, nextPlus - nextDash - 1))
                                 : "";

    std::string build = (nextPlus != std::string_view::npos)
                            ? std::string(versionStr.substr(nextPlus + 1))
                            : "";

    return {major, minor, patch, prerelease, build};
}

constexpr auto Version::operator<(const Version& other) const -> bool {
    if (major != other.major) {
        return major < other.major;
    }
    if (minor != other.minor) {
        return minor < other.minor;
    }
    if (patch != other.patch) {
        return patch < other.patch;
    }
    if (prerelease.empty() && other.prerelease.empty()) {
        return false;
    }
    if (prerelease.empty()) {
        return false;
    }
    if (other.prerelease.empty()) {
        return true;
    }
    return prerelease < other.prerelease;
}

constexpr auto Version::operator>(const Version& other) const -> bool {
    return other < *this;
}
constexpr auto Version::operator==(const Version& other) const -> bool {
    return major == other.major && minor == other.minor &&
           patch == other.patch && prerelease == other.prerelease;
}
constexpr auto Version::operator<=(const Version& other) const -> bool {
    return *this < other || *this == other;
}
constexpr auto Version::operator>=(const Version& other) const -> bool {
    return *this > other || *this == other;
}

constexpr auto DateVersion::operator<(const DateVersion& other) const -> bool {
    if (year != other.year) {
        return year < other.year;
    }

    if (month != other.month) {
        return month < other.month;
    }
    return day < other.day;
}

constexpr auto DateVersion::operator>(const DateVersion& other) const -> bool {
    return other < *this;
}
constexpr auto DateVersion::operator==(const DateVersion& other) const -> bool {
    return year == other.year && month == other.month && day == other.day;
}
constexpr auto DateVersion::operator<=(const DateVersion& other) const -> bool {
    return *this < other || *this == other;
}
constexpr auto DateVersion::operator>=(const DateVersion& other) const -> bool {
    return *this > other || *this == other;
}

}  // namespace lithium

#endif
