/*
 * sshclient.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: SSH客户端连接和文件操作类。

*************************************************/

#ifndef ATOM_CONNECTION_SSHCLIENT_HPP
#define ATOM_CONNECTION_SSHCLIENT_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>


#include <libssh/libssh.h>
#include <libssh/sftp.h>

namespace atom::connection {
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
    explicit SSHClient(const std::string &host, int port = 22);

    /**
     * @brief Destructor.
     */
    ~SSHClient();

    /**
     * @brief Connects to the SSH server.
     * @param username The username for authentication.
     * @param password The password for authentication.
     * @param timeout The connection timeout in seconds. Default is 10 seconds.
     * @throws std::runtime_error if connection or authentication fails.
     */
    void Connect(const std::string &username, const std::string &password,
                 int timeout = 10);

    /**
     * @brief Checks if the SSH client is connected to the server.
     * @return true if connected, false otherwise.
     */
    bool IsConnected();

    /**
     * @brief Disconnects from the SSH server.
     */
    void Disconnect();

    /**
     * @brief Executes a single command on the SSH server.
     * @param command The command to execute.
     * @param output Output vector to store the command output.
     * @throws std::runtime_error if command execution fails.
     */
    void ExecuteCommand(const std::string &command,
                        std::vector<std::string> &output);

    /**
     * @brief Executes multiple commands on the SSH server.
     * @param commands Vector of commands to execute.
     * @param output Vector of vectors to store the command outputs.
     * @throws std::runtime_error if any command execution fails.
     */
    void ExecuteCommands(const std::vector<std::string> &commands,
                         std::vector<std::vector<std::string>> &output);

    /**
     * @brief Checks if a file exists on the remote server.
     * @param remote_path The path of the remote file.
     * @return true if the file exists, false otherwise.
     */
    bool FileExists(const std::string &remote_path);

    /**
     * @brief Creates a directory on the remote server.
     * @param remote_path The path of the remote directory.
     * @param mode The permissions of the directory. Default is S_NORMAL.
     * @throws std::runtime_error if directory creation fails.
     */
    void CreateDirectory(const std::string &remote_path, int mode = S_NORMAL);

    /**
     * @brief Removes a file from the remote server.
     * @param remote_path The path of the remote file.
     * @throws std::runtime_error if file removal fails.
     */
    void RemoveFile(const std::string &remote_path);

    /**
     * @brief Removes a directory from the remote server.
     * @param remote_path The path of the remote directory.
     * @throws std::runtime_error if directory removal fails.
     */
    void RemoveDirectory(const std::string &remote_path);

    /**
     * @brief Lists the contents of a directory on the remote server.
     * @param remote_path The path of the remote directory.
     * @return Vector of strings containing the names of the directory contents.
     * @throws std::runtime_error if listing directory fails.
     */
    std::vector<std::string> ListDirectory(const std::string &remote_path);

    /**
     * @brief Renames a file or directory on the remote server.
     * @param old_path The current path of the remote file or directory.
     * @param new_path The new path of the remote file or directory.
     * @throws std::runtime_error if renaming fails.
     */
    void Rename(const std::string &old_path, const std::string &new_path);

    /**
     * @brief Retrieves file information for a remote file.
     * @param remote_path The path of the remote file.
     * @param attrs Attribute struct to store the file information.
     * @throws std::runtime_error if getting file information fails.
     */
    void GetFileInfo(const std::string &remote_path, sftp_attributes &attrs);

    /**
     * @brief Downloads a file from the remote server.
     * @param remote_path The path of the remote file.
     * @param local_path The path of the local destination file.
     * @throws std::runtime_error if file download fails.
     */
    void DownloadFile(const std::string &remote_path,
                      const std::string &local_path);

    /**
     * @brief Uploads a file to the remote server.
     * @param local_path The path of the local source file.
     * @param remote_path The path of the remote destination file.
     * @throws std::runtime_error if file upload fails.
     */
    void UploadFile(const std::string &local_path,
                    const std::string &remote_path);

private:
    std::string m_host;
    int m_port;
    ssh_session m_ssh_session;
    sftp_session m_sftp_session;
};
}  // namespace atom::connection

#endif