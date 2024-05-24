/*
 * sshserver.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-5-24

Description: SSH Server

*************************************************/

#include "sshserver.hpp"

#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#else
#include <unistd.h>
#include <csignal>
#endif

#include "atom/error/exception.hpp"

namespace atom::connection {
class SshServer::Impl {
public:
    explicit Impl(const std::filesystem::path& configFile)
        : configFile_(configFile) {
        loadConfig();
    }

    void start() {
        if (isRunning()) {
            THROW_RUNTIME_ERROR("SSH server is already running");
        }

        saveConfig();

#ifdef _WIN32
        std::string command =
            "start /b sshd -f \"" + configFile_.string() + "\"";
        system(command.c_str());
#else
        std::string command =
            "/usr/sbin/sshd -f \"" + configFile_.string() + "\" -D &";
        system(command.c_str());
#endif
    }

    void stop() {
        if (!isRunning()) {
            THROW_RUNTIME_ERROR("SSH server is not running");
        }

#ifdef _WIN32
        system("taskkill /F /IM sshd.exe > nul");
#else
        system("pkill -f sshd");
#endif
    }

    bool isRunning() const {
#ifdef _WIN32
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return false;
        }

        PROCESSENTRY32 entry{};
        entry.dwSize = sizeof(entry);

        if (!Process32First(snapshot, &entry)) {
            CloseHandle(snapshot);
            return false;
        }

        do {
            if (_stricmp(entry.szExeFile, "sshd.exe") == 0) {
                CloseHandle(snapshot);
                return true;
            }
        } while (Process32Next(snapshot, &entry));

        CloseHandle(snapshot);
        return false;
#else
        std::array<char, 128> buffer{};
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("pgrep sshd", "r"),
                                                      pclose);
        if (!pipe) {
            THROW_RUNTIME_ERROR("Failed to execute pgrep command");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return !result.empty();
#endif
    }

    void setPort(int port) { port_ = port; }

    int getPort() const { return port_; }

    void setListenAddress(const std::string& address) {
        listenAddress_ = address;
    }

    std::string getListenAddress() const { return listenAddress_; }

    void setHostKey(const std::filesystem::path& keyFile) {
        hostKey_ = keyFile;
    }

    std::filesystem::path getHostKey() const { return hostKey_; }

    void setAuthorizedKeys(const std::vector<std::filesystem::path>& keyFiles) {
        authorizedKeys_ = keyFiles;
    }

    std::vector<std::filesystem::path> getAuthorizedKeys() const {
        return authorizedKeys_;
    }

    void allowRootLogin(bool allow) { allowRootLogin_ = allow; }

    bool isRootLoginAllowed() const { return allowRootLogin_; }

    void setPasswordAuthentication(bool enable) {
        passwordAuthentication_ = enable;
    }

    bool isPasswordAuthenticationEnabled() const {
        return passwordAuthentication_;
    }

    void setSubsystem(const std::string& name, const std::string& command) {
        subsystems_[name] = command;
    }

    void removeSubsystem(const std::string& name) { subsystems_.erase(name); }

    std::string getSubsystem(const std::string& name) const {
        auto it = subsystems_.find(name);
        if (it != subsystems_.end()) {
            return it->second;
        }
        return {};
    }

private:
    void loadConfig() {
        std::ifstream file(configFile_);
        if (!file) {
            THROW_RUNTIME_ERROR(
                "Failed to open SSH server configuration file");
        }

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string key, value;
            if (std::getline(iss, key, ' ') && std::getline(iss, value)) {
                if (key == "Port") {
                    port_ = std::stoi(value);
                } else if (key == "ListenAddress") {
                    listenAddress_ = value;
                } else if (key == "HostKey") {
                    hostKey_ = value;
                } else if (key == "AuthorizedKeysFile") {
                    authorizedKeys_.push_back(value);
                } else if (key == "PermitRootLogin") {
                    allowRootLogin_ = (value == "yes");
                } else if (key == "PasswordAuthentication") {
                    passwordAuthentication_ = (value == "yes");
                } else if (key == "Subsystem") {
                    std::istringstream subsystemIss(value);
                    std::string subsystemName, subsystemCommand;
                    if (std::getline(subsystemIss, subsystemName, ' ') &&
                        std::getline(subsystemIss, subsystemCommand)) {
                        subsystems_[subsystemName] = subsystemCommand;
                    }
                }
            }
        }
    }

    void saveConfig() {
        std::ofstream file(configFile_);
        if (!file) {
            THROW_RUNTIME_ERROR(
                "Failed to save SSH server configuration file");
        }

        file << "Port " << port_ << '\n';
        file << "ListenAddress " << listenAddress_ << '\n';
        file << "HostKey " << hostKey_.string() << '\n';
        for (const auto& keyFile : authorizedKeys_) {
            file << "AuthorizedKeysFile " << keyFile.string() << '\n';
        }
        file << "PermitRootLogin " << (allowRootLogin_ ? "yes" : "no") << '\n';
        file << "PasswordAuthentication "
             << (passwordAuthentication_ ? "yes" : "no") << '\n';
        for (const auto& [name, command] : subsystems_) {
            file << "Subsystem " << name << " " << command << '\n';
        }
    }

    std::filesystem::path configFile_;
    int port_ = 22;
    std::string listenAddress_ = "0.0.0.0";
    std::filesystem::path hostKey_;
    std::vector<std::filesystem::path> authorizedKeys_;
    bool allowRootLogin_ = false;
    bool passwordAuthentication_ = false;
    std::unordered_map<std::string, std::string> subsystems_;
};

SshServer::SshServer(const std::filesystem::path& configFile)
    : impl_(std::make_unique<Impl>(configFile)) {}

SshServer::~SshServer() = default;

void SshServer::start() { impl_->start(); }

void SshServer::stop() { impl_->stop(); }

bool SshServer::isRunning() const { return impl_->isRunning(); }

void SshServer::setPort(int port) { impl_->setPort(port); }

int SshServer::getPort() const { return impl_->getPort(); }

void SshServer::setListenAddress(const std::string& address) {
    impl_->setListenAddress(address);
}

std::string SshServer::getListenAddress() const {
    return impl_->getListenAddress();
}

void SshServer::setHostKey(const std::filesystem::path& keyFile) {
    impl_->setHostKey(keyFile);
}

std::filesystem::path SshServer::getHostKey() const {
    return impl_->getHostKey();
}

void SshServer::setAuthorizedKeys(
    const std::vector<std::filesystem::path>& keyFiles) {
    impl_->setAuthorizedKeys(keyFiles);
}

std::vector<std::filesystem::path> SshServer::getAuthorizedKeys() const {
    return impl_->getAuthorizedKeys();
}

void SshServer::allowRootLogin(bool allow) { impl_->allowRootLogin(allow); }

bool SshServer::isRootLoginAllowed() const {
    return impl_->isRootLoginAllowed();
}

void SshServer::setPasswordAuthentication(bool enable) {
    impl_->setPasswordAuthentication(enable);
}

bool SshServer::isPasswordAuthenticationEnabled() const {
    return impl_->isPasswordAuthenticationEnabled();
}

void SshServer::setSubsystem(const std::string& name,
                             const std::string& command) {
    impl_->setSubsystem(name, command);
}

void SshServer::removeSubsystem(const std::string& name) {
    impl_->removeSubsystem(name);
}

std::string SshServer::getSubsystem(const std::string& name) const {
    return impl_->getSubsystem(name);
}
}  // namespace atom::connection
