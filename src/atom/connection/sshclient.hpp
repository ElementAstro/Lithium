/*
 * sshclient.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: SSH Client

*************************************************/

#ifndef ATOM_CONNECTION_SSHCLIENT_HPP
#define ATOM_CONNECTION_SSHCLIENT_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#if __has_include(<libssh/libssh.h>)
#include <libssh/libssh.h>
#include <libssh/sftp.h>

namespace atom::connection {

constexpr int DEFAULT_SSH_PORT = 22;
constexpr int DEFAULT_TIMEOUT = 10;
constexpr int DEFAULT_MODE = S_NORMAL;

/**
 * @class SSHClient
 * @brief A class for SSH client connection and file operations.
 */
class SSHClient {
public:
    /**
     * @brief Constructor.
     * @param host The hostname or IP address of the SSH server.
     * @param port The port number of the SSH server. Default is 22.
     */
    explicit SSHClient(const std::string &host, int port = DEFAULT_SSH_PORT);

    /**
     * @brief Destructor.
     */
    ~SSHClient();

    // Copy constructor
    SSHClient(const SSHClient &other) = default;

    // Copy assignment operator
    auto operator=(const SSHClient &other) -> SSHClient & = default;

    // Move constructor
    SSHClient(SSHClient &&other) noexcept = default;

    // Move assignment operator
    auto operator=(SSHClient &&other) noexcept -> SSHClient & = default;

    /**
     * @brief Connects to the SSH server.
     * @param username The username for authentication.
     * @param password The password for authentication.
     * @param timeout The connection timeout in seconds. Default is 10 seconds.
     * @throws std::runtime_error if connection or authentication fails.
     */
    void connect(const std::string &username, const std::string &password,
                 int timeout = DEFAULT_TIMEOUT);

    /**
     * @brief Checks if the SSH client is connected to the server.
     * @return true if connected, false otherwise.
     */
    [[nodiscard]] auto isConnected() const -> bool;

    /**
     * @brief Disconnects from the SSH server.
     */
    void disconnect();

    /**
     * @brief Executes a single command on the SSH server.
     * @param command The command to execute.
     * @param output Output vector to store the command output.
     * @throws std::runtime_error if command execution fails.
     */
    void executeCommand(const std::string &command,
                        std::vector<std::string> &output);

    /**
     * @brief Executes multiple commands on the SSH server.
     * @param commands Vector of commands to execute.
     * @param output Vector of vectors to store the command outputs.
     * @throws std::runtime_error if any command execution fails.
     */
    void executeCommands(const std::vector<std::string> &commands,
                         std::vector<std::vector<std::string>> &output);

    /**
     * @brief Checks if a file exists on the remote server.
     * @param remote_path The path of the remote file.
     * @return true if the file exists, false otherwise.
     */
    [[nodiscard]] auto fileExists(const std::string &remote_path) const -> bool;

    /**
     * @brief Creates a directory on the remote server.
     * @param remote_path The path of the remote directory.
     * @param mode The permissions of the directory. Default is S_NORMAL.
     * @throws std::runtime_error if directory creation fails.
     */
    void createDirectory(const std::string &remote_path,
                         int mode = DEFAULT_MODE);

    /**
     * @brief Removes a file from the remote server.
     * @param remote_path The path of the remote file.
     * @throws std::runtime_error if file removal fails.
     */
    void removeFile(const std::string &remote_path);

    /**
     * @brief Removes a directory from the remote server.
     * @param remote_path The path of the remote directory.
     * @throws std::runtime_error if directory removal fails.
     */
    void removeDirectory(const std::string &remote_path);

    /**
     * @brief Lists the contents of a directory on the remote server.
     * @param remote_path The path of the remote directory.
     * @return Vector of strings containing the names of the directory contents.
     * @throws std::runtime_error if listing directory fails.
     */
    auto listDirectory(const std::string &remote_path) const
        -> std::vector<std::string>;

    /**
     * @brief Renames a file or directory on the remote server.
     * @param old_path The current path of the remote file or directory.
     * @param new_path The new path of the remote file or directory.
     * @throws std::runtime_error if renaming fails.
     */
    void rename(const std::string &old_path, const std::string &new_path);

    /**
     * @brief Retrieves file information for a remote file.
     * @param remote_path The path of the remote file.
     * @param attrs Attribute struct to store the file information.
     * @throws std::runtime_error if getting file information fails.
     */
    void getFileInfo(const std::string &remote_path, sftp_attributes &attrs);

    /**
     * @brief Downloads a file from the remote server.
     * @param remote_path The path of the remote file.
     * @param local_path The path of the local destination file.
     * @throws std::runtime_error if file download fails.
     */
    void downloadFile(const std::string &remote_path,
                      const std::string &local_path);

    /**
     * @brief Uploads a file to the remote server.
     * @param local_path The path of the local source file.
     * @param remote_path The path of the remote destination file.
     * @throws std::runtime_error if file upload fails.
     */
    void uploadFile(const std::string &local_path,
                    const std::string &remote_path);

    /**
     * @brief Uploads a directory to the remote server.
     * @param local_path The path of the local source directory.
     * @param remote_path The path of the remote destination directory.
     * @throws std::runtime_error if directory upload fails.
     */
    void uploadDirectory(const std::string &local_path,
                         const std::string &remote_path);

private:
    std::string host_;
    int port_;
    ssh_session ssh_session_;
    sftp_session sftp_session_;
};
}  // namespace atom::connection
#endif

#endif