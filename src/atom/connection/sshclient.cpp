/*
 * sshclient.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: SSH客户端连接和文件操作类。

*************************************************/

#include "sshclient.hpp"

namespace Atom::Connection {
SSHClient::SSHClient(const std::string &host, int port = 22)
    : m_host(host),
      m_port(port),
      m_ssh_session(nullptr),
      m_sftp_session(nullptr) {}

SSHClient::~SSHClient() {
    if (m_sftp_session) {
        sftp_free(m_sftp_session);
    }
    if (m_ssh_session) {
        ssh_disconnect(m_ssh_session);
        ssh_free(m_ssh_session);
    }
}

void SSHClient::Connect(const std::string &username,
                        const std::string &password, int timeout = 10) {
    m_ssh_session = ssh_new();
    if (!m_ssh_session) {
        throw std::runtime_error("Failed to create SSH session.");
    }

    ssh_options_set(m_ssh_session, SSH_OPTIONS_HOST, m_host.c_str());
    ssh_options_set(m_ssh_session, SSH_OPTIONS_PORT, &m_port);
    ssh_options_set(m_ssh_session, SSH_OPTIONS_USER, username.c_str());
    ssh_options_set(m_ssh_session, SSH_OPTIONS_TIMEOUT, &timeout);

    int rc = ssh_connect(m_ssh_session);
    if (rc != SSH_OK) {
        throw std::runtime_error("Failed to connect to SSH server: " +
                                 std::string(ssh_get_error(m_ssh_session)));
    }

    rc = ssh_userauth_password(m_ssh_session, nullptr, password.c_str());
    if (rc != SSH_AUTH_SUCCESS) {
        throw std::runtime_error("Failed to authenticate with SSH server: " +
                                 std::string(ssh_get_error(m_ssh_session)));
    }

    m_sftp_session = sftp_new(m_ssh_session);
    if (!m_sftp_session) {
        throw std::runtime_error("Failed to create SFTP session.");
    }

    rc = sftp_init(m_sftp_session);
    if (rc != SSH_OK) {
        throw std::runtime_error("Failed to initialize SFTP session: " +
                                 std::string(ssh_get_error(m_ssh_session)));
    }
}

bool SSHClient::IsConnected() {
    return (m_ssh_session != nullptr && m_sftp_session != nullptr);
}

void SSHClient::Disconnect() {
    if (m_sftp_session) {
        sftp_free(m_sftp_session);
        m_sftp_session = nullptr;
    }
    if (m_ssh_session) {
        ssh_disconnect(m_ssh_session);
        ssh_free(m_ssh_session);
        m_ssh_session = nullptr;
    }
}

void SSHClient::ExecuteCommand(const std::string &command,
                               std::vector<std::string> &output) {
    ssh_channel channel = ssh_channel_new(m_ssh_session);
    if (!channel) {
        throw std::runtime_error("Failed to create SSH channel.");
    }

    int rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        ssh_channel_free(channel);
        throw std::runtime_error("Failed to open SSH channel: " +
                                 std::string(ssh_get_error(m_ssh_session)));
    }

    rc = ssh_channel_request_exec(channel, command.c_str());
    if (rc != SSH_OK) {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        throw std::runtime_error("Failed to execute command: " +
                                 std::string(ssh_get_error(m_ssh_session)));
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
        throw std::runtime_error("Failed to read command output: " +
                                 std::string(ssh_get_error(m_ssh_session)));
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
}

void SSHClient::ExecuteCommands(const std::vector<std::string> &commands,
                                std::vector<std::vector<std::string>> &output) {
    ssh_channel channel = ssh_channel_new(m_ssh_session);
    if (!channel) {
        throw std::runtime_error("Failed to create SSH channel.");
    }

    int rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        ssh_channel_free(channel);
        throw std::runtime_error("Failed to open SSH channel: " +
                                 std::string(ssh_get_error(m_ssh_session)));
    }

    for (const auto &cmd : commands) {
        rc = ssh_channel_request_exec(channel, cmd.c_str());
        if (rc != SSH_OK) {
            ssh_channel_close(channel);
            ssh_channel_free(channel);
            throw std::runtime_error("Failed to execute command: " +
                                     std::string(ssh_get_error(m_ssh_session)));
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
            throw std::runtime_error("Failed to read command output: " +
                                     std::string(ssh_get_error(m_ssh_session)));
        }

        ssh_channel_send_eof(channel);
        output.push_back(std::move(cmd_output));
    }

    ssh_channel_close(channel);
    ssh_channel_free(channel);
}

bool SSHClient::FileExists(const std::string &remote_path) {
    sftp_attributes attrs = sftp_stat(m_sftp_session, remote_path.c_str());
    if (attrs) {
        sftp_attributes_free(attrs);
        return true;
    } else {
        return false;
    }
}

void SSHClient::CreateDirectory(const std::string &remote_path,
                                int mode = S_NORMAL) {
    int rc = sftp_mkdir(m_sftp_session, remote_path.c_str(), mode);
    if (rc != SSH_OK) {
        throw std::runtime_error("Failed to create remote directory: " +
                                 remote_path);
    }
}

void SSHClient::RemoveFile(const std::string &remote_path) {
    int rc = sftp_unlink(m_sftp_session, remote_path.c_str());
    if (rc != SSH_OK) {
        throw std::runtime_error("Failed to remove remote file: " +
                                 remote_path);
    }
}

void SSHClient::RemoveDirectory(const std::string &remote_path) {
    int rc = sftp_rmdir(m_sftp_session, remote_path.c_str());
    if (rc != SSH_OK) {
        throw std::runtime_error("Failed to remove remote directory: " +
                                 remote_path);
    }
}

std::vector<std::string> SSHClient::ListDirectory(
    const std::string &remote_path) {
    std::vector<std::string> file_list;
    sftp_dir dir = sftp_opendir(m_sftp_session, remote_path.c_str());
    if (dir) {
        sftp_attributes attributes;
        while ((attributes = sftp_readdir(m_sftp_session, dir)) != NULL) {
            file_list.push_back(attributes->name);
            sftp_attributes_free(attributes);
        }
        sftp_closedir(dir);
    }
    return file_list;
}

void SSHClient::Rename(const std::string &old_path,
                       const std::string &new_path) {
    int rc = sftp_rename(m_sftp_session, old_path.c_str(), new_path.c_str());
    if (rc != SSH_OK) {
        throw std::runtime_error("Failed to rename remote file or directory: " +
                                 old_path + " to " + new_path);
    }
}

void SSHClient::GetFileInfo(const std::string &remote_path,
                            sftp_attributes &attrs) {
    attrs = sftp_stat(m_sftp_session, remote_path.c_str());
    if (!attrs) {
        throw std::runtime_error("Failed to get file info for remote path: " +
                                 remote_path);
    }
}

void SSHClient::DownloadFile(const std::string &remote_path,
                             const std::string &local_path) {
    sftp_file file =
        sftp_open(m_sftp_session, remote_path.c_str(), OFN_READONLY, 0);
    if (!file) {
        throw std::runtime_error("Failed to open remote file for download: " +
                                 remote_path);
    }

    FILE *fp = fopen(local_path.c_str(), "wb");
    if (!fp) {
        sftp_close(file);
        throw std::runtime_error("Failed to open local file for download: " +
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

void SSHClient::UploadFile(const std::string &local_path,
                           const std::string &remote_path) {
    sftp_file file =
        sftp_open(m_sftp_session, remote_path.c_str(), OF_CREATE, OF_WRITE);
    if (!file) {
        throw std::runtime_error("Failed to open remote file for upload: " +
                                 remote_path);
    }

    FILE *fp = fopen(local_path.c_str(), "rb");
    if (!fp) {
        sftp_close(file);
        throw std::runtime_error("Failed to open local file for upload: " +
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
}  // namespace Atom::Connection
