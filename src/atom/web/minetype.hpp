#ifndef MIMETYPES_H
#define MIMETYPES_H

#include <memory>
#include <optional>
#include <string>
#include <vector>

class MimeTypes {
public:
    MimeTypes(const std::vector<std::string>& knownFiles, bool lenient = false);
    ~MimeTypes();

    void readJson(const std::string& jsonFile);
    std::pair<std::optional<std::string>, std::optional<std::string>> guessType(
        const std::string& url);
    std::vector<std::string> guessAllExtensions(const std::string& mimeType);
    std::optional<std::string> guessExtension(const std::string& mimeType);
    void addType(const std::string& mimeType, const std::string& extension);
    void listAllTypes() const;
    std::optional<std::string> guessTypeByContent(const std::string& filePath);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif  // MIMETYPES_H