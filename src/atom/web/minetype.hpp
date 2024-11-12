#ifndef MIMETYPES_H
#define MIMETYPES_H

#include <memory>
#include <optional>
#include <string>
#include <vector>

/**
 * @class MimeTypes
 * @brief A class for handling MIME types and file extensions.
 */
class MimeTypes {
public:
    /**
     * @brief Constructs a MimeTypes object.
     * @param knownFiles A vector of known file paths.
     * @param lenient A flag indicating whether to be lenient in MIME type
     * detection.
     */
    MimeTypes(const std::vector<std::string>& knownFiles, bool lenient = false);

    /**
     * @brief Destructor.
     */
    ~MimeTypes();

    /**
     * @brief Reads MIME types from a JSON file.
     * @param jsonFile The path to the JSON file.
     */
    void readJson(const std::string& jsonFile);

    /**
     * @brief Guesses the MIME type and charset of a URL.
     * @param url The URL to guess the MIME type for.
     * @return A pair containing the guessed MIME type and charset, if
     * available.
     */
    std::pair<std::optional<std::string>, std::optional<std::string>> guessType(
        const std::string& url);

    /**
     * @brief Guesses all possible file extensions for a given MIME type.
     * @param mimeType The MIME type to guess extensions for.
     * @return A vector of possible file extensions.
     */
    std::vector<std::string> guessAllExtensions(const std::string& mimeType);

    /**
     * @brief Guesses the file extension for a given MIME type.
     * @param mimeType The MIME type to guess the extension for.
     * @return The guessed file extension, if available.
     */
    std::optional<std::string> guessExtension(const std::string& mimeType);

    /**
     * @brief Adds a new MIME type and file extension pair.
     * @param mimeType The MIME type to add.
     * @param extension The file extension to associate with the MIME type.
     */
    void addType(const std::string& mimeType, const std::string& extension);

    /**
     * @brief Lists all known MIME types and their associated file extensions.
     */
    void listAllTypes() const;

    /**
     * @brief Guesses the MIME type of a file based on its content.
     * @param filePath The path to the file.
     * @return The guessed MIME type, if available.
     */
    std::optional<std::string> guessTypeByContent(const std::string& filePath);

private:
    class Impl;  ///< Forward declaration of the implementation class.
    std::unique_ptr<Impl> pImpl;  ///< Pointer to the implementation.
};

#endif  // MIMETYPES_H