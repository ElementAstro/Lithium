#include "samba.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"

namespace lithium {
auto SambaManager::addUser(const std::string& username) -> bool {
    std::string command = "sudo smbpasswd -a " + username;
    auto res = atom::system::executeCommandWithStatus(command);
    if (res.second != 0) {
        LOG_F(ERROR, "Failed to add Samba user: {}", username);
        return false;
    }
    LOG_F(INFO, "Added Samba user: {}", username);
    return true;
}

auto SambaManager::deleteUser(const std::string& username) -> bool {
    std::string command = "sudo smbpasswd -x " + username;
    auto res = atom::system::executeCommandWithStatus(command);
    if (res.second != 0) {
        LOG_F(ERROR, "Failed to delete Samba user: {}", username);
        return false;
    }
    LOG_F(INFO, "Deleted Samba user: {}", username);
    return true;
}

auto SambaManager::changeUserPassword(const std::string& username) -> bool {
    std::string command = "sudo smbpasswd " + username;
    auto res = atom::system::executeCommandWithStatus(command);
    if (res.second != 0) {
        LOG_F(ERROR, "Failed to change password for Samba user: {}", username);
        return false;
    }
    LOG_F(INFO, "Changed password for Samba user: {}", username);
    return true;
}

auto SambaManager::enableUser(const std::string& username) -> bool {
    std::string command = "sudo smbpasswd -e " + username;
    auto res = atom::system::executeCommandWithStatus(command);
    if (res.second != 0) {
        LOG_F(ERROR, "Failed to enable Samba user: {}", username);
        return false;
    }
    LOG_F(INFO, "Enabled Samba user: {}", username);
    return true;
}

auto SambaManager::disableUser(const std::string& username) -> bool {
    std::string command = "sudo smbpasswd -d " + username;
    auto res = atom::system::executeCommandWithStatus(command);
    if (res.second != 0) {
        LOG_F(ERROR, "Failed to disable Samba user: {}", username);
        return false;
    }
    LOG_F(INFO, "Disabled Samba user: {}", username);
    return true;
}

auto SambaManager::createSharedDirectory(const std::string& path) -> bool {
    std::string command =
        "sudo mkdir -p " + path + " && sudo chmod 777 " + path;
    auto res = atom::system::executeCommandWithStatus(command);
    if (res.second != 0) {
        LOG_F(ERROR, "Failed to create shared directory: {}", path);
        return false;
    }
    LOG_F(INFO, "Created shared directory: {}", path);
    return true;
}

auto SambaManager::deleteSharedDirectory(const std::string& path) -> bool {
    std::string command = "sudo rm -rf " + path;
    auto res = atom::system::executeCommandWithStatus(command);
    if (res.second != 0) {
        LOG_F(ERROR, "Failed to delete shared directory: {}", path);
        return false;
    }
    LOG_F(INFO, "Deleted shared directory: {}", path);
    return true;
}

auto SambaManager::addSharedDirectoryConfig(const std::string& name,
                                            const std::string& path) -> bool {
    std::ofstream smbConf("/etc/samba/smb.conf", std::ios::app);
    if (smbConf.is_open()) {
        smbConf << "\n[" << name << "]\n";
        smbConf << "   path = " << path << "\n";
        smbConf << "   browseable = yes\n";
        smbConf << "   read only = no\n";
        smbConf << "   guest ok = yes\n";
        smbConf.close();
        LOG_F(INFO, "Added shared directory config: {}", name);
        restartSamba();
        return true;
    }
    LOG_F(ERROR, "Failed to open smb.conf for writing");
    return false;
}

auto SambaManager::modifySharedDirectoryConfig(
    const std::string& name, const std::string& path,
    const std::string& newPath) -> bool {
    std::ifstream smbConfIn("/etc/samba/smb.conf");
    std::ofstream smbConfOut("/etc/samba/smb.conf.tmp");
    std::string line;
    bool found = false;

    if (smbConfIn.is_open() && smbConfOut.is_open()) {
        while (getline(smbConfIn, line)) {
            if (line == "[" + name + "]") {
                found = true;
                smbConfOut << line << "\n";
                while (getline(smbConfIn, line) && !line.empty()) {
                    if (line.find("path = " + path) != std::string::npos) {
                        smbConfOut << "   path = " << newPath << "\n";
                    } else {
                        smbConfOut << line << "\n";
                    }
                }
            } else {
                smbConfOut << line << "\n";
            }
        }
        smbConfIn.close();
        smbConfOut.close();

        if (found) {
            std::string command =
                "sudo mv /etc/samba/smb.conf.tmp /etc/samba/smb.conf";
            auto res = atom::system::executeCommandWithStatus(command);
            if (res.second != 0) {
                LOG_F(ERROR, "Failed to modify shared directory config: {}",
                      name);
                return false;
            }
            LOG_F(INFO, "Modified shared directory config: {}", name);
            restartSamba();
            return true;
        }
        LOG_F(ERROR, "Shared directory config not found: {}", name);
    } else {
        LOG_F(ERROR, "Failed to open smb.conf for reading or writing");
    }
    return false;
}

auto SambaManager::deleteSharedDirectoryConfig(const std::string& name)
    -> bool {
    std::ifstream smbConfIn("/etc/samba/smb.conf");
    std::ofstream smbConfOut("/etc/samba/smb.conf.tmp");
    std::string line;
    bool found = false;

    if (smbConfIn.is_open() && smbConfOut.is_open()) {
        while (getline(smbConfIn, line)) {
            if (line == "[" + name + "]") {
                found = true;
                while (getline(smbConfIn, line) && !line.empty())
                    ;
            } else {
                smbConfOut << line << "\n";
            }
        }
        smbConfIn.close();
        smbConfOut.close();

        if (found) {
            std::string command =
                "sudo mv /etc/samba/smb.conf.tmp /etc/samba/smb.conf";
            auto res = atom::system::executeCommandWithStatus(command);
            if (res.second != 0) {
                LOG_F(ERROR, "Failed to delete shared directory config: {}",
                      name);
                return false;
            }
            LOG_F(INFO, "Deleted shared directory config: {}", name);

            restartSamba();
            return true;
        }
        LOG_F(ERROR, "Shared directory config not found: {}", name);

    } else {
        LOG_F(ERROR, "Failed to open smb.conf for reading or writing");
    }
    return false;
}

auto SambaManager::listSambaUsers() -> bool {
    std::string command = "sudo pdbedit -L";
    auto res = atom::system::executeCommand(command);
    if (res.empty()) {
        LOG_F(ERROR, "Failed to list Samba users");
        return false;
    }
    LOG_F(INFO, "Samba users: {}", res);
    return true;
}

auto SambaManager::listSharedDirectories() -> bool {
    std::ifstream smbConf("/etc/samba/smb.conf");
    std::string line;
    if (smbConf.is_open()) {
        LOG_F(INFO, "Shared directories:");
        while (getline(smbConf, line)) {
            if (line[0] == '[' && line[line.size() - 1] == ']') {
                LOG_F(INFO, "{}", line);
            }
        }
        smbConf.close();
        return true;
    }
    LOG_F(ERROR, "Failed to open smb.conf for reading");

    return false;
}

auto SambaManager::restartSamba() -> bool {
    std::string command = "sudo systemctl restart smbd";
    auto res = atom::system::executeCommandWithStatus(command);
    if (res.second != 0) {
        LOG_F(ERROR, "Failed to restart Samba service");
        return false;
    }
    LOG_F(INFO, "Samba service restarted");
    return true;
}

}  // namespace lithium
