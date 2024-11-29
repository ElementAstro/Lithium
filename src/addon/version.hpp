#ifndef LITHIUM_ADDON_VERSION_HPP
#define LITHIUM_ADDON_VERSION_HPP

#include <charconv>
#include <string>
#include <string_view>
#include <utility>

#include "atom/error/exception.hpp"

#include "atom/macro.hpp"

namespace lithium {

/**
 * @struct Version
 * @brief Represents a semantic version.
 */
struct Version {
    int major;  ///< Major version number.
    int minor;  ///< Minor version number.
    int patch;  ///< Patch version number.
    std::string
        prerelease;     ///< Prerelease information (e.g., alpha, beta, rc).
    std::string build;  ///< Build metadata.

    /**
     * @brief Default constructor initializing version to 0.0.0.
     */
    constexpr Version() : major(0), minor(0), patch(0) {}

    /**
     * @brief Constructs a Version with specified major, minor, patch,
     * prerelease, and build.
     * @param maj Major version number.
     * @param min Minor version number.
     * @param pat Patch version number.
     * @param pre Prerelease information.
     * @param bld Build metadata.
     */
    constexpr Version(int maj, int min, int pat, std::string pre = "",
                      std::string bld = "")
        : major(maj),
          minor(min),
          patch(pat),
          prerelease(std::move(pre)),
          build(std::move(bld)) {}

    /**
     * @brief Parses a version string into a Version object.
     * @param versionStr The version string to parse.
     * @return The parsed Version object.
     */
    static constexpr auto parse(std::string_view versionStr) -> Version;

    /**
     * @brief Converts the Version object to a string.
     * @return The version as a string.
     */
    [[nodiscard]] auto toString() const -> std::string;

    /**
     * @brief Less-than comparison operator.
     * @param other The other Version to compare to.
     * @return True if this version is less than the other version.
     */
    constexpr auto operator<(const Version& other) const -> bool;

    /**
     * @brief Greater-than comparison operator.
     * @param other The other Version to compare to.
     * @return True if this version is greater than the other version.
     */
    constexpr auto operator>(const Version& other) const -> bool;

    /**
     * @brief Equality comparison operator.
     * @param other The other Version to compare to.
     * @return True if this version is equal to the other version.
     */
    constexpr auto operator==(const Version& other) const -> bool;

    /**
     * @brief Less-than-or-equal-to comparison operator.
     * @param other The other Version to compare to.
     * @return True if this version is less than or equal to the other version.
     */
    constexpr auto operator<=(const Version& other) const -> bool;

    /**
     * @brief Greater-than-or-equal-to comparison operator.
     * @param other The other Version to compare to.
     * @return True if this version is greater than or equal to the other
     * version.
     */
    constexpr auto operator>=(const Version& other) const -> bool;
} ATOM_ALIGNAS(128);

/**
 * @brief Stream insertion operator for Version.
 * @param os The output stream.
 * @param version The Version object to insert.
 * @return The output stream.
 */
auto operator<<(std::ostream& os, const Version& version) -> std::ostream&;

/**
 * @struct DateVersion
 * @brief Represents a date-based version.
 */
struct DateVersion {
    int year;   ///< Year component of the date.
    int month;  ///< Month component of the date.
    int day;    ///< Day component of the date.

    /**
     * @brief Constructs a DateVersion with specified year, month, and day.
     * @param y Year component.
     * @param m Month component.
     * @param d Day component.
     */
    constexpr DateVersion(int y, int m, int d) : year(y), month(m), day(d) {}

    /**
     * @brief Parses a date string into a DateVersion object.
     * @param dateStr The date string to parse.
     * @return The parsed DateVersion object.
     */
    static constexpr auto parse(std::string_view dateStr) -> DateVersion;

    /**
     * @brief Less-than comparison operator.
     * @param other The other DateVersion to compare to.
     * @return True if this date is less than the other date.
     */
    constexpr auto operator<(const DateVersion& other) const -> bool;

    /**
     * @brief Greater-than comparison operator.
     * @param other The other DateVersion to compare to.
     * @return True if this date is greater than the other date.
     */
    constexpr auto operator>(const DateVersion& other) const -> bool;

    /**
     * @brief Equality comparison operator.
     * @param other The other DateVersion to compare to.
     * @return True if this date is equal to the other date.
     */
    constexpr auto operator==(const DateVersion& other) const -> bool;

    /**
     * @brief Less-than-or-equal-to comparison operator.
     * @param other The other DateVersion to compare to.
     * @return True if this date is less than or equal to the other date.
     */
    constexpr auto operator<=(const DateVersion& other) const -> bool;

    /**
     * @brief Greater-than-or-equal-to comparison operator.
     * @param other The other DateVersion to compare to.
     * @return True if this date is greater than or equal to the other date.
     */
    constexpr auto operator>=(const DateVersion& other) const -> bool;
} ATOM_ALIGNAS(16);

/**
 * @brief Stream insertion operator for DateVersion.
 * @param os The output stream.
 * @param version The DateVersion object to insert.
 * @return The output stream.
 */
auto operator<<(std::ostream& os, const DateVersion& version) -> std::ostream&;

/**
 * @brief Checks if the actual version meets the required version.
 * @param actualVersion The actual Version object.
 * @param requiredVersionStr The required version string.
 * @return True if the actual version meets the required version.
 */
auto checkVersion(const Version& actualVersion,
                  const std::string& requiredVersionStr) -> bool;

/**
 * @brief Checks if the actual date version meets the required date version.
 * @param actualVersion The actual DateVersion object.
 * @param requiredVersionStr The required date version string.
 * @return True if the actual date version meets the required date version.
 */
auto checkDateVersion(const DateVersion& actualVersion,
                      const std::string& requiredVersionStr) -> bool;

/**
 * @brief Parses a string into an integer.
 * @param str The string to parse.
 * @return The parsed integer.
 * @throws std::invalid_argument if the string is not a valid integer.
 */
constexpr auto parseInt(std::string_view str) -> int {
    int result = 0;
    auto [ptr, ec] =
        std::from_chars(str.data(), str.data() + str.size(), result);
    if (ec != std::errc()) {
        THROW_INVALID_ARGUMENT("Invalid integer format");
    }
    return result;
}

/**
 * @brief Parses a version string into a Version object.
 * @param versionStr The version string to parse.
 * @return The parsed Version object.
 * @throws std::invalid_argument if the version string is not valid.
 */
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

#endif  // LITHIUM_ADDON_VERSION_HPP