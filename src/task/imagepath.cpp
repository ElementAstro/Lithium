#include "imagepath.hpp"

#include <filesystem>
#include <regex>
#include <unordered_map>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

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
    explicit Impl(const std::string& pattern) { parsePattern(pattern); }

    [[nodiscard]] auto parseFilename(const std::string& filename) const
        -> std::optional<ImageInfo> {
        std::smatch matchResult;
        if (!std::regex_match(filename, matchResult, fullRegexPattern_)) {
            LOG_F(ERROR, "Filename does not match the pattern: {}", filename);
            return std::nullopt;
        }

        ImageInfo info;
        info.path = fs::absolute(fs::path(filename)).string();

        for (size_t i = 0; i < fieldKeys_.size(); ++i) {
            const auto& key = fieldKeys_[i];
            const std::string VALUE = matchResult[i + 1];

            if (auto parserIter = parsers_.find(key);
                parserIter != parsers_.end()) {
                parserIter->second(info, VALUE);
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

    void addFieldPattern(const std::string& key,
                         const std::string& regexPattern) {
        fieldPatterns_[key] = regexPattern;
    }

    [[nodiscard]] auto getPatterns() const -> const std::vector<std::string>& {
        return patterns_;
    }

private:
    std::vector<std::string> fieldKeys_;
    std::vector<std::string> patterns_;
    std::unordered_map<std::string, FieldParser> parsers_;
    std::unordered_map<std::string, std::string> optionalFields_;
    std::unordered_map<std::string, std::string> fieldPatterns_;
    std::regex fullRegexPattern_;

    void parsePattern(const std::string& pattern) {
        static const std::regex TOKEN_REGEX(R"(\$(\w+))");
        std::smatch match;
        std::string regexPattern;
        std::string::const_iterator searchStart(pattern.cbegin());

        while (
            std::regex_search(searchStart, pattern.cend(), match, TOKEN_REGEX)) {
            regexPattern += std::regex_replace(
                match.prefix().str(),
                std::regex(R"([\.\+\*\?\^\$\(\)\[\]\{\}\\\|])"), "\\$&");

            std::string key = match[1];
            fieldKeys_.push_back(key);

            std::string fieldPattern = R"(.*)";  // 默认匹配任意内容

            if (auto it = fieldPatterns_.find(key);
                it != fieldPatterns_.end()) {
                fieldPattern = it->second;
            } else {
                // 默认模式，可根据需要调整
                if (key == "DATETIME") {
                    fieldPattern = R"(\d{4}-\d{2}-\d{2}-\d{2}-\d{2}-\d{2})";
                } else if (key == "EXPOSURETIME") {
                    fieldPattern = R"(\d+(\.\d+)?)";
                } else {
                    fieldPattern = R"(\w+)";
                }
            }

            regexPattern += "(" + fieldPattern + ")";
            searchStart = match.suffix().first;
        }
        regexPattern += std::regex_replace(
            std::string(searchStart, pattern.cend()),
            std::regex(R"([\.\+\*\?\^\$\(\)\[\]\{\}\\\|])"), "\\$&");

        patterns_.push_back(regexPattern);

        fullRegexPattern_ = std::regex(regexPattern);

        initializeParsers();
    }

    void initializeParsers() {
        parsers_["DATETIME"] = [](ImageInfo& info, const std::string& value) {
            info.dateTime = value;
        };
        parsers_["IMAGETYPE"] = [](ImageInfo& info, const std::string& value) {
            info.imageType = value;
        };
        parsers_["FILTER"] = [](ImageInfo& info, const std::string& value) {
            info.filter = value;
        };
        parsers_["SENSORTEMP"] = [](ImageInfo& info, const std::string& value) {
            info.sensorTemp = value;
        };
        parsers_["EXPOSURETIME"] = [](ImageInfo& info,
                                      const std::string& value) {
            info.exposureTime = value;
        };
        parsers_["FRAMENR"] = [](ImageInfo& info, const std::string& value) {
            info.frameNr = value;
        };

        // 设置可选字段的默认值
        for (const auto& [key, defaultValue] : optionalFields_) {
            parsers_[key] = [defaultValue, key](ImageInfo& info,
                                                const std::string& value) {
                if (value.empty()) {
                    // 字段缺失，设置为默认值
                    if (key == "DATETIME")
                        info.dateTime = defaultValue;
                    else if (key == "IMAGETYPE")
                        info.imageType = defaultValue;
                    else if (key == "FILTER")
                        info.filter = defaultValue;
                    else if (key == "SENSORTEMP")
                        info.sensorTemp = defaultValue;
                    else if (key == "EXPOSURETIME")
                        info.exposureTime = defaultValue;
                    else if (key == "FRAMENR")
                        info.frameNr = defaultValue;
                } else {
                    // 字段存在，使用实际值
                    if (key == "DATETIME")
                        info.dateTime = value;
                    else if (key == "IMAGETYPE")
                        info.imageType = value;
                    else if (key == "FILTER")
                        info.filter = value;
                    else if (key == "SENSORTEMP")
                        info.sensorTemp = value;
                    else if (key == "EXPOSURETIME")
                        info.exposureTime = value;
                    else if (key == "FRAMENR")
                        info.frameNr = value;
                }
            };
        }
    }
};

ImagePatternParser::ImagePatternParser(const std::string& pattern)
    : pImpl(std::make_unique<Impl>(pattern)) {}

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

void ImagePatternParser::addFieldPattern(const std::string& key,
                                         const std::string& regexPattern) {
    pImpl->addFieldPattern(key, regexPattern);
}

auto ImagePatternParser::getPatterns() const -> std::vector<std::string> {
    return pImpl->getPatterns();
}

}  // namespace lithium