#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace fs = std::filesystem;

namespace Atom::IO {

class FileWrapper {
public:
    /**
     * @brief Constructs a FileWrapper object with the given file path.
     * @param file_path The path to the file.
     */
    explicit FileWrapper(const fs::path& file_path);

    /**
     * @brief Writes string content to the file.
     * @param content The string content to be written.
     */
    void write(const std::string& content);

    /**
     * @brief Writes binary content to the file.
     * @param content The binary content to be written.
     */
    void write(const std::vector<uint8_t>& content);

    /**
     * @brief Reads content from the file.
     * @return A variant containing either a string or binary content.
     */
    std::variant<std::string, std::vector<uint8_t>> read();

    /**
     * @brief Checks if the file exists.
     * @return True if the file exists, false otherwise.
     */
    bool exists() const;

    /**
     * @brief Removes the file.
     */
    void remove();

    /**
     * @brief Gets the path of the file.
     * @return The path of the file.
     */
    fs::path get_path() const;

    /**
     * @brief Checks if the file is a binary file.
     * @return True if the file is binary, false otherwise.
     */
    bool is_binary_file() const;

    /**
     * @brief Gets the size of the file.
     * @return The size of the file in bytes.
     */
    uintmax_t get_size() const;

    /**
     * @brief Gets the size of the file as a human-readable string.
     * @return The size of the file as a string (e.g., "10 KB").
     */
    std::string get_size_string() const;

    /**
     * @brief Gets the last write time of the file.
     * @return A string representing the last write time of the file.
     */
    std::string get_last_write_time() const;

    /**
     * @brief Renames the file.
     * @param new_path The new path of the file.
     */
    void rename(const fs::path& new_path);

    /**
     * @brief Copies the file to a new location.
     * @param destination The destination path.
     */
    void copy_to(const fs::path& destination) const;

    /**
     * @brief Moves the file to a new location.
     * @param destination The destination path.
     */
    void move_to(const fs::path& destination);

    /**
     * @brief Checks if the file is empty.
     * @return True if the file is empty, false otherwise.
     */
    bool is_empty() const;

    /**
     * @brief Appends content to the file.
     * @param content The content to be appended.
     */
    void append(const std::string& content);

    /**
     * @brief Appends binary content to the file.
     * @param content The binary content to be appended.
     */
    void append(const std::vector<uint8_t>& content);

    /**
     * @brief Writes content to the file at a specific position.
     * @param content The content to be written.
     * @param position The position to write the content.
     */
    void write_at(const std::string& content, std::streampos position);

    /**
     * @brief Writes binary content to the file at a specific position.
     * @param content The binary content to be written.
     * @param position The position to write the content.
     */
    void write_at(const std::vector<uint8_t>& content, std::streampos position);

    /**
     * @brief Reads content from the file at a specific position.
     * @param start The starting position.
     * @param count The number of bytes to read.
     * @return A variant containing either a string or binary content.
     */
    std::variant<std::string, std::vector<uint8_t>> read_from(
        std::streampos start, std::streamsize count);

    /**
     * @brief Gets the extension of the file.
     * @return The extension of the file.
     */
    std::string get_extension() const;

    /**
     * @brief Gets the stem of the file.
     * @return The stem of the file.
     */
    std::string get_stem() const;

    /**
     * @brief Gets the parent path of the file.
     * @return The parent path of the file.
     */
    std::string get_parent_path() const;

    /**
     * @brief Checks if the file is a directory.
     * @return True if the file is a directory, false otherwise.
     */
    bool is_directory() const;

    /**
     * @brief Checks if the file is a regular file.
     * @return True if the file is a regular file, false otherwise.
     */
    bool is_regular_file() const;

    /**
     * @brief Checks if the file is a symbolic link.
     * @return True if the file is a symbolic link, false otherwise.
     */
    bool is_symlink() const;

    /**
     * @brief Gets the hard link count of the file.
     * @return The hard link count of the file.
     */
    std::uintmax_t get_hard_link_count() const;

    /**
     * @brief Creates a symbolic link to the file.
     * @param target The target path of the symbolic link.
     */
    void create_symlink(const fs::path& target);

    /**
     * @brief Creates a hard link to the file.
     * @param target The target path of the hard link.
     */
    void create_hardlink(const fs::path& target);

    /**
     * @brief Sets the permissions of the file.
     * @param prms The permissions to set.
     */
    void permissions(fs::perms prms);

    /**
     * @brief Gets the permissions of the file.
     * @return The permissions of the file.
     */
    fs::perms permissions() const;

private:
    fs::path file_path_;

    /**
     * @brief Writes content to the file.
     * @tparam T The type of content to be written.
     * @param content The content to be written.
     */
    template <typename T>
    void write_file(const T& content) {
        auto file = std::ofstream(file_path_, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to write " + file_path_.string());
        }
        file.write(reinterpret_cast<const char*>(content.data()),
                   content.size());
    }

    /**
     * @brief Reads content from the file.
     * @tparam T The type of content to be read.
     * @return The content read from the file.
     */
    template <typename T>
    T read_file() {
        auto file = std::ifstream(file_path_, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to read " + file_path_.string());
        }
        return T(std::istreambuf_iterator<char>(file), {});
    }
};
}  // namespace Atom::IO
