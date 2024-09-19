#include "imagepath.hpp"

#include <array>
#include <charconv>
#include <filesystem>
#include <regex>
#include <unordered_map>

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

auto ImageInfo::fromJson(const json& jsonObj) -> ImageInfo {
    ImageInfo info;
    try {
        info.path = jsonObj.at("path").get<std::string>();
        info.dateTime = jsonObj.value("dateTime", "");
        info.imageType = jsonObj.value("imageType", "");
        info.filter = jsonObj.value("filter", "");
        info.sensorTemp = jsonObj.value("sensorTemp", "");
        info.exposureTime = jsonObj.value("exposureTime", "");
        info.frameNr = jsonObj.value("frameNr", "");
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Error deserializing ImageInfo from JSON: {}", e.what());
    }
    return info;
}

class ImagePatternParser::Impl {
public:
    explicit Impl(const std::string& pattern, char delimiter)
        : delimiter_(delimiter) {
        parsePattern(pattern);
    }

    [[nodiscard]] auto parseFilename(const std::string& filename) const
        -> std::optional<ImageInfo> {
        ImageInfo info;
        info.path = fs::absolute(fs::path(filename)).string();

        // Remove file extension
        auto name = filename.substr(0, filename.find_last_of('.'));

        // Split into parts
        auto parts = atom::utils::splitString(name, delimiter_);
        if (parts.size() < patterns_.size()) {
            LOG_F(ERROR, "Filename does not match the pattern: {}", name);
            return std::nullopt;
        }

// Assign parts dynamically based on the pattern
#pragma unroll
        for (size_t index = 0; index < patterns_.size(); ++index) {
            const auto& key = patterns_[index];
            const auto& value = parts[index];
            if (auto parserIter = parsers_.find(key);
                parserIter != parsers_.end()) {
                parserIter->second(info, value);
            } else {
                LOG_F(ERROR, "No parser for key: {}", key);
            }
        }

        return info;
    }

    void addCustomParser(const std::string& key, FieldParser parser) {
        parsers_[key] = std::move(parser);
    }

    void setOptionalField(const std::string& key,
                          const std::string& defaultValue) {
        optionalFields_[key] = defaultValue;
    }

    [[nodiscard]] auto getPatterns() const -> const std::vector<std::string>& {
        return patterns_;
    }

    [[nodiscard]] auto getDelimiter() const -> char { return delimiter_; }

private:
    std::vector<std::string> patterns_;
    std::unordered_map<std::string, FieldParser> parsers_;
    std::unordered_map<std::string, std::string> optionalFields_;
    char delimiter_;

    void parsePattern(const std::string& pattern) {
        std::string temp;
        bool inVar = false;
#pragma unroll
        for (char character : pattern) {
            if (character == '$') {
                if (inVar) {
                    patterns_.push_back(temp);
                    temp.clear();
                }
                inVar = !inVar;
            } else if (inVar) {
                temp += character;
            }
        }

        initializeParsers();
    }

    void initializeParsers() {
        parsers_["DATETIME"] = [](ImageInfo& info, const std::string& value) {
            info.dateTime = validateDateTime(value)
                                ? std::optional<std::string>(value)
                                : std::nullopt;
        };
        parsers_["IMAGETYPE"] = [](ImageInfo& info, const std::string& value) {
            info.imageType = !value.empty() ? std::optional<std::string>(value)
                                            : std::nullopt;
        };
        parsers_["FILTER"] = [](ImageInfo& info, const std::string& value) {
            info.filter = !value.empty() ? std::optional<std::string>(value)
                                         : std::nullopt;
        };
        parsers_["SENSORTEMP"] = [](ImageInfo& info, const std::string& value) {
            info.sensorTemp = formatTemperature(value);
        };
        parsers_["EXPOSURETIME"] = [](ImageInfo& info,
                                      const std::string& value) {
            if (auto pos = value.find('s'); pos != std::string::npos) {
                info.exposureTime = value.substr(0, pos);
            }
        };
        parsers_["FRAMENR"] = [](ImageInfo& info, const std::string& value) {
            info.frameNr = !value.empty() ? std::optional<std::string>(value)
                                          : std::nullopt;
        };

// Set default values for optional fields
#pragma unroll
        for (const auto& [key, value] : optionalFields_) {
            if (parsers_.find(key) == parsers_.end()) {
                parsers_[key] = [value](ImageInfo&, const std::string&) {
                    // No-op: Assign default value if key is not present
                };
            }
        }
    }

    static auto validateDateTime(const std::string& dateTime) -> bool {
        static const std::regex kDateTimePattern(
            R"(^\d{4}-\d{2}-\d{2}-\d{2}-\d{2}-\d{2}$)");
        return std::regex_match(dateTime, kDateTimePattern);
    }

    static auto formatTemperature(const std::string& temp) -> std::string {
        // Assume temperature is in the format -10.0, format to 1 decimal place
        float temperature;
        auto [ptr, ec] = std::from_chars(temp.data(), temp.data() + temp.size(),
                                         temperature);
        if (ec == std::errc()) {
            std::array<char, 16> buffer;
            auto [ptr2, ec2] =
                std::to_chars(buffer.data(), buffer.data() + buffer.size(),
                              temperature, std::chars_format::fixed, 1);
            if (ec2 == std::errc()) {
                return {buffer.data(),
                        static_cast<size_t>(ptr2 - buffer.data())};
            }
        }
        return temp;  // Return as is if parsing fails
    }
};

ImagePatternParser::ImagePatternParser(const std::string& pattern,
                                       char delimiter)
    : pImpl(std::make_unique<Impl>(pattern, delimiter)) {}

ImagePatternParser::~ImagePatternParser() = default;

ImagePatternParser::ImagePatternParser(ImagePatternParser&&) noexcept = default;
auto ImagePatternParser::operator=(ImagePatternParser&&) noexcept
    -> ImagePatternParser& = default;

auto ImagePatternParser::parseFilename(const std::string& filename) const
    -> std::optional<ImageInfo> {
    return pImpl->parseFilename(filename);
}

auto ImagePatternParser::serializeToJson(const ImageInfo& info) -> json {
    return info.toJson();
}

auto ImagePatternParser::deserializeFromJson(const json& jsonObj) -> ImageInfo {
    return ImageInfo::fromJson(jsonObj);
}

void ImagePatternParser::addCustomParser(const std::string& key,
                                         FieldParser parser) {
    pImpl->addCustomParser(key, std::move(parser));
}

void ImagePatternParser::setOptionalField(const std::string& key,
                                          const std::string& defaultValue) {
    pImpl->setOptionalField(key, defaultValue);
}

auto ImagePatternParser::getPatterns() const -> std::vector<std::string> {
    return pImpl->getPatterns();
}

auto ImagePatternParser::getDelimiter() const -> char {
    return pImpl->getDelimiter();
}

}  // namespace lithium