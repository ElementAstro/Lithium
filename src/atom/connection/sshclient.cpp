/*
 * sshclient.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: SSH Client

*************************************************/

#include "sshclient.hpp"

#include <filesystem>

#include "atom/error/exception.hpp"

namespace fs = std::filesystem;

namespace atom::connection {
SSHClient::SSHClient(const std::string &host, int port)
    : host_(host), port_(port), ssh_session_(nullptr), sftp_session_(nullptr) {}

SSHClient::~SSHClient() {
    if (sftp_session_) {
        sftp_free(sftp_session_);
    }
    if (ssh_session_) {
        ssh_disconnect(ssh_session_);
        ssh_free(ssh_session_);
    }
}

void SSHClient::connect(const std::string &username,
                        const std::string &password, int timeout) {
    ssh_session_ = ssh_new();
    if (!ssh_session_) {
        THROW_RUNTIME_ERROR("Failed to create SSH session.");
    }

    ssh_options_set(ssh_session_, SSH_OPTIONS_HOST, host_.c_str());
    ssh_options_set(ssh_session_, SSH_OPTIONS_PORT, &port_);
    ssh_options_set(ssh_session_, SSH_OPTIONS_USER, username.c_str());
    ssh_options_set(ssh_session_, SSH_OPTIONS_TIMEOUT, &timeout);

    int rc = ssh_connect(ssh_session_);
    if (rc != SSH_OK) {
        THROW_RUNTIME_ERROR("Failed to connect to SSH server: " +
                            std::string(ssh_get_error(ssh_session_)));
    }

    rc = ssh_userauth_password(ssh_session_, nullptr, password.c_str());
    if (rc != SSH_AUTH_SUCCESS) {
        THROW_RUNTIME_ERROR("Failed to authenticate with SSH server: " +
                            std::string(ssh_get_error(ssh_session_)));
    }

    sftp_session_ = sftp_new(ssh_session_);
    if (!sftp_session_) {
        THROW_RUNTIME_ERROR("Failed to create SFTP session.");
    }

    rc = sftp_init(sftp_session_);
    if (rc != SSH_OK) {
        THROW_RUNTIME_ERROR("Failed to initialize SFTP session: " +
                            std::string(ssh_get_error(ssh_session_)));
    }
}

bool SSHClient::isConnected() const {
    return (ssh_session_ != nullptr && sftp_session_ != nullptr);
}

void SSHClient::disconnect() {
    if (sftp_session_) {
        sftp_free(sftp_session_);
        sftp_session_ = nullptr;
    }
    if (ssh_session_) {
        ssh_disconnect(ssh_session_);
        ssh_free(ssh_session_);
        ssh_session_ = nullptr;
    }
}

void SSHClient::executeCommand(const std::string &command,
                               std::vector<std::string> &output) {
    ssh_channel channel = ssh_channel_new(ssh_session_);
    if (!channel) {
        THROW_RUNTIME_ERROR("Failed to create SSH channel.");
    }

    int rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        ssh_channel_free(channel);
        THROW_RUNTIME_ERROR("Failed to open SSH channel: " +
                            std::string(ssh_get_error(ssh_session_)));
    }

    rc = ssh_channel_request_exec(channel, command.c_str());
    if (rc != SSH_OK) {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        THROW_RUNTIME_ERROR("Failed to execute command: " +
                            std::string(ssh_get_error(ssh_session_)));
    }

    char buffer[256];
    int nbytes = 0;
    while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0)) >
           0) {
        output.emplace_back(buffer, nbytes);
    }

    if (nbytes < 0) {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        THROW_RUNTIME_ERROR("Failed to read command output: " +
                            std::string(ssh_get_error(ssh_session_)));
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
}

void SSHClient::executeCommands(const std::vector<std::string> &commands,
                                std::vector<std::vector<std::string>> &output) {
    ssh_channel channel = ssh_channel_new(ssh_session_);
    if (!channel) {
        THROW_RUNTIME_ERROR("Failed to create SSH channel.");
    }

    int rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        ssh_channel_free(channel);
        THROW_RUNTIME_ERROR("Failed to open SSH channel: " +
                            std::string(ssh_get_error(ssh_session_)));
    }

    for (const auto &cmd : commands) {
        rc = ssh_channel_request_exec(channel, cmd.c_str());
        if (rc != SSH_OK) {
            ssh_channel_close(channel);
            ssh_channel_free(channel);
            THROW_RUNTIME_ERROR("Failed to execute command: " +
                                std::string(ssh_get_error(ssh_session_)));
        }

        std::vector<std::string> cmd_output;
        char buffer[256];
        int nbytes = 0;
        while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0)) >
               0) {
            cmd_output.emplace_back(buffer, nbytes);
        }

        if (nbytes < 0) {
            ssh_channel_close(channel);
            ssh_channel_free(channel);
            THROW_RUNTIME_ERROR("Failed to read command output: " +
                                std::string(ssh_get_error(ssh_session_)));
        }

        ssh_channel_send_eof(channel);
        output.push_back(std::move(cmd_output));
    }

    ssh_channel_close(channel);
    ssh_channel_free(channel);
}

bool SSHClient::fileExists(const std::string &remote_path) const {
    sftp_attributes attrs = sftp_stat(sftp_session_, remote_path.c_str());
    if (attrs) {
        sftp_attributes_free(attrs);
        return true;
    } else {
        return false;
    }
}

void SSHClient::createDirectory(const std::string &remote_path, int mode) {
    int rc = sftp_mkdir(sftp_session_, remote_path.c_str(), mode);
    if (rc != SSH_OK) {
        THROW_RUNTIME_ERROR("Failed to create remote directory: " +
                            remote_path);
    }
}

void SSHClient::removeFile(const std::string &remote_path) {
    int rc = sftp_unlink(sftp_session_, remote_path.c_str());
    if (rc != SSH_OK) {
        THROW_RUNTIME_ERROR("Failed to remove remote file: " + remote_path);
    }
}

void SSHClient::removeDirectory(const std::string &remote_path) {
    int rc = sftp_rmdir(sftp_session_, remote_path.c_str());
    if (rc != SSH_OK) {
        THROW_RUNTIME_ERROR("Failed to remove remote directory: " +
                            remote_path);
    }
}

std::vector<std::string> SSHClient::listDirectory(
    const std::string &remote_path) const {
    std::vector<std::string> file_list;
    sftp_dir dir = sftp_opendir(sftp_session_, remote_path.c_str());
    if (dir) {
        sftp_attributes attributes;
        while ((attributes = sftp_readdir(sftp_session_, dir)) != NULL) {
            file_list.push_back(attributes->name);
            sftp_attributes_free(attributes);
        }
        sftp_closedir(dir);
    }
    return file_list;
}

void SSHClient::rename(const std::string &old_path,
                       const std::string &new_path) {
    int rc = sftp_rename(sftp_session_, old_path.c_str(), new_path.c_str());
    if (rc != SSH_OK) {
        THROW_RUNTIME_ERROR("Failed to rename remote file or directory: " +
                            old_path + " to " + new_path);
    }
}

void SSHClient::getFileInfo(const std::string &remote_path,
                            sftp_attributes &attrs) {
    attrs = sftp_stat(sftp_session_, remote_path.c_str());
    if (!attrs) {
        THROW_RUNTIME_ERROR("Failed to get file info for remote path: " +
                            remote_path);
    }
}

void SSHClient::downloadFile(const std::string &remote_path,
                             const std::string &local_path) {
    sftp_file file =
        sftp_open(sftp_session_, remote_path.c_str(), OFN_READONLY, 0);
    if (!file) {
        THROW_RUNTIME_ERROR("Failed to open remote file for download: " +
                            remote_path);
    }

    FILE *fp = fopen(local_path.c_str(), "wb");
    if (!fp) {
        sftp_close(file);
        THROW_RUNTIME_ERROR("Failed to open local file for download: " +
                            local_path);
    }

    char buffer[256];
    int nbytes = 0;
    while ((nbytes = sftp_read(file, buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, 1, nbytes, fp);
    }

    fclose(fp);
    sftp_close(file);
}

void SSHClient::uploadFile(const std::string &local_path,
                           const std::string &remote_path) {
    sftp_file file =
        sftp_open(sftp_session_, remote_path.c_str(), OF_CREATE, OF_WRITE);
    if (!file) {
        THROW_RUNTIME_ERROR("Failed to open remote file for upload: " +
                            remote_path);
    }

    FILE *fp = fopen(local_path.c_str(), "rb");
    if (!fp) {
        sftp_close(file);
        THROW_RUNTIME_ERROR("Failed to open local file for upload: " +
                            local_path);
    }

    char buffer[256];
    int nbytes = 0;
    while ((nbytes = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        sftp_write(file, buffer, nbytes);
    }

    fclose(fp);
    sftp_close(file);
}

void SSHClient::uploadDirectory(const std::string &local_path,
                                const std::string &remote_path) {
    for (const auto &entry : fs::recursive_directory_iterator(local_path)) {
        const auto &path = entry.path();
        auto relativePath = fs::relative(path, local_path);
        auto remoteFilePath = remote_path + "/" + relativePath.string();

        if (entry.is_directory()) {
            createDirectory(remoteFilePath);
        } else if (entry.is_regular_file()) {
            uploadFile(path.string(), remoteFilePath);
        }
    }
}
}  // namespace atom::connection
