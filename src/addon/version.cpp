#include "version.hpp"

#include <charconv>
#include <format>

#include "atom/error/exception.hpp"

namespace lithium {

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

auto Version::toString() const -> std::string {
    auto result = std::format("{}.{}.{}", major, minor, patch);
    if (!prerelease.empty()) {
        result += "-" + prerelease;
    }
    if (!build.empty()) {
        result += "+" + build;
    }

    return result;
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

constexpr auto DateVersion::parse(std::string_view dateStr) -> DateVersion {
    size_t pos = 0;
    auto nextDash = dateStr.find('-', pos);
    if (nextDash == std::string_view::npos) {
        THROW_INVALID_ARGUMENT("Invalid date format");
    }

    int year = parseInt(dateStr.substr(pos, nextDash - pos));
    pos = nextDash + 1;

    nextDash = dateStr.find('-', pos);
    if (nextDash == std::string_view::npos) {
        THROW_INVALID_ARGUMENT("Invalid date format");
    }

    int month = parseInt(dateStr.substr(pos, nextDash - pos));
    int day = parseInt(dateStr.substr(nextDash + 1));

    if (month < 1 || month > 12 || day < 1 || day > 31) {
        THROW_INVALID_ARGUMENT("Invalid date values");
    }

    return {year, month, day};
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

auto operator<<(std::ostream& os, const DateVersion& version) -> std::ostream& {
    os << version.year << '-' << version.month << '-' << version.day;
    return os;
}

auto checkVersion(const Version& actualVersion,
                  const std::string& requiredVersionStr) -> bool {
    size_t opLength = 1;
    if (requiredVersionStr.size() > 1 &&
        (requiredVersionStr[1] == '=' || requiredVersionStr[1] == '>')) {
        opLength = 2;
    }

    std::string op = requiredVersionStr.substr(0, opLength);
    std::string versionPart = requiredVersionStr.substr(opLength);
    Version requiredVersion;

    try {
        requiredVersion = Version::parse(versionPart);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid version format: " << versionPart << std::endl;
        throw;
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
    if (op == ">")
        return actualVersion > requiredVersion;
    if (op == "<")
        return actualVersion < requiredVersion;
    if (op == ">=")
        return actualVersion >= requiredVersion;
    if (op == "<=")
        return actualVersion <= requiredVersion;
    if (op == "=")
        return actualVersion == requiredVersion;

    return actualVersion == requiredVersion;
}

auto checkDateVersion(const DateVersion& actualVersion,
                      const std::string& requiredVersionStr) -> bool {
    size_t opLength = 1;
    if (requiredVersionStr.size() > 1 && requiredVersionStr[1] == '=') {
        opLength = 2;
    }

    std::string op = requiredVersionStr.substr(0, opLength);
    DateVersion requiredVersion =
        DateVersion::parse(requiredVersionStr.substr(opLength));

    if (op == ">")
        return actualVersion > requiredVersion;
    if (op == "<")
        return actualVersion < requiredVersion;
    if (op == ">=")
        return actualVersion >= requiredVersion;
    if (op == "<=")
        return actualVersion <= requiredVersion;
    if (op == "=")
        return actualVersion == requiredVersion;

    THROW_INVALID_ARGUMENT("Invalid comparison operator");
}

}  // namespace lithium
