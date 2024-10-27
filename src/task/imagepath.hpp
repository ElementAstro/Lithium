#ifndef LITHIUM_TASK_IMAGEPATH_HPP
#define LITHIUM_TASK_IMAGEPATH_HPP

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "atom/macro.hpp"

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

    bool operator==(const ImageInfo&) const = default;
} ATOM_ALIGNAS(128);

class ImagePatternParser {
public:
    using FieldParser = std::function<void(ImageInfo&, const std::string&)>;

    explicit ImagePatternParser(const std::string& pattern,
                                char delimiter = '_');
    ~ImagePatternParser();

    // Disable copy operations
    ImagePatternParser(const ImagePatternParser&) = delete;
    ImagePatternParser& operator=(const ImagePatternParser&) = delete;

    // Enable move operations
    ImagePatternParser(ImagePatternParser&&) noexcept;
    ImagePatternParser& operator=(ImagePatternParser&&) noexcept;

    [[nodiscard]] auto parseFilename(const std::string& filename) const
        -> std::optional<ImageInfo>;

    static auto serializeToJson(const ImageInfo& info) -> json;
    static auto deserializeFromJson(const json& j) -> ImageInfo;

    void addCustomParser(const std::string& key, FieldParser parser);
    void setOptionalField(const std::string& key,
                          const std::string& defaultValue);

    // New methods
    [[nodiscard]] auto getPatterns() const -> std::vector<std::string>;
    [[nodiscard]] auto getDelimiter() const -> char;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

// Template function to parse multiple filenames
template <typename... Filenames>
[[nodiscard]] auto parseMultipleFilenames(const ImagePatternParser& parser,
                                          const Filenames&... filenames)
    -> std::vector<std::optional<ImageInfo>> {
    return {parser.parseFilename(filenames)...};
}

}  // namespace lithium

#endif
