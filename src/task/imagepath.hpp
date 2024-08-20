#ifndef LITHIUM_TASK_IMAGEPATH_HPP
#define LITHIUM_TASK_IMAGEPATH_HPP

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "macro.hpp"

#include "atom/type/json_fwd.hpp"
using json = nlohmann::json;

namespace lithium {
struct ImageInfo {
    std::string path;
    std::optional<std::string> dateTime;
    std::optional<std::string> imageType;
    std::optional<std::string> filter;
    std::optional<std::string> sensorTemp;
    std::optional<std::string> exposureTime;
    std::optional<std::string> frameNr;

    [[nodiscard]] auto toJson() const -> json;

    static auto fromJson(const json& j) -> ImageInfo;
} ATOM_ALIGNAS(128);

class ImagePatternParser {
public:
    using FieldParser = std::function<void(ImageInfo&, const std::string&)>;

    explicit ImagePatternParser(const std::string& pattern,
                                char delimiter = '_');

    auto parseFilename(const std::string& filename) const
        -> std::optional<ImageInfo>;

    static auto serializeToJson(const ImageInfo& info) -> json;

    static auto deserializeFromJson(const json& j) -> ImageInfo;

    // Allow adding custom parsers for new elements
    void addCustomParser(const std::string& key, FieldParser parser);

    // Allow optional fields
    void setOptionalField(const std::string& key,
                          const std::string& defaultValue);

private:
    std::vector<std::string> patterns_;
    std::unordered_map<std::string, FieldParser> parsers_;
    std::unordered_map<std::string, std::string> optionalFields_;
    char delimiter_;

    void parsePattern(const std::string& pattern);

    static auto validateDateTime(const std::string& dateTime) -> bool;

    static auto formatTemperature(const std::string& temp) -> std::string;
};
}  // namespace lithium

#endif