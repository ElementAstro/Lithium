/*
 * sshserver.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-5-24

Description: SSH Server

*************************************************/

#ifndef ATOM_CONNECTION_SSHSERVER_HPP
#define ATOM_CONNECTION_SSHSERVER_HPP

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "atom/type/noncopyable.hpp"

#include "macro.hpp"

namespace atom::connection {
/**
 * @class SshServer
 * @brief Represents an SSH server for handling secure shell connections.
 *
 * This class provides methods to configure and manage an SSH server, handling
 * client connections and user authentication through various methods including
 * public key and password authentication.
 */
class SshServer : public NonCopyable {
public:
    /**
     * @brief Constructor for SshServer.
     *
     * Initializes the SSH server with a specified configuration file.
     *
     * @param configFile The path to the configuration file for the SSH server.
     */
    explicit SshServer(const std::filesystem::path& configFile);

    /**
     * @brief Destructor for SshServer.
     *
     * Cleans up resources used by the SSH server.
     */
    ~SshServer() override;

    /**
     * @brief Starts the SSH server.
     *
     * This method will begin listening for incoming connections on the
     * configured port and address.
     */
    void start();

    /**
     * @brief Stops the SSH server.
     *
     * This method will stop the server from accepting new connections and
     * cleanly shut down any existing connections.
     */
    void stop();

    /**
     * @brief Checks if the SSH server is currently running.
     *
     * @return true if the server is running, false otherwise.
     */
    ATOM_NODISCARD auto isRunning() const -> bool;

    /**
     * @brief Sets the port on which the SSH server listens for connections.
     *
     * @param port The port number to listen on.
     *
     * This method updates the server's listening port to the specified value.
     */
    void setPort(int port);

    /**
     * @brief Gets the port on which the SSH server is listening.
     *
     * @return The current listening port.
     */
    ATOM_NODISCARD auto getPort() const -> int;

    /**
     * @brief Sets the address on which the SSH server listens for connections.
     *
     * @param address The IP address or hostname for listening.
     *
     * The server will bind to this address, allowing connections from it.
     */
    void setListenAddress(const std::string& address);

    /**
     * @brief Gets the address on which the SSH server is listening.
     *
     * @return The current listening address as a string.
     */
    ATOM_NODISCARD auto getListenAddress() const -> std::string;

    /**
     * @brief Sets the host key file used for SSH connections.
     *
     * @param keyFile The path to the host key file.
     *
     * The host key is used to establish the identity of the server,
     * enabling secure communication with clients.
     */
    void setHostKey(const std::filesystem::path& keyFile);

    /**
     * @brief Gets the path to the host key file.
     *
     * @return The current host key file path.
     */
    ATOM_NODISCARD auto getHostKey() const -> std::filesystem::path;

    /**
     * @brief Sets the list of authorized public key files for user
     * authentication.
     *
     * @param keyFiles A vector of paths to public key files.
     *
     * This method updates the SSH server to allow authentication using the
     * specified public keys.
     */
    void setAuthorizedKeys(const std::vector<std::filesystem::path>& keyFiles);

    /**
     * @brief Gets the list of authorized public key files.
     *
     * @return A vector of paths to authorized public key files.
     */
    ATOM_NODISCARD auto getAuthorizedKeys() const
        -> std::vector<std::filesystem::path>;

    /**
     * @brief Enables or disables root login to the SSH server.
     *
     * @param allow true to permit root login, false to deny it.
     *
     * This method must be configured with caution, as enabling root login
     * can pose a security risk.
     */
    void allowRootLogin(bool allow);

    /**
     * @brief Checks if root login is allowed.
     *
     * @return true if root login is permitted, false otherwise.
     */
    ATOM_NODISCARD auto isRootLoginAllowed() const -> bool;

    /**
     * @brief Enables or disables password authentication for the SSH server.
     *
     * @param enable true to enable password authentication, false to disable
     * it.
     */
    void setPasswordAuthentication(bool enable);

    /**
     * @brief Checks if password authentication is enabled.
     *
     * @return true if password authentication is enabled, false otherwise.
     */
    ATOM_NODISCARD auto isPasswordAuthenticationEnabled() const -> bool;

    /**
     * @brief Sets a subsystem for handling a specific command.
     *
     * @param name The name of the subsystem.
     * @param command The command that the subsystem will execute.
     *
     * This allows for additional functionality to be added to the SSH server,
     * such as file transfers or other custom commands.
     */
    void setSubsystem(const std::string& name, const std::string& command);

    /**
     * @brief Removes a previously set subsystem by name.
     *
     * @param name The name of the subsystem to remove.
     *
     * After this method is called, the subsystem will no longer be available.
     */
    void removeSubsystem(const std::string& name);

    /**
     * @brief Gets the command associated with a subsystem by name.
     *
     * @param name The name of the subsystem.
     * @return The command associated with the subsystem.
     *
     * If the subsystem does not exist, an empty string may be returned.
     */
    ATOM_NODISCARD auto getSubsystem(const std::string& name) const
        -> std::string;

private:
    class Impl;  ///< Forward declaration of the implementation class.
    std::unique_ptr<Impl> impl_;  ///< Pointer to the implementation object
                                  ///< holding the core functionalities.
};

}  // namespace atom::connection

#endif  // ATOM_CONNECTION_SSHSERVER_HPP
