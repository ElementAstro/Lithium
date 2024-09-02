#include "imagepath.hpp"

#include <filesystem>
#include <regex>
#include <sstream>
#include <utility>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/string.hpp"
namespace fs = std::filesystem;

namespace lithium {
auto ImageInfo::toJson() const -> json {
    return {{"path", path},
            {"dateTime", dateTime.value_or("")},
            {"imageType", imageType.value_or("")},
            {"filter", filter.value_or("")},
            {"sensorTemp", sensorTemp.value_or("")},
            {"exposureTime", exposureTime.value_or("")},
            {"frameNr", frameNr.value_or("")}};
}

auto ImageInfo::fromJson(const json& j) -> ImageInfo {
    ImageInfo info;
    try {
        info.path = j.at("path").get<std::string>();
        info.dateTime = j.value("dateTime", "");
        info.imageType = j.value("imageType", "");
        info.filter = j.value("filter", "");
        info.sensorTemp = j.value("sensorTemp", "");
        info.exposureTime = j.value("exposureTime", "");
        info.frameNr = j.value("frameNr", "");
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Error deserializing ImageInfo from JSON: {}",
              e.what());
    }
    return info;
}
ImagePatternParser::ImagePatternParser(const std::string& pattern,
                                       char delimiter)
    : delimiter_(delimiter) {
    parsePattern(pattern);
}

auto ImagePatternParser::parseFilename(const std::string& filename) const
    -> std::optional<ImageInfo> {
    ImageInfo info;
    info.path = fs::absolute(fs::path(filename)).string();

    // Remove file extension
    std::string name = filename.substr(0, filename.find_last_of('.'));

    // Split into parts
    std::vector<std::string> parts = atom::utils::splitString(name, delimiter_);
    if (parts.size() < patterns_.size()) {
        LOG_F(ERROR, "Filename does not match the pattern: {}", name);
        return std::nullopt;
    }

    // Assign parts dynamically based on the pattern
    for (size_t i = 0; i < patterns_.size(); ++i) {
        const auto& key = patterns_[i];
        const auto& value = parts[i];
        if (parsers_.find(key) != parsers_.end()) {
            parsers_.at(key)(info, value);
        } else {
            LOG_F(ERROR, "No parser for key: {}", key);
        }
    }

    return info;
}

auto ImagePatternParser::serializeToJson(const ImageInfo& info) -> json {
    return info.toJson();
}

auto ImagePatternParser::deserializeFromJson(const json& j) -> ImageInfo {
    return ImageInfo::fromJson(j);
}

// Allow adding custom parsers for new elements
void ImagePatternParser::addCustomParser(const std::string& key,
                                         FieldParser parser) {
    parsers_[key] = std::move(parser);
}

// Allow optional fields
void ImagePatternParser::setOptionalField(const std::string& key,
                                          const std::string& defaultValue) {
    optionalFields_[key] = defaultValue;
}

void ImagePatternParser::parsePattern(const std::string& pattern) {
    std::string temp;
    bool inVar = false;
    for (char ch : pattern) {
        if (ch == '$') {
            if (inVar) {
                patterns_.push_back(temp);
                temp.clear();
            }
            inVar = !inVar;
        } else if (inVar) {
            temp += ch;
        }
    }

    // Initialize parsers
    parsers_["DATETIME"] = [](ImageInfo& info, const std::string& value) {
        info.dateTime = validateDateTime(value)
                            ? std::optional<std::string>(value)
                            : std::nullopt;
    };
    parsers_["IMAGETYPE"] = [](ImageInfo& info, const std::string& value) {
        info.imageType =
            value.empty() ? std::nullopt : std::optional<std::string>(value);
    };
    parsers_["FILTER"] = [](ImageInfo& info, const std::string& value) {
        info.filter =
            value.empty() ? std::nullopt : std::optional<std::string>(value);
    };
    parsers_["SENSORTEMP"] = [](ImageInfo& info, const std::string& value) {
        info.sensorTemp = formatTemperature(value);
    };
    parsers_["EXPOSURETIME"] = [](ImageInfo& info, const std::string& value) {
        size_t pos = value.find('s');
        if (pos != std::string::npos) {
            info.exposureTime = value.substr(0, pos);
        }
    };
    parsers_["FRAMENR"] = [](ImageInfo& info, const std::string& value) {
        info.frameNr =
            value.empty() ? std::nullopt : std::optional<std::string>(value);
    };

    // Set default values for optional fields
    for (const auto& [key, value] : optionalFields_) {
        if (parsers_.find(key) == parsers_.end()) {
            parsers_[key] = [value]([[maybe_unused]] ImageInfo& info,
                                    const std::string&) {
                // No-op: Assign default value if key is not present
            };
        }
    }
}

auto ImagePatternParser::validateDateTime(const std::string& dateTime) -> bool {
    // Simple regex validation for DateTime: YYYY-MM-DD-HH-MM-SS
    std::regex dateTimePattern(R"(^\d{4}-\d{2}-\d{2}-\d{2}-\d{2}-\d{2}$)");
    return std::regex_match(dateTime, dateTimePattern);
}

auto ImagePatternParser::formatTemperature(const std::string& temp)
    -> std::string {
    // Assume temperature is in the format -10.0, format to 1 decimal place
    std::ostringstream oss;
    try {
        float t = std::stof(temp);
        oss.precision(1);
        oss << std::fixed << t;
    } catch (const std::exception&) {
        return temp;  // Return as is if parsing fails
    }
    return oss.str();
}
}  // namespace lithium
