/*
 * finder.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: Component finder (the core of the plugin system)

**************************************************/

#pragma once

#include <filesystem>
#include <functional>
#include <vector>

namespace Lithium {
/**
 * @brief The DirContainer class represents a container for directory contents.
 */
class DirContainer {
public:
    /**
     * @brief Constructs a DirContainer object with the specified path.
     * @param path The path of the directory represented by this container.
     */
    explicit DirContainer(const std::filesystem::path &path);

    /**
     * @brief Gets the path of the directory represented by this container.
     * @return The path of the directory represented by this container.
     */
    const std::filesystem::path &getPath() const;

    /**
     * @brief Gets the subdirectories of the directory represented by this
     * container.
     * @return The subdirectories of the directory represented by this
     * container.
     */
    const std::vector<DirContainer> &getSubdirs() const;

    /**
     * @brief Gets the files of the directory represented by this container.
     * @return The files of the directory represented by this container.
     */
    const std::vector<std::filesystem::path> &getFiles() const;

    /**
     * @brief Adds a subdirectory to the directory represented by this
     * container.
     * @param subdir The subdirectory to add.
     */
    void addSubdir(const DirContainer &subdir);

    /**
     * @brief Adds a file to the directory represented by this container.
     * @param file The file to add.
     */
    void addFile(const std::filesystem::path &file);

private:
    std::filesystem::path m_path; /**< The path of the directory. */
    std::vector<DirContainer>
        m_subdirs; /**< Subdirectories within the directory. */
    std::vector<std::filesystem::path>
        m_files; /**< Files within the directory. */
};

/**
 * @brief The AddonFinder class is responsible for finding components within a
 * given directory.
 */
class AddonFinder {
public:
    /**
     * @brief FilterFunction represents a function type used for filtering
     * paths.
     */
    using FilterFunction = std::function<bool(const std::filesystem::path &)>;

    /**
     * @brief Constructs a AddonFinder object with the specified path and filter
     * function.
     * @param path The path to the directory to search for components.
     * @param filterFunc The function used to filter paths within the directory
     * (optional).
     */
    explicit AddonFinder(const std::filesystem::path &path,
                         const FilterFunction &filterFunc = {});

    /**
     * @brief Traverses the directory structure and populates the DirContainer
     * object.
     * @param path The path of the directory to traverse.
     * @return True if the traversal was successful, false otherwise.
     */
    bool traverseDir(const std::filesystem::path &path);

    /**
     * @brief Gets the names of the subdirectories that match the filter
     * function.
     * @return The names of the subdirectories that match the filter function.
     */
    std::vector<std::string> getAvailableDirs() const;

    /**
     * @brief Checks if a file with the specified name exists within the given
     * path.
     * @param path The path to the directory to search for the file.
     * @param filename The name of the file to search for.
     * @return True if the file exists, false otherwise.
     */
    static bool hasFile(const std::filesystem::path &path,
                        const std::string &filename);

private:
    /**
     * @brief Recursively traverses the directory structure and populates the
     * DirContainer object.
     * @param path The path of the directory to traverse.
     * @param container The DirContainer object to populate with directory
     * contents.
     */
    static void traverseDir(const std::filesystem::path &path,
                            DirContainer &container);

private:
    std::filesystem::path m_path; /**< The path of the directory. */
    DirContainer m_dirContainer;  /**< The DirContainer object representing the
                                     directory. */
    static FilterFunction
        m_filterFunc; /**< The filter function for path filtering. */
};
}  // namespace Lithium

/*
// Example filter function: exclude files with ".txt" extension
bool filterFunc(const std::filesystem::path &path)
{
    return path.extension() != ".txt";
}

// Example check function: check if "example.txt" file exists in subdir
bool checkFunc(const std::filesystem::path &path)
{
    return AddonFinder::hasFile(path, "example.txt");
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <path>" << std::endl;
        return -1;
    }

    std::filesystem::path path(argv[1]);
    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path))
    {
        std::cerr << "Invalid path: " << argv[1] << std::endl;
        return -1;
    }

    AddonFinder finder(path, filterFunc);
    finder.print();

    // Check if "example.txt" file exists in subdirs
    AddonFinder exampleFinder(path, checkFunc);
    if (exampleFinder.hasFile(path, "package.json"))
    {
        std::cout << "Found 'example.txt' in subdirs." << std::endl;
    }
    else
    {
        std::cout << "Did not find 'example.txt' in subdirs." << std::endl;
    }

    return 0;
}

*/
