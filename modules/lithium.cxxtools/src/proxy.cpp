#include "proxy.hpp"

#include <chrono>
#include <ctime>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <filesystem>
#include <format>

#ifdef _WIN32
#include <windows.h>
#else
#include <cstdlib>
#endif

#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"

namespace fs = std::filesystem;

namespace lithium::cxxtools {

auto NetworkProxy::setProxy(const std::string& proxy,
                            NetworkProxy::ProxyMode mode,
                            const std::string& listenIP,
                            const std::string& dns) -> bool {
    LOG_F(INFO, "Setting proxy: {} with mode: {}", proxy,
          getProxyModeName(mode));
    this->proxyMode_ = mode;
    this->listenIP_ = listenIP;
    this->dns_ = dns;

#ifdef _WIN32
    return setWindowsProxy(proxy);
#else
    return setLinuxProxy(proxy);
#endif
}

[[nodiscard]] auto NetworkProxy::disableProxy() const -> bool {
    LOG_F(INFO, "Disabling proxy");
#ifdef _WIN32
    return disableWindowsProxy();
#else
    return disableLinuxProxy();
#endif
}

[[nodiscard]] auto NetworkProxy::getCurrentProxy() -> std::string {
#ifdef _WIN32

    return getWindowsCurrentProxy();
#else
    return getLinuxCurrentProxy();
#endif
}

[[nodiscard]] auto NetworkProxy::installCertificate(
    const std::string& certPath) const -> bool {
    LOG_F(INFO, "Installing certificate: {}", certPath);
#ifdef _WIN32
    return installWindowsCertificate(certPath);
#else
    return installLinuxCertificate(certPath);
#endif
}

[[nodiscard]] auto NetworkProxy::uninstallCertificate(
    const std::string& certName) const -> bool {
    LOG_F(INFO, "Uninstalling certificate: {}", certName);
#ifdef _WIN32
    return uninstallWindowsCertificate(certName);
#else
    return uninstallLinuxCertificate(certName);
#endif
}

[[nodiscard]] auto NetworkProxy::viewCertificateInfo(
    const std::string& certName) const -> std::string {
    LOG_F(INFO, "Viewing certificate info: {}", certName);
#ifdef _WIN32
    return viewWindowsCertificateInfo(certName);
#else
    return viewLinuxCertificateInfo(certName);
#endif
}

void NetworkProxy::editHostsFile(
    const std::vector<std::pair<std::string, std::string>>& hostsEntries) {
    LOG_F(INFO, "Editing Hosts file");
#ifdef _WIN32
    editWindowsHostsFile(hostsEntries);
#else
    editLinuxHostsFile(hostsEntries);
#endif
}

void NetworkProxy::resetHostsFile() {
    LOG_F(INFO, "Resetting Hosts file");
#ifdef _WIN32
    resetWindowsHostsFile();
#else
    resetLinuxHostsFile();
#endif
}

void NetworkProxy::enableHttpToHttpsRedirect(bool enable) {
    LOG_F(INFO, "Enabling HTTP to HTTPS redirect: {}",
          std::string(enable ? "enabled" : "disabled"));
    this->httpToHttpsRedirect_ = enable;
}

void NetworkProxy::setCustomDoH(const std::string& dohUrl) {
    LOG_F(INFO, "Setting custom DoH: {}", dohUrl);
    this->dohUrl_ = dohUrl;
}

auto NetworkProxy::getProxyModeName(ProxyMode mode) -> std::string {
    switch (mode) {
        case ProxyMode::Hosts:
            return "Hosts";
        case ProxyMode::PAC:
            return "PAC";
        case ProxyMode::System:
            return "System";
        default:
            return "Unknown";
    }
}

#ifdef _WIN32
bool NetworkProxy::setWindowsProxy(const std::string& proxy) const {
    HKEY hKey;
    const char* regPath =
        "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";

    // 打开注册表键
    if (RegOpenKeyEx(HKEY_CURRENT_USER, regPath, 0, KEY_SET_VALUE, &hKey) !=
        ERROR_SUCCESS) {
        LOG_F(ERROR, "Failed to open registry key.");
        return false;
    }

    // 启用代理设置（ProxyEnable=1）
    DWORD proxyEnable = 1;
    if (RegSetValueEx(hKey, "ProxyEnable", 0, REG_DWORD, (BYTE*)&proxyEnable,
                      sizeof(proxyEnable)) != ERROR_SUCCESS) {
        LOG_F(ERROR, "Failed to enable proxy.");
        RegCloseKey(hKey);
        return false;
    }

    // 设置代理服务器地址
    if (RegSetValueEx(hKey, "ProxyServer", 0, REG_SZ, (BYTE*)proxy.c_str(),
                      proxy.length() + 1) != ERROR_SUCCESS) {
        LOG_F(ERROR, "Failed to set proxy server.");
        RegCloseKey(hKey);
        return false;
    }

    // 可选：设置代理例外（如不使用代理的本地地址）
    std::string proxyOverride = "<local>";  // 默认设置本地地址例外
    if (RegSetValueEx(hKey, "ProxyOverride", 0, REG_SZ,
                      (BYTE*)proxyOverride.c_str(),
                      proxyOverride.length() + 1) != ERROR_SUCCESS) {
        LOG_F(ERROR, "Failed to set proxy override.");
        RegCloseKey(hKey);
        return false;
    }

    // 如果指定了监听IP，则更新代理覆盖字段以包含此IP
    if (!listenIP_.empty() && listenIP_ != "0.0.0.0") {
        std::string currentOverride;
        DWORD size = 0;
        RegQueryValueEx(hKey, "ProxyOverride", NULL, NULL, NULL,
                        &size);  // 获取当前大小
        currentOverride.resize(size);
        RegQueryValueEx(hKey, "ProxyOverride", NULL, NULL,
                        (BYTE*)&currentOverride[0], &size);

        if (!currentOverride.empty() && currentOverride.back() != ';') {
            currentOverride += ";";
        }
        currentOverride += listenIP_;

        if (RegSetValueEx(hKey, "ProxyOverride", 0, REG_SZ,
                          (BYTE*)currentOverride.c_str(),
                          currentOverride.length() + 1) != ERROR_SUCCESS) {
            LOG_F(ERROR, "Failed to update proxy override with listen IP.");
            RegCloseKey(hKey);
            return false;
        }
    }

    // 如果指定了自定义DNS，则通过 `netsh` 命令设置
    if (!dns_.empty()) {
        std::string dnsCommand =
            "netsh interface ip set dns name=\"Local Area Connection\" "
            " " +
            dns_;
        if (atom::system::executeCommandWithStatus(dnsCommand).second != 0) {
            LOG_F(ERROR, "Failed to set custom DNS.");
        }
    }

    RegCloseKey(hKey);
    return true;
}

bool NetworkProxy::disableWindowsProxy() const {
    HKEY hKey;
    const char* regPath =
        "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";

    if (RegOpenKeyEx(HKEY_CURRENT_USER, regPath, 0, KEY_SET_VALUE, &hKey) !=
        ERROR_SUCCESS) {
        LOG_F(ERROR, "Failed to open registry key.");
        return false;
    }

    DWORD proxyEnable = 0;
    if (RegSetValueEx(hKey, "ProxyEnable", 0, REG_DWORD, (BYTE*)&proxyEnable,
                      sizeof(proxyEnable)) != ERROR_SUCCESS) {
        LOG_F(ERROR, "Failed to disable proxy.");
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}

std::string NetworkProxy::getWindowsCurrentProxy() const {
    HKEY hKey;
    const char* regPath =
        "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";
    char buffer[256];
    DWORD bufferSize = sizeof(buffer);

    if (RegOpenKeyEx(HKEY_CURRENT_USER, regPath, 0, KEY_QUERY_VALUE, &hKey) !=
        ERROR_SUCCESS) {
        LOG_F(ERROR, "Failed to open registry key.");
        return "";
    }

    if (RegQueryValueEx(hKey, "ProxyServer", NULL, NULL, (BYTE*)buffer,
                        &bufferSize) != ERROR_SUCCESS) {
        LOG_F(ERROR, "Failed to query proxy.");
        RegCloseKey(hKey);
        return "";
    }

    RegCloseKey(hKey);
    return std::string(buffer);
}

bool NetworkProxy::installWindowsCertificate(const std::string& certPath) const {
    std::string command = "certutil -addstore -f \"Root\" " + certPath;
    int result = system(command.c_str());
    if (result != 0) {
        LOG_F(ERROR, "Failed to install certificate.");
        return false;
    }
    return true;
}

bool NetworkProxy::uninstallWindowsCertificate(const std::string& certName) const {
    std::string command = "certutil -delstore \"Root\" " + certName;
    int result = system(command.c_str());
    if (result != 0) {
        LOG_F(ERROR, "Failed to uninstall certificate.");
        return false;
    }
    return true;
}

std::string NetworkProxy::viewWindowsCertificateInfo(const std::string& certName) const {
    std::string command = "certutil -store \"Root\" " + certName;
    std::string result;
    char buffer[128];
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe)
        throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            result += buffer;
        }
    } catch (...) {
        _pclose(pipe);
        throw;
    }
    _pclose(pipe);
    return result;
}

void NetworkProxy::editWindowsHostsFile(
    const std::vector<std::pair<std::string, std::string>>& hostsEntries)
    const {
    std::string hostsPath = "C:\\Windows\\System32\\drivers\\etc\\hosts";
    std::ofstream hostsFile(hostsPath, std::ios_base::app);
    for (const auto& entry : hostsEntries) {
        hostsFile << entry.first << " " << entry.second << "\n";
    }
    hostsFile.close();
}

void NetworkProxy::resetWindowsHostsFile() const {
    std::string hostsPath = "C:\\Windows\\System32\\drivers\\etc\\hosts";
    std::ofstream hostsFile(hostsPath, std::ios_base::trunc);
    hostsFile.close();
    LOG_F(INFO, "Hosts file has been reset.");
}

#else
[[nodiscard]] auto NetworkProxy::setLinuxProxy(const std::string& proxy) const
    -> bool {
    std::string httpProxy = "http_proxy=" + proxy;
    std::string httpsProxy = "https_proxy=" + proxy;
    std::string noProxy =
        "no_proxy=localhost,127.0.0.1";  // Example for local exceptions

    if (putenv(const_cast<char*>(httpProxy.c_str())) != 0 ||
        putenv(const_cast<char*>(httpsProxy.c_str())) != 0 ||
        putenv(const_cast<char*>(noProxy.c_str())) != 0) {
        LOG_F(ERROR, "Failed to set proxy environment variables.");
        return false;
    }

    if (!dns_.empty()) {
        std::string resolvConfPath = "/etc/resolv.conf";
        std::ofstream resolvConf(resolvConfPath, std::ios_base::trunc);
        if (!resolvConf.is_open()) {
            LOG_F(ERROR, "Failed to open /etc/resolv.conf for writing.");
            return false;
        }
        resolvConf << "nameserver " << dns_ << "\n";
        resolvConf.close();
        LOG_F(INFO, "Custom DNS set to: {}", dns_);
    }

    return true;
}

auto NetworkProxy::disableLinuxProxy() -> bool {
    if (unsetenv("http_proxy") != 0 || unsetenv("https_proxy") != 0 ||
        unsetenv("no_proxy") != 0) {
        LOG_F(ERROR, "Failed to unset proxy environment variables.");
        return false;
    }

    return true;
}

auto NetworkProxy::getLinuxCurrentProxy() -> std::string {
    char* httpProxy = getenv("http_proxy");
    if (httpProxy != nullptr) {
        return std::string(httpProxy);
    }

    return "";
}

auto NetworkProxy::installLinuxCertificate(const std::string& certPath)
    -> bool {
    std::string command =
        "sudo cp " + certPath +
        " /usr/local/share/ca-certificates/ && sudo update-ca-certificates";
    int result = atom::system::executeCommandWithStatus(command).second;
    if (result != 0) {
        LOG_F(ERROR, "Failed to install certificate.");
        return false;
    }
    return true;
}

auto NetworkProxy::uninstallLinuxCertificate(const std::string& certName)
    -> bool {
    std::string command = "sudo rm /usr/local/share/ca-certificates/" +
                          certName + " && sudo update-ca-certificates --fresh";
    int result = atom::system::executeCommandWithStatus(command).second;
    if (result != 0) {
        LOG_F(ERROR, "Failed to uninstall certificate.");
        return false;
    }
    return true;
}

auto NetworkProxy::viewLinuxCertificateInfo(const std::string& certName)
    -> std::string {
    std::string command = "openssl x509 -in /usr/local/share/ca-certificates/" +
                          certName + " -text -noout";
    return atom::system::executeCommand(command);
}

void NetworkProxy::editLinuxHostsFile(
    const std::vector<std::pair<std::string, std::string>>& hostsEntries)
    const {
    std::string hostsPath = "/etc/hosts";
    std::ofstream hostsFile(hostsPath, std::ios_base::app);
    if (!hostsFile.is_open()) {
        LOG_F(ERROR, "Failed to open hosts file for writing.");
        return;
    }
    for (const auto& entry : hostsEntries) {
        hostsFile << entry.first << " " << entry.second << "\n";
    }
    hostsFile.close();
    LOG_F(INFO, "Hosts file has been edited.");
}

void NetworkProxy::resetLinuxHostsFile() const {
    std::string hostsPath = "/etc/hosts";
    std::ofstream hostsFile(hostsPath, std::ios_base::trunc);
    if (!hostsFile.is_open()) {
        LOG_F(ERROR, "Failed to open hosts file for writing.");
        return;
    }
    hostsFile << "127.0.0.1   localhost\n";
    hostsFile.close();
    LOG_F(INFO, "Hosts file has been reset.");
}
#endif
}  // namespace lithium::cxxtools
