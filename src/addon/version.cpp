#include "version.hpp"

#include <format>

#include "atom/error/exception.hpp"

namespace lithium {

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
        THROW_INVALID_ARGUMENT("Invalid version format: ", e.what());
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
    if (requiredVersionStr.size() > 1 && requiredVersionStr[1] == '=') {
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

    THROW_INVALID_ARGUMENT("Invalid comparison operator");
}

}  // namespace lithium
