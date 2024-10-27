#pragma once
#include <asio.hpp>
#include <cassert>
#include <filesystem>
#include <functional>
#include <map>
#include <regex>
#include <string>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/macro.hpp"

namespace atom::io {

namespace fs = std::filesystem;

/**
 * @brief Class for performing asynchronous file globbing operations.
 */
class AsyncGlob {
public:
    /**
     * @brief Constructs an AsyncGlob object.
     * @param io_context The ASIO I/O context.
     */
    AsyncGlob(asio::io_context& io_context);

    /**
     * @brief Performs a glob operation to match files.
     * @param pathname The pattern to match files.
     * @param callback The callback function to call with the matched files.
     * @param recursive Whether to search directories recursively.
     * @param dironly Whether to match directories only.
     */
    void glob(const std::string& pathname,
              const std::function<void(std::vector<fs::path>)>& callback,
              bool recursive = false, bool dironly = false);

private:
    /**
     * @brief Replaces all occurrences of a substring in a string.
     * @param str The string to modify.
     * @param from The substring to replace.
     * @param toStr The string to replace with.
     */
    void stringReplace(std::string& str, const std::string& from,
                       const std::string& toStr);

    /**
     * @brief Translates a glob pattern to a regular expression.
     * @param pattern The glob pattern.
     * @return The translated regular expression.
     */
    std::string translate(const std::string& pattern);

    /**
     * @brief Compiles a glob pattern into a regular expression.
     * @param pattern The glob pattern.
     * @return The compiled regular expression.
     */
    std::regex compilePattern(const std::string& pattern);

    /**
     * @brief Matches a file name against a glob pattern.
     * @param name The file name.
     * @param pattern The glob pattern.
     * @return True if the file name matches the pattern, false otherwise.
     */
    bool fnmatch(const fs::path& name, const std::string& pattern);

    /**
     * @brief Filters a list of file names against a glob pattern.
     * @param names The list of file names.
     * @param pattern The glob pattern.
     * @return The filtered list of file names.
     */
    std::vector<fs::path> filter(const std::vector<fs::path>& names,
                                 const std::string& pattern);

    /**
     * @brief Expands a tilde in a file path to the home directory.
     * @param path The file path.
     * @return The expanded file path.
     */
    fs::path expandTilde(fs::path path);

    /**
     * @brief Checks if a pathname contains glob magic characters.
     * @param pathname The pathname to check.
     * @return True if the pathname contains magic characters, false otherwise.
     */
    bool hasMagic(const std::string& pathname);

    /**
     * @brief Checks if a pathname is hidden.
     * @param pathname The pathname to check.
     * @return True if the pathname is hidden, false otherwise.
     */
    bool isHidden(const std::string& pathname);

    /**
     * @brief Checks if a glob pattern is recursive.
     * @param pattern The glob pattern.
     * @return True if the pattern is recursive, false otherwise.
     */
    bool isRecursive(const std::string& pattern);

    /**
     * @brief Iterates over a directory and calls a callback with the file
     * names.
     * @param dirname The directory to iterate.
     * @param dironly Whether to match directories only.
     * @param callback The callback function to call with the file names.
     */
    void iterDirectory(
        const fs::path& dirname, bool dironly,
        const std::function<void(std::vector<fs::path>)>& callback);

    /**
     * @brief Recursively lists files in a directory and calls a callback with
     * the file names.
     * @param dirname The directory to list.
     * @param dironly Whether to match directories only.
     * @param callback The callback function to call with the file names.
     */
    void rlistdir(const fs::path& dirname, bool dironly,
                  const std::function<void(std::vector<fs::path>)>& callback);

    /**
     * @brief Performs a glob operation in a directory.
     * @param dirname The directory to search.
     * @param pattern The glob pattern.
     * @param dironly Whether to match directories only.
     * @param callback The callback function to call with the matched files.
     */
    void glob2(const fs::path& dirname, const std::string& pattern,
               bool dironly,
               const std::function<void(std::vector<fs::path>)>& callback);

    /**
     * @brief Performs a glob operation in a directory.
     * @param dirname The directory to search.
     * @param pattern The glob pattern.
     * @param dironly Whether to match directories only.
     * @param callback The callback function to call with the matched files.
     */
    void glob1(const fs::path& dirname, const std::string& pattern,
               bool dironly,
               const std::function<void(std::vector<fs::path>)>& callback);

    /**
     * @brief Performs a glob operation in a directory.
     * @param dirname The directory to search.
     * @param basename The base name to match.
     * @param dironly Whether to match directories only.
     * @param callback The callback function to call with the matched files.
     */
    void glob0(const fs::path& dirname, const fs::path& basename, bool dironly,
               const std::function<void(std::vector<fs::path>)>& callback);

    asio::io_context& io_context_;  ///< The ASIO I/O context.
};

}  // namespace atom::io
