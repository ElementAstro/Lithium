/*
 * sshclient.hpp
 * 
 * Copyright (C) 2023 Max Qian <lightapt.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/************************************************* 
 
Copyright: 2023 Max Qian. All rights reserved
 
Author: Max Qian

E-mail: astro_air@126.com
 
Date: 2023-4-5
 
Description: SSH Client
 
**************************************************/

#include <string>
#include <vector>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <libssh/ssh2.h>


/**
 * @brief The SSHClient class represents a SSH client and provides methods for connecting to a remote host,
 * executing commands on the host, and transferring files to/from the host over SFTP.
 */
class SSHClient {
public:
    /**
     * @brief Construct a new SSHClient object
     * 
     * @param ip The IP address of the remote host
     * @param username The username used for authentication
     * @param password The password used for authentication
     * @param port The port number to connect to, defaults to 22
     */
    SSHClient(const std::string& ip, const std::string& username, const std::string& password, int port = 22);
    /**
     * @brief Destroy the SSHClient object
     * 
     * Disconnect from the remote host and release resources allocated by the SSH session and SFTP session.
     */
    ~SSHClient();
    /**
     * @brief Connect to the remote host
     * 
     * @return true if connected successfully, false otherwise
     */
    bool connect();
    /**
     * @brief Disconnect from the remote host
     * 
     * @return true if disconnected successfully, false otherwise
     */
    bool disconnect();
    /**
     * @brief Execute a command on the remote host
     * 
     * @param command The command to execute
     * @param output The output captured from the command execution
     * @return true if executed successfully, false otherwise
     */
    bool execCommand(const std::string& command, std::string& output);
    /**
     * @brief Upload a local file to the remote host
     * 
     * @param localPath The path of the local file to be uploaded
     * @param remotePath The path of the remote file to be created
     * @return true if uploaded successfully, false otherwise
     */
    bool uploadFile(const std::string& localPath, const std::string& remotePath);
    /**
     * @brief Download a file from the remote host to the local machine
     * 
     * @param remotePath The path of the remote file to be downloaded
     * @param localPath The path of the local file to be created
     * @return true if downloaded successfully, false otherwise
     */
    bool downloadFile(const std::string& remotePath, const std::string& localPath);

    /**
     * @brief Get the SSH connection state
     * 
     * @return true if connected, false otherwise
     */
    bool isConnected() const;

    /**
     * @brief Set the connection parameters
     * 
     * @param ip The IP address of the remote host
     * @param username The username used for authentication
     * @param password The password used for authentication
     * @param port The port number to connect to, defaults to 22
     */
    void setConnectionParams(const std::string& ip, const std::string& username, const std::string& password, int port = 22);

    /**
     * @brief Check if the SFTP session is opened
     * 
     * @return true if the SFTP session is opened, false otherwise
     */
    bool isSftpSessionOpened() const;

    /**
     * @brief Open the SFTP session
     * 
     * @return true if opened successfully, false otherwise
     */
    bool openSftpSession();

    /**
     * @brief Close the SFTP session
     * 
     * @return true if closed successfully, false otherwise
     */
    bool closeSftpSession();

    /**
     * @brief Create a remote directory
     * 
     * @param remoteDirPath The path of the remote directory to create
     * @return true if created successfully, false otherwise
     */
    bool createRemoteDirectory(const std::string& remoteDirPath) const;

    /**
     * @brief Rename the remote file or directory
     * 
     * @param oldPath The path of the remote file or directory to rename
     * @param newPath The new name of the remote file or directory
     * @return true if renamed successfully, false otherwise
     */
    bool renameRemoteFileOrDir(const std::string& oldPath, const std::string& newPath) const;

private:
    std::string m_ip;
    std::string m_username;
    std::string m_password;
    int m_port;
    ssh_session m_session;
    sftp_session m_sftp;
    bool m_connected;
};


#include <spdlog/spdlog.h>
SSHClient::SSHClient(const std::string& ip, const std::string& username, const std::string& password, int port)
    : m_ip(ip), m_username(username), m_password(password), m_port(port), m_connected(false), m_session(nullptr), m_sftp(nullptr) {
}
SSHClient::~SSHClient() {
	try {
		if (m_connected) {
			ssh_disconnect(m_session);
			ssh_free(m_session);
			sftp_free(m_sftp);
		}
	}
	catch (...) {
		spdlog::error("Error occurred while disconnecting from SSH server");
	}
	// 关闭日志
	spdlog::drop_all();
}
bool SSHClient::connect() {
	try {
		// 建立SSH连接
		m_session = ssh_new();
		if (!m_session) {
			spdlog::error("Failed to create SSH session");
			return false;
		}
		if (ssh_options_set(m_session, SSH_OPTIONS_HOST, m_ip.c_str()) != SSH_OK
		            || ssh_options_set(m_session, SSH_OPTIONS_PORT, &m_port) != SSH_OK
		            || ssh_options_set(m_session, SSH_OPTIONS_USER, m_username.c_str()) != SSH_OK) {
			spdlog::error("Failed to set SSH options");
			ssh_free(m_session);
			return false;
		}
		int rc = ssh_connect(m_session);
		if (rc != SSH_OK) {
			spdlog::error("Failed to connect to SSH server: {}", ssh_get_error(m_session));
			ssh_free(m_session);
			return false;
		}
		rc = ssh_userauth_password(m_session, NULL, m_password.c_str());
		if (rc != SSH_AUTH_SUCCESS) {
			spdlog::error("Failed to authenticate: {}", ssh_get_error(m_session));
			ssh_disconnect(m_session);
			ssh_free(m_session);
			return false;
		}
		// 建立SFTP连接
		m_sftp = sftp_new(m_session);
		if (!m_sftp) {
			spdlog::error("Failed to create SFTP session");
			ssh_disconnect(m_session);
			ssh_free(m_session);
			return false;
		}
		rc = sftp_init(m_sftp);
		if (rc != SSH_OK) {
			spdlog::error("Failed to initialize SFTP session: {}", ssh_get_error(m_session));
			sftp_free(m_sftp);
			ssh_disconnect(m_session);
			ssh_free(m_session);
			return false;
		}
		m_connected = true;
		spdlog::info("Connected to SSH server");
		return true;
	}
	catch (...) {
		spdlog::error("Error occurred while connecting to SSH server");
		return false;
	}
}
bool SSHClient::disconnect() {
	try {
		if (m_connected) {
			ssh_disconnect(m_session);
			ssh_free(m_session);
			sftp_free(m_sftp);
			m_connected = false;
			spdlog::info("Disconnected from SSH server");
		}
		// 关闭日志
		spdlog::drop_all();
		return true;
	}
	catch (...) {
		spdlog::error("Error occurred while disconnecting from SSH server");
		return false;
	}
}
bool SSHClient::execCommand(const std::string& command, std::string& output) {
	try {
		if (!m_connected) {
			spdlog::error("Not connected to SSH server");
			return false;
		}
		ssh_channel channel = ssh_channel_new(m_session);
		if (!channel) {
			spdlog::error("Failed to create SSH channel: {}", ssh_get_error(m_session));
			return false;
		}
		int rc = ssh_channel_open_session(channel);
		if (rc != SSH_OK) {
			spdlog::error("Failed to open SSH channel: {}", ssh_get_error(m_session));
			ssh_channel_free(channel);
			return false;
		}
		rc = ssh_channel_request_exec(channel, command.c_str());
		if (rc != SSH_OK) {
			spdlog::error("Failed to execute command '{}': {}", command, ssh_get_error(m_session));
			ssh_channel_close(channel);
			ssh_channel_free(channel);
			return false;
		}
		char buffer[1024];
		int nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
		while (nbytes > 0) {
			output.append(buffer, nbytes);
			nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
		}
		ssh_channel_send_eof(channel);
		ssh_channel_close(channel);
		ssh_channel_free(channel);
		return true;
	}
	catch (...) {
		spdlog::error("Error occurred while executing command on SSH server");
		return false;
	}
}
bool SSHClient::uploadFile(const std::string& localPath, const std::string& remotePath) {
	try {
		if (!m_connected) {
			spdlog::error("Not connected to SSH server");
			return false;
		}
		sftp_file file = sftp_open(m_sftp, remotePath.c_str(), O_WRONLY | O_CREAT, S_IRWXU);
		if (!file) {
			spdlog::error("Failed to open remote file '{}': {}", remotePath, ssh_get_error(m_session));
			return false;
		}
		FILE* fp = fopen(localPath.c_str(), "rb");
		if (!fp) {
			spdlog::error("Failed to open local file '{}'", localPath);
			sftp_close(file);
			return false;
		}
		char buffer[1024];
		size_t nread;
		while ((nread = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
			if (sftp_write(file, buffer, nread) != nread) {
				spdlog::error("Failed to write to remote file '{}': {}", remotePath, ssh_get_error(m_session));
				fclose(fp);
				sftp_close(file);
				return false;
			}
		}
		fclose(fp);
		sftp_close(file);
		return true;
	}
	catch (...) {
		spdlog::error("Error occurred while uploading file to SSH server");
		return false;
	}
}
bool SSHClient::downloadFile(const std::string& remotePath, const std::string& localPath) {
	try {
		if (!m_connected) {
			spdlog::error("Not connected to SSH server");
			return false;
		}
		sftp_file file = sftp_open(m_sftp, remotePath.c_str(), O_RDONLY, S_IRWXU);
		if (!file) {
			spdlog::error("Failed to open remote file '{}': {}", remotePath, ssh_get_error(m_session));
			return false;
		}
		FILE* fp = fopen(localPath.c_str(), "wb");
		if (!fp) {
			spdlog::error("Failed to create local file '{}'", localPath);
			sftp_close(file);
			return false;
		}
		char buffer[1024];
		ssize_t nread;
		while ((nread = sftp_read(file, buffer, sizeof(buffer))) > 0) {
			if (fwrite(buffer, 1, nread, fp) != (size_t)nread) {
				spdlog::error("Failed to write to local file '{}'", localPath);
				fclose(fp);
				sftp_close(file);
				return false;
			}
		}
		fclose(fp);
		sftp_close(file);
		return true;
	}
	catch (...) {
		spdlog::error("Error occurred while downloading file from SSH server");
		return false;
	}
}

bool SSHClient::isConnected() const {
    return m_connected;
}

void SSHClient::setConnectionParams(const std::string& ip, const std::string& username, const std::string& password, int port) {
    m_ip = ip;
    m_username = username;
    m_password = password;
    m_port = port;
    m_connected = false;
    m_session = ssh_new();
    if (m_session == nullptr) {
        return;
    }

    ssh_options_set(m_session, SSH_OPTIONS_HOST, m_ip.c_str());
    ssh_options_set(m_session, SSH_OPTIONS_USER, m_username.c_str());
    ssh_options_set(m_session, SSH_OPTIONS_PORT, &m_port);
    int rc = ssh_connect(m_session);
    if (rc != SSH_OK) {
        ssh_free(m_session);
        m_session = nullptr;
        return;
    }

    rc = ssh_userauth_password(m_session, nullptr, m_password.c_str());
    if (rc != SSH_AUTH_SUCCESS) {
        ssh_disconnect(m_session);
        ssh_free(m_session);
        m_session = nullptr;
        return;
    }

    m_connected = true;
}

bool SSHClient::isSftpSessionOpened() const {
    return (m_sftp != nullptr);
}

bool SSHClient::openSftpSession() {
    if (!isConnected()) {
        return false;
    }

    m_sftp = sftp_new(m_session);
    if (m_sftp == nullptr) {
        return false;
    }

    int rc = sftp_init(m_sftp);
    if (rc != SSH_OK) {
        sftp_free(m_sftp);
        m_sftp = nullptr;
        return false;
    }

    return true;
}

bool SSHClient::closeSftpSession() {
    if (isSftpSessionOpened()) {
        sftp_free(m_sftp);
        m_sftp = nullptr;
    }

    return true;
}

bool SSHClient::createRemoteDirectory(const std::string& remoteDirPath) const {
    if (!isSftpSessionOpened()) {
        return false;
    }

    return (sftp_mkdir(m_sftp, remoteDirPath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == SSH_OK);
}

bool SSHClient::renameRemoteFileOrDir(const std::string& oldPath, const std::string& newPath) const {
    if (!isSftpSessionOpened()) {
        return false;
    }

    return (sftp_rename(m_sftp, oldPath.c_str(), newPath.c_str()) == SSH_OK);
}

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SSHClientWrapper {
   SSHClient* client;
};

SSHClientWrapper* SSHClient_new(const char* ip, const char* username, const char* password, int port) {
    SSHClientWrapper* wrapper = (SSHClientWrapper*)malloc(sizeof(SSHClientWrapper));
    wrapper->client = new SSHClient(ip, username, password, port);
    return wrapper;
}

void SSHClient_delete(SSHClientWrapper* wrapper) {
    delete wrapper->client;
    free(wrapper);
}

int SSHClient_connect(SSHClientWrapper* wrapper) {
    return wrapper->client->connect();
}

int SSHClient_disconnect(SSHClientWrapper* wrapper) {
    return wrapper->client->disconnect();
}

int SSHClient_execCommand(SSHClientWrapper* wrapper, const char* command, char** output) {
    std::string out;
    bool result = wrapper->client->execCommand(command, out);
    if (result) {
        *output = strdup(out.c_str());
    }
    return result;
}

int SSHClient_uploadFile(SSHClientWrapper* wrapper, const char* localPath, const char* remotePath) {
    return wrapper->client->uploadFile(localPath, remotePath);
}

int SSHClient_downloadFile(SSHClientWrapper* wrapper, const char* remotePath, const char* localPath) {
    return wrapper->client->downloadFile(remotePath, localPath);
}

int SSHClient_isConnected(SSHClientWrapper* wrapper) {
    return wrapper->client->isConnected();
}

void SSHClient_setConnectionParams(SSHClientWrapper* wrapper, const char* ip, const char* username, const char* password, int port) {
    wrapper->client->setConnectionParams(ip, username, password, port);
}

int SSHClient_isSftpSessionOpened(SSHClientWrapper* wrapper) {
    return wrapper->client->isSftpSessionOpened();
}

int SSHClient_openSftpSession(SSHClientWrapper* wrapper) {
    return wrapper->client->openSftpSession();
}

int SSHClient_closeSftpSession(SSHClientWrapper* wrapper) {
    return wrapper->client->closeSftpSession();
}

int SSHClient_createRemoteDirectory(SSHClientWrapper* wrapper, const char* remoteDirPath) {
    return wrapper->client->createRemoteDirectory(remoteDirPath);
}

int SSHClient_renameRemoteFileOrDir(SSHClientWrapper* wrapper, const char* oldPath, const char* newPath) {
    return wrapper->client->renameRemoteFileOrDir(oldPath, newPath);
}

#ifdef __cplusplus
}
#endif
