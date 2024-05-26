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

namespace atom::connection {
/**
 * @class SshServer
 * @brief Represents an SSH server for handling secure shell connections.
 */
class SshServer {
public:
    /**
     * @brief Constructor.
     * @param configFile The path to the configuration file for the SSH server.
     */
    SshServer(const std::filesystem::path& configFile);

    /**
     * @brief Destructor.
     */
    ~SshServer();

    SshServer(const SshServer&) =
        delete; /**< Deleted copy constructor to prevent copying. */
    SshServer& operator=(const SshServer&) =
        delete; /**< Deleted copy assignment operator to prevent copying. */

    void start(); /**< Starts the SSH server. */
    void stop();  /**< Stops the SSH server. */
    bool isRunning()
        const; /**< Checks if the SSH server is currently running. */

    void setPort(int port); /**< Sets the port on which the SSH server listens
                               for connections. */
    int getPort()
        const; /**< Gets the port on which the SSH server is listening. */

    void setListenAddress(
        const std::string& address); /**< Sets the address on which the SSH
                                        server listens for connections. */
    std::string getListenAddress()
        const; /**< Gets the address on which the SSH server is listening. */

    void setHostKey(
        const std::filesystem::path&
            keyFile); /**< Sets the host key file used for SSH connections. */
    std::filesystem::path getHostKey()
        const; /**< Gets the path to the host key file. */

    void setAuthorizedKeys(
        const std::vector<std::filesystem::path>&
            keyFiles); /**< Sets the list of authorized public key files for
                          user authentication. */
    std::vector<std::filesystem::path> getAuthorizedKeys()
        const; /**< Gets the list of authorized public key files. */

    void allowRootLogin(
        bool allow); /**< Enables or disables root login to the SSH server. */
    bool isRootLoginAllowed() const; /**< Checks if root login is allowed. */

    void setPasswordAuthentication(
        bool enable); /**< Enables or disables password authentication for the
                         SSH server. */
    bool isPasswordAuthenticationEnabled()
        const; /**< Checks if password authentication is enabled. */

    void setSubsystem(
        const std::string& name,
        const std::string&
            command); /**< Sets a subsystem for handling a specific command. */
    void removeSubsystem(const std::string& name); /**< Removes a previously set
                                                      subsystem by name. */
    std::string getSubsystem(const std::string& name)
        const; /**< Gets the command associated with a subsystem by name. */

private:
    class Impl; /**< Forward declaration of the implementation class. */
    std::unique_ptr<Impl> impl_; /**< Pointer to the implementation object. */
};
}  // namespace atom::connection

#endif  // ATOM_CONNECTION_SSHSERVER_HPP
