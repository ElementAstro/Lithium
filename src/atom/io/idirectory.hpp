/*
 * idirectory.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-23

Description: Directory Wrapper

**************************************************/

#ifndef ATOM_IO_IDIRECTORY_HPP
#define ATOM_IO_IDIRECTORY_HPP

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace Atom::IO {
/**
 * @brief A wrapper class for interacting with directories.
 */
class DirectoryWrapper {
public:
    /**
     * @brief Constructs a DirectoryWrapper object with the specified directory
     * path.
     * @param dir_path The path to the directory.
     */
    explicit DirectoryWrapper(const fs::path& dir_path);

    /**
     * @brief Constructs a DirectoryWrapper object with the specified directory
     * path.
     * @param dir_path The path to the directory.
     */
    explicit DirectoryWrapper(const std::string& dir_path);

    /**
     * @brief Checks if the directory exists.
     * @return True if the directory exists, otherwise false.
     */
    bool exists() const;

    /**
     * @brief Removes the directory and its contents.
     */
    void remove();

    /**
     * @brief Gets the path of the directory.
     * @return The path of the directory.
     */
    fs::path get_path() const;

    /**
     * @brief Gets the size of the directory in bytes.
     * @return The size of the directory.
     */
    uintmax_t get_size() const;

    /**
     * @brief Gets the size of the directory as a string representation.
     * @return The size of the directory as a string.
     */
    std::string get_size_string() const;

    /**
     * @brief Lists all files in the directory.
     * @return A vector containing paths to all files in the directory.
     */
    std::vector<fs::path> list_files() const;

    /**
     * @brief Lists all subdirectories in the directory.
     * @return A vector containing paths to all subdirectories in the directory.
     */
    std::vector<fs::path> list_directories() const;

    /**
     * @brief Creates a new directory within the current directory.
     * @param name The name of the new directory to create.
     */
    void create_directory(const std::string& name) const;

private:
    fs::path dir_path_;  ///< The path to the directory.
};

}  // namespace Atom::IO

#endif
