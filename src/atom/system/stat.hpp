/*
 * stat.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: Python like stat for Windows & Linux

**************************************************/

#ifndef ATOM_SYSTEM_STAT_HPP
#define ATOM_SYSTEM_STAT_HPP

#include <ctime>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace atom::system {
/**
 * @brief Class representing file statistics.
 *
 * This class provides methods to retrieve various attributes of a file,
 * such as its type, size, access time, modification time, and so on.
 */
class Stat {
public:
    /**
     * @brief Constructs a `Stat` object for the specified file path.
     *
     * @param path The path to the file whose statistics are to be retrieved.
     */
    explicit Stat(const fs::path& path);

    /**
     * @brief Updates the file statistics.
     *
     * This method refreshes the statistics for the file specified in the
     * constructor.
     */
    void update();

    /**
     * @brief Gets the type of the file.
     *
     * @return The type of the file as an `fs::file_type` enum value.
     */
    [[nodiscard]] auto type() const -> fs::file_type;

    /**
     * @brief Gets the size of the file.
     *
     * @return The size of the file in bytes.
     */
    [[nodiscard]] auto size() const -> std::uintmax_t;

    /**
     * @brief Gets the last access time of the file.
     *
     * @return The last access time of the file as a `std::time_t` value.
     */
    [[nodiscard]] auto atime() const -> std::time_t;

    /**
     * @brief Gets the last modification time of the file.
     *
     * @return The last modification time of the file as a `std::time_t` value.
     */
    [[nodiscard]] auto mtime() const -> std::time_t;

    /**
     * @brief Gets the creation time of the file.
     *
     * @return The creation time of the file as a `std::time_t` value.
     */
    [[nodiscard]] auto ctime() const -> std::time_t;

    /**
     * @brief Gets the file mode/permissions.
     *
     * @return The file mode/permissions as an integer value.
     */
    [[nodiscard]] auto mode() const -> int;

    /**
     * @brief Gets the user ID of the file owner.
     *
     * @return The user ID of the file owner as an integer value.
     */
    [[nodiscard]] auto uid() const -> int;

    /**
     * @brief Gets the group ID of the file owner.
     *
     * @return The group ID of the file owner as an integer value.
     */
    [[nodiscard]] auto gid() const -> int;

    /**
     * @brief Gets the path of the file.
     *
     * @return The path of the file as an `fs::path` object.
     */
    [[nodiscard]] auto path() const -> fs::path;

private:
    fs::path path_;  ///< The path to the file.
    std::error_code
        ec_;  ///< The error code for handling errors during file operations.
};
}  // namespace atom::system

#endif  // ATOM_SYSTEM_STAT_HPP
