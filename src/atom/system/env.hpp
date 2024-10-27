/*
 * env.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-16

Description: Environment variable management

**************************************************/

#ifndef ATOM_UTILS_ENV_HPP
#define ATOM_UTILS_ENV_HPP

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "atom/macro.hpp"

namespace atom::utils {
/**
 * @brief Environment variable class for managing program environment variables,
 * command-line arguments, and other related information.
 */
class Env {
public:
    /**
     * @brief Default constructor that initializes environment variable
     * information.
     */
    Env();

    /**
     * @brief Constructor that initializes environment variable information with
     * command-line arguments.
     * @param argc Number of command-line arguments.
     * @param argv Array of command-line arguments.
     */
    explicit Env(int argc, char** argv);

    /**
     * @brief Static method to create a shared pointer to an Env object.
     * @param argc Number of command-line arguments.
     * @param argv Array of command-line arguments.
     * @return Shared pointer to an Env object.
     */
    static auto createShared(int argc, char** argv) -> std::shared_ptr<Env>;

    /**
     * @brief Static method to get the current environment variables.
     * @return Unordered map of environment variables.
     */
    static auto Environ() -> std::unordered_map<std::string, std::string>;

    /**
     * @brief Adds a key-value pair to the environment variables.
     * @param key The key name.
     * @param val The value associated with the key.
     */
    void add(const std::string& key, const std::string& val);

    /**
     * @brief Checks if a key exists in the environment variables.
     * @param key The key name.
     * @return True if the key exists, otherwise false.
     */
    bool has(const std::string& key);

    /**
     * @brief Deletes a key-value pair from the environment variables.
     * @param key The key name.
     */
    void del(const std::string& key);

    /**
     * @brief Gets the value associated with a key, or returns a default value
     * if the key does not exist.
     * @param key The key name.
     * @param default_value The default value to return if the key does not
     * exist.
     * @return The value associated with the key, or the default value.
     */
    ATOM_NODISCARD auto get(const std::string& key,
                            const std::string& default_value = "")
        -> std::string;

    /**
     * @brief Adds a command-line argument and its description to the help
     * information list.
     * @param key The argument name.
     * @param desc The argument description.
     */
    void addHelp(const std::string& key, const std::string& desc);

    /**
     * @brief Removes a command-line argument from the help information list.
     * @param key The argument name.
     */
    void removeHelp(const std::string& key);

    /**
     * @brief Prints the program's help information, including all added
     * command-line arguments and their descriptions.
     */
    void printHelp();

    /**
     * @brief Sets the value of an environment variable.
     * @param key The key name.
     * @param val The value to set.
     * @return True if the environment variable was set successfully, otherwise
     * false.
     */
    auto setEnv(const std::string& key, const std::string& val) -> bool;

    /**
     * @brief Gets the value of an environment variable, or returns a default
     * value if the variable does not exist.
     * @param key The key name.
     * @param default_value The default value to return if the variable does not
     * exist.
     * @return The value of the environment variable, or the default value.
     */
    ATOM_NODISCARD auto getEnv(const std::string& key,
                               const std::string& default_value = "")
        -> std::string;

    /**
     * @brief Gets the absolute path of a given path.
     * @param path The path to convert to an absolute path.
     * @return The absolute path.
     */
    ATOM_NODISCARD auto getAbsolutePath(const std::string& path) const
        -> std::string;

    /**
     * @brief Gets the absolute path of a given path relative to the working
     * directory.
     * @param path The path to convert to an absolute path relative to the
     * working directory.
     * @return The absolute path.
     */
    ATOM_NODISCARD auto getAbsoluteWorkPath(const std::string& path) const
        -> std::string;

    /**
     * @brief Gets the path of the configuration file. By default, the
     * configuration file is in the same directory as the program.
     * @return The configuration file path.
     */
    ATOM_NODISCARD auto getConfigPath() -> std::string;

    /**
     * @brief Sets an environment variable.
     * @param name The variable name.
     * @param value The variable value.
     * @param overwrite Whether to overwrite the variable if it already exists.
     */
    static void setVariable(const std::string& name, const std::string& value,
                            bool overwrite = true);

    /**
     * @brief Gets the value of an environment variable.
     * @param name The variable name.
     * @return The variable value.
     */
    static auto getVariable(const std::string& name) -> std::string;

    /**
     * @brief Unsets (deletes) an environment variable.
     * @param name The variable name.
     */
    static void unsetVariable(const std::string& name);

    /**
     * @brief Lists all environment variables.
     * @return A vector of environment variable names.
     */
    static auto listVariables() -> std::vector<std::string>;

    /**
     * @brief Prints all environment variables.
     */
    static void printAllVariables();

private:
    std::string m_exe;      ///< Full path of the executable file.
    std::string m_cwd;      ///< Working directory.
    std::string m_program;  ///< Program name.

    std::unordered_map<std::string, std::string>
        m_args;  ///< List of command-line arguments.
    std::vector<std::pair<std::string, std::string>>
        m_helps;                 ///< List of help information.
    mutable std::mutex m_mutex;  ///< Mutex to protect member variables.
};

}  // namespace atom::utils

#endif
