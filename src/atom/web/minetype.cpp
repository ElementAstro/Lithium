#include "minetype.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <unordered_map>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

using json = nlohmann::json;

class MimeTypes::Impl {
public:
    Impl(const std::vector<std::string>& knownFiles, bool lenient)
        : lenient_(lenient) {
        for (const auto& file : knownFiles) {
            read(file);
        }
    }

    void readJson(const std::string& jsonFile) {
        std::ifstream file(jsonFile);
        if (!file) {
            LOG_F(WARNING, "Could not open JSON file {}", jsonFile);
            return;
        }

        json jsonData;
        file >> jsonData;

        std::unique_lock lock(mutex_);
        for (const auto& [mimeType, extensions] : jsonData.items()) {
            for (const auto& ext : extensions) {
                addType(mimeType, ext.get<std::string>());
            }
        }
    }

    auto guessType(const std::string& url)
        -> std::pair<std::optional<std::string>, std::optional<std::string>> {
        std::filesystem::path path(url);
        std::string extension = path.extension().string();
        return getMimeType(extension);
    }

    auto guessAllExtensions(const std::string& mimeType)
        -> std::vector<std::string> {
        std::shared_lock lock(mutex_);
        auto iter = reverseMap_.find(mimeType);
        if (iter != reverseMap_.end()) {
            return iter->second;
        }
        return {};
    }

    auto guessExtension(const std::string& mimeType)
        -> std::optional<std::string> {
        auto extensions = guessAllExtensions(mimeType);
        return extensions.empty() ? std::nullopt
                                  : std::make_optional(extensions[0]);
    }

    void addType(const std::string& mimeType, const std::string& extension) {
        std::unique_lock lock(mutex_);
        typesMap_[extension] = mimeType;
        reverseMap_[mimeType].emplace_back(extension);
    }

    void listAllTypes() const {
        std::shared_lock lock(mutex_);
        for (const auto& [ext, type] : typesMap_) {
            LOG_F(INFO, "Extension: {} -> MIME Type: {}", ext, type);
        }
    }

    auto guessTypeByContent(const std::string& filePath)
        -> std::optional<std::string> {
        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            LOG_F(WARNING, "Could not open file {}", filePath);
            return std::nullopt;
        }

        std::array<char, 8> buffer;
        file.read(buffer.data(), buffer.size());

        if (buffer[0] == '\xFF' && buffer[1] == '\xD8') {
            return "image/jpeg";
        }
        if (buffer[0] == '\x89' && buffer[1] == 'P' && buffer[2] == 'N' &&
            buffer[3] == 'G') {
            return "image/png";
        }
        if (buffer[0] == 'G' && buffer[1] == 'I' && buffer[2] == 'F') {
            return "image/gif";
        }
        if (buffer[0] == 'P' && buffer[1] == 'K') {
            return "application/zip";
        }

        return std::nullopt;
    }

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::string> typesMap_;
    std::unordered_map<std::string, std::vector<std::string>> reverseMap_;
    bool lenient_;

    void read(const std::string& file) {
        std::ifstream fileStream(file);
        if (!fileStream) {
            LOG_F(WARNING, "Could not open file {}", file);
            return;
        }

        std::string line;
        while (std::getline(fileStream, line)) {
            if (line.empty() || line[0] == '#') {
                continue;
            }
            std::istringstream iss(line);
            std::string mimeType;
            if (iss >> mimeType) {
                std::string ext;
                while (iss >> ext) {
                    addType(mimeType, ext);
                }
            }
        }
    }

    auto getMimeType(const std::string& extension)
        -> std::pair<std::optional<std::string>, std::optional<std::string>> {
        std::shared_lock lock(mutex_);
        auto iter = typesMap_.find(extension);
        if (iter != typesMap_.end()) {
            return {iter->second, std::nullopt};
        }
        if (lenient_) {
            return {"application/octet-stream", std::nullopt};
        }
        return {std::nullopt, std::nullopt};
    }
};

MimeTypes::MimeTypes(const std::vector<std::string>& knownFiles, bool lenient)
    : pImpl(std::make_unique<Impl>(knownFiles, lenient)) {}

MimeTypes::~MimeTypes() = default;

void MimeTypes::readJson(const std::string& jsonFile) {
    pImpl->readJson(jsonFile);
}

auto MimeTypes::guessType(const std::string& url)
    -> std::pair<std::optional<std::string>, std::optional<std::string>> {
    return pImpl->guessType(url);
}

auto MimeTypes::guessAllExtensions(const std::string& mimeType)
    -> std::vector<std::string> {
    return pImpl->guessAllExtensions(mimeType);
}

auto MimeTypes::guessExtension(const std::string& mimeType)
    -> std::optional<std::string> {
    return pImpl->guessExtension(mimeType);
}

void MimeTypes::addType(const std::string& mimeType,
                        const std::string& extension) {
    pImpl->addType(mimeType, extension);
}

void MimeTypes::listAllTypes() const { pImpl->listAllTypes(); }

auto MimeTypes::guessTypeByContent(const std::string& filePath)
    -> std::optional<std::string> {
    return pImpl->guessTypeByContent(filePath);
}
