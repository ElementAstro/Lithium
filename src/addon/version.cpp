#include "version.hpp"

#include <iostream>
#include <regex>
#include <string>
#include <utility>

namespace lithium {
Version::Version() : major(0), minor(0), patch(0) {}

Version::Version(int maj, int min, int pat, std::string pre, std::string bld)
    : major(maj),
      minor(min),
      patch(pat),
      prerelease(std::move(pre)),
      build(std::move(bld)) {}

auto Version::parse(const std::string& versionStr) -> Version {
    std::regex versionPattern(
        R"((\d+)\.(\d+)\.(\d+)(?:-([0-9A-Za-z.-]+))?(?:\+([0-9A-Za-z.-]+))?)");
    std::smatch match;
    if (std::regex_match(versionStr, match, versionPattern)) {
        int major = std::stoi(match[1].str());
        int minor = std::stoi(match[2].str());
        int patch = std::stoi(match[3].str());
        std::string prerelease = match[4].matched ? match[4].str() : "";
        std::string build = match[5].matched ? match[5].str() : "";
        return {major, minor, patch, prerelease, build};
    }
    throw std::invalid_argument("Invalid version format");
}

auto Version::operator<(const Version& other) const -> bool {
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

auto Version::operator>(const Version& other) const -> bool {
    return other < *this;
}

auto Version::operator==(const Version& other) const -> bool {
    return major == other.major && minor == other.minor &&
           patch == other.patch && prerelease == other.prerelease;
}

auto Version::operator<=(const Version& other) const -> bool {
    return *this < other || *this == other;
}

auto Version::operator>=(const Version& other) const -> bool {
    return *this > other || *this == other;
}

auto operator<<(std::ostream& os, const Version& version) -> std::ostream& {
    os << version.major << '.' << version.minor << '.' << version.patch;
    if (!version.prerelease.empty()) {
        os << '-' << version.prerelease;
    }
    if (!version.build.empty()) {
        os << '+' << version.build;
    }
    return os;
}

DateVersion::DateVersion(int y, int m, int d) : year(y), month(m), day(d) {}

auto DateVersion::parse(const std::string& dateStr) -> DateVersion {
    std::regex datePattern(R"((\d{4})-(\d{2})-(\d{2}))");
    std::smatch match;
    if (std::regex_match(dateStr, match, datePattern)) {
        int year = std::stoi(match[1].str());
        int month = std::stoi(match[2].str());
        int day = std::stoi(match[3].str());
        // Check if month and day are valid
        if (month < 1 || month > 12 || day < 1 || day > 31) {
            throw std::invalid_argument("Invalid date format");
        }
        return {year, month, day};
    }
    throw std::invalid_argument("Invalid date format");
}

auto DateVersion::operator<(const DateVersion& other) const -> bool {
    if (year != other.year) {
        return year < other.year;
    }
    if (month != other.month) {
        return month < other.month;
    }
    return day < other.day;
}

auto DateVersion::operator>(const DateVersion& other) const -> bool {
    return other < *this;
}

auto DateVersion::operator==(const DateVersion& other) const -> bool {
    return year == other.year && month == other.month && day == other.day;
}

auto DateVersion::operator<=(const DateVersion& other) const -> bool {
    return *this < other || *this == other;
}

auto DateVersion::operator>=(const DateVersion& other) const -> bool {
    return *this > other || *this == other;
}

auto operator<<(std::ostream& os, const DateVersion& version) -> std::ostream& {
    os << version.year << '-' << version.month << '-' << version.day;
    return os;
}

auto checkVersion(const Version& actualVersion,
                  const std::string& requiredVersionStr) -> bool {
    size_t opLength = 1;
    if (requiredVersionStr[1] == '=' || requiredVersionStr[1] == '>') {
        opLength = 2;
    }

    std::string op = requiredVersionStr.substr(0, opLength);
    std::string versionPart = requiredVersionStr.substr(opLength);
    Version requiredVersion;

    try {
        requiredVersion = Version::parse(versionPart);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid version format: " << versionPart << std::endl;
        throw;  // Rethrow or handle the error appropriately
    }

    if (op == "^") {
        return actualVersion.major == requiredVersion.major &&
               actualVersion >= requiredVersion;
    }
    if (op == "~") {
        return actualVersion.major == requiredVersion.major &&
               actualVersion.minor == requiredVersion.minor &&
               actualVersion >= requiredVersion;
    }
    if (op == ">") {
        return actualVersion > requiredVersion;
    }
    if (op == "<") {
        return actualVersion < requiredVersion;
    }
    if (op == ">=") {
        return actualVersion >= requiredVersion;
    }
    if (op == "<=") {
        return actualVersion <= requiredVersion;
    }
    if (op == "=") {
        return actualVersion == requiredVersion;
    }
    return actualVersion == requiredVersion;
}

auto checkDateVersion(const DateVersion& actualVersion,
                      const std::string& requiredVersionStr) -> bool {
    size_t opLength = 1;
    if (requiredVersionStr[1] == '=') {
        opLength = 2;
    }

    std::string op = requiredVersionStr.substr(0, opLength);
    DateVersion requiredVersion =
        DateVersion::parse(requiredVersionStr.substr(opLength));

    if (op == ">") {
        return actualVersion > requiredVersion;
    }
    if (op == "<") {
        return actualVersion < requiredVersion;
    }
    if (op == ">=") {
        return actualVersion >= requiredVersion;
    }
    if (op == "<=") {
        return actualVersion <= requiredVersion;
    }
    if (op == "=") {
        return actualVersion == requiredVersion;
    }
    throw std::invalid_argument("Invalid comparison operator");
}
}  // namespace lithium
