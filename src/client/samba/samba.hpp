#ifndef LITHIUM_CLIENT_SAMBA_MANAGER_HPP
#define LITHIUM_CLIENT_SAMBA_MANAGER_HPP

#include <string>

namespace lithium {
class SambaManager {
public:
    auto addUser(const std::string& username) -> bool;
    auto deleteUser(const std::string& username) -> bool;
    auto changeUserPassword(const std::string& username) -> bool;
    auto enableUser(const std::string& username) -> bool;
    auto disableUser(const std::string& username) -> bool;
    auto createSharedDirectory(const std::string& path) -> bool;
    auto deleteSharedDirectory(const std::string& path) -> bool;
    auto addSharedDirectoryConfig(const std::string& name,
                                  const std::string& path) -> bool;
    auto modifySharedDirectoryConfig(const std::string& name,
                                     const std::string& path,
                                     const std::string& newPath) -> bool;
    auto deleteSharedDirectoryConfig(const std::string& name) -> bool;
    auto listSambaUsers() -> bool;
    auto listSharedDirectories() -> bool;

private:
    auto restartSamba() -> bool;
};
}  // namespace lithium

#endif  // LITHIUM_CLIENT_SAMBA_MANAGER_HPP
