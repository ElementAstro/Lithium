#include "proxy.hpp"

#include <ctime>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <cstdlib>
#endif

#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"

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

    try {
#ifdef _WIN32
        if (!setWindowsProxy(proxy)) {
            LOG_F(ERROR, "Failed to set Windows proxy.");
            return false;
        }
#else
        if (!setLinuxProxy(proxy)) {
            LOG_F(ERROR, "Failed to set Linux proxy.");
            return false;
        }
#endif
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in setProxy: {}", e.what());
        return false;
    }

    LOG_F(INFO, "Proxy has been set successfully.");
    return true;
}

[[nodiscard]] auto NetworkProxy::disableProxy() const -> bool {
    LOG_F(INFO, "Disabling proxy");
    try {
#ifdef _WIN32
        if (!disableWindowsProxy()) {
            LOG_F(ERROR, "Failed to disable Windows proxy.");
            return false;
        }
#else
        if (!disableLinuxProxy()) {
            LOG_F(ERROR, "Failed to disable Linux proxy.");
            return false;
        }
#endif
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in disableProxy: {}", e.what());
        return false;
    }
    LOG_F(INFO, "Proxy has been disabled successfully.");
    return true;
}

[[nodiscard]] auto NetworkProxy::getCurrentProxy() -> std::string {
    LOG_F(INFO, "Retrieving current proxy settings.");
    try {
#ifdef _WIN32
        return getWindowsCurrentProxy();
#else
        return getLinuxCurrentProxy();
#endif
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in getCurrentProxy: {}", e.what());
        return "";
    }
}

[[nodiscard]] auto NetworkProxy::installCertificate(
    const std::string& certPath) const -> bool {
    LOG_F(INFO, "Installing certificate: {}", certPath);
    try {
#ifdef _WIN32
        if (!installWindowsCertificate(certPath)) {
            LOG_F(ERROR, "Failed to install Windows certificate.");
            return false;
        }
#else
        if (!installLinuxCertificate(certPath)) {
            LOG_F(ERROR, "Failed to install Linux certificate.");
            return false;
        }
#endif
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in installCertificate: {}", e.what());
        return false;
    }
    LOG_F(INFO, "Certificate installed successfully.");
    return true;
}

[[nodiscard]] auto NetworkProxy::uninstallCertificate(
    const std::string& certName) const -> bool {
    LOG_F(INFO, "Uninstalling certificate: {}", certName);
    try {
#ifdef _WIN32
        if (!uninstallWindowsCertificate(certName)) {
            LOG_F(ERROR, "Failed to uninstall Windows certificate.");
            return false;
        }
#else
        if (!uninstallLinuxCertificate(certName)) {
            LOG_F(ERROR, "Failed to uninstall Linux certificate.");
            return false;
        }
#endif
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in uninstallCertificate: {}", e.what());
        return false;
    }
    LOG_F(INFO, "Certificate uninstalled successfully.");
    return true;
}

[[nodiscard]] auto NetworkProxy::viewCertificateInfo(
    const std::string& certName) const -> std::string {
    LOG_F(INFO, "Viewing certificate info: {}", certName);
    try {
#ifdef _WIN32
        return viewWindowsCertificateInfo(certName);
#else
        return viewLinuxCertificateInfo(certName);
#endif
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in viewCertificateInfo: {}", e.what());
        return "";
    }
}

void NetworkProxy::editHostsFile(
    const std::vector<std::pair<std::string, std::string>>& hostsEntries) {
    LOG_F(INFO, "Editing Hosts file with {} entries.", hostsEntries.size());
    try {
#ifdef _WIN32
        editWindowsHostsFile(hostsEntries);
#else
        editLinuxHostsFile(hostsEntries);
#endif
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in editHostsFile: {}", e.what());
    }
    LOG_F(INFO, "Hosts file has been edited successfully.");
}

void NetworkProxy::resetHostsFile() {
    LOG_F(INFO, "Resetting Hosts file.");
    try {
#ifdef _WIN32
        resetWindowsHostsFile();
#else
        resetLinuxHostsFile();
#endif
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in resetHostsFile: {}", e.what());
    }
    LOG_F(INFO, "Hosts file has been reset successfully.");
}

void NetworkProxy::enableHttpToHttpsRedirect(bool enable) {
    LOG_F(INFO, "Enabling HTTP to HTTPS redirect: {}",
          enable ? "enabled" : "disabled");
    this->httpToHttpsRedirect_ = enable;
    // Additional functionality can be implemented here if needed
}

void NetworkProxy::setCustomDoH(const std::string& dohUrl) {
    LOG_F(INFO, "Setting custom DoH: {}", dohUrl);
    this->dohUrl_ = dohUrl;
    // Additional functionality can be implemented here if needed
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

    LOG_F(INFO, "Opening registry key: {}", regPath);
    // Open registry key
    if (RegOpenKeyEx(HKEY_CURRENT_USER, regPath, 0, KEY_SET_VALUE, &hKey) !=
        ERROR_SUCCESS) {
        LOG_F(ERROR, "Failed to open registry key: {}", regPath);
        return false;
    }
    LOG_F(INFO, "Registry key opened successfully.");

    // Enable proxy settings (ProxyEnable=1)
    DWORD proxyEnable = 1;
    if (RegSetValueEx(hKey, "ProxyEnable", 0, REG_DWORD,
                      reinterpret_cast<const BYTE*>(&proxyEnable),
                      sizeof(proxyEnable)) != ERROR_SUCCESS) {
        LOG_F(ERROR, "Failed to enable proxy.");
        RegCloseKey(hKey);
        return false;
    }
    LOG_F(INFO, "Proxy enabled in registry.");

    // Set proxy server address
    if (RegSetValueEx(hKey, "ProxyServer", 0, REG_SZ,
                      reinterpret_cast<const BYTE*>(proxy.c_str()),
                      proxy.length() + 1) != ERROR_SUCCESS) {
        LOG_F(ERROR, "Failed to set proxy server.");
        RegCloseKey(hKey);
        return false;
    }
    LOG_F(INFO, "Proxy server set to {}", proxy);

    // Optional: Set proxy exceptions (e.g., local addresses)
    std::string proxyOverride =
        "<local>";  // Default to local address exceptions
    if (RegSetValueEx(hKey, "ProxyOverride", 0, REG_SZ,
                      reinterpret_cast<const BYTE*>(proxyOverride.c_str()),
                      proxyOverride.length() + 1) != ERROR_SUCCESS) {
        LOG_F(ERROR, "Failed to set proxy override.");
        RegCloseKey(hKey);
        return false;
    }
    LOG_F(INFO, "Proxy override set to {}", proxyOverride);

    // If a listenIP is specified, update the proxy override field to include
    // this IP
    if (!listenIP_.empty() && listenIP_ != "0.0.0.0") {
        LOG_F(INFO, "Updating proxy override with listen IP: {}", listenIP_);
        std::string currentOverride;
        DWORD size = 0;
        RegQueryValueEx(hKey, "ProxyOverride", NULL, NULL, NULL,
                        &size);  // Get current size
        currentOverride.resize(size);
        RegQueryValueEx(hKey, "ProxyOverride", NULL, NULL,
                        reinterpret_cast<BYTE*>(&currentOverride[0]), &size);

        if (!currentOverride.empty() && currentOverride.back() != ';') {
            currentOverride += ";";
        }
        currentOverride += listenIP_;

        if (RegSetValueEx(
                hKey, "ProxyOverride", 0, REG_SZ,
                reinterpret_cast<const BYTE*>(currentOverride.c_str()),
                currentOverride.length() + 1) != ERROR_SUCCESS) {
            LOG_F(ERROR, "Failed to update proxy override with listen IP.");
            RegCloseKey(hKey);
            return false;
        }
        LOG_F(INFO, "Proxy override updated with listen IP: {}", listenIP_);
    }

    // If a custom DNS is specified, set it using the `netsh` command
    if (!dns_.empty()) {
        LOG_F(INFO, "Setting custom DNS: {}", dns_);
        std::string dnsCommand =
            "netsh interface ip set dns name=\"Local Area Connection\" " + dns_;
        if (atom::system::executeCommandWithStatus(dnsCommand).second != 0) {
            LOG_F(ERROR, "Failed to set custom DNS.");
            // Continue even if DNS setting fails
        } else {
            LOG_F(INFO, "Custom DNS set to {}", dns_);
        }
    }

    RegCloseKey(hKey);
    LOG_F(INFO, "Windows proxy settings configured successfully.");
    return true;
}

bool NetworkProxy::disableWindowsProxy() const {
    HKEY hKey;
    const char* regPath =
        "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";

    LOG_F(INFO, "Opening registry key: {}", regPath);
    if (RegOpenKeyEx(HKEY_CURRENT_USER, regPath, 0, KEY_SET_VALUE, &hKey) !=
        ERROR_SUCCESS) {
        LOG_F(ERROR, "Failed to open registry key: {}", regPath);
        return false;
    }
    LOG_F(INFO, "Registry key opened successfully.");

    DWORD proxyEnable = 0;
    if (RegSetValueEx(hKey, "ProxyEnable", 0, REG_DWORD,
                      reinterpret_cast<const BYTE*>(&proxyEnable),
                      sizeof(proxyEnable)) != ERROR_SUCCESS) {
        LOG_F(ERROR, "Failed to disable proxy.");
        RegCloseKey(hKey);
        return false;
    }
    LOG_F(INFO, "Proxy disabled in registry.");

    RegCloseKey(hKey);
    LOG_F(INFO, "Windows proxy settings disabled successfully.");
    return true;
}

std::string NetworkProxy::getWindowsCurrentProxy() const {
    HKEY hKey;
    const char* regPath =
        "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";
    char buffer[256];
    DWORD bufferSize = sizeof(buffer);

    LOG_F(INFO, "Opening registry key to get current proxy: {}", regPath);
    if (RegOpenKeyEx(HKEY_CURRENT_USER, regPath, 0, KEY_QUERY_VALUE, &hKey) !=
        ERROR_SUCCESS) {
        LOG_F(ERROR, "Failed to open registry key: {}", regPath);
        return "";
    }
    LOG_F(INFO, "Registry key opened successfully.");

    if (RegQueryValueEx(hKey, "ProxyServer", NULL, NULL,
                        reinterpret_cast<BYTE*>(buffer),
                        &bufferSize) != ERROR_SUCCESS) {
        LOG_F(ERROR, "Failed to query proxy server.");
        RegCloseKey(hKey);
        return "";
    }

    RegCloseKey(hKey);
    std::string proxy(buffer);
    LOG_F(INFO, "Current proxy server: {}", proxy);
    return proxy;
}

bool NetworkProxy::installWindowsCertificate(
    const std::string& certPath) const {
    LOG_F(INFO, "Installing Windows certificate from path: {}", certPath);
    std::string command = "certutil -addstore -f \"Root\" " + certPath;
    int result = system(command.c_str());
    if (result != 0) {
        LOG_F(ERROR, "Failed to install Windows certificate from path: {}",
              certPath);
        return false;
    }
    LOG_F(INFO, "Windows certificate installed successfully from path: {}",
          certPath);
    return true;
}

bool NetworkProxy::uninstallWindowsCertificate(
    const std::string& certName) const {
    LOG_F(INFO, "Uninstalling Windows certificate: {}", certName);
    std::string command = "certutil -delstore \"Root\" " + certName;
    int result = system(command.c_str());
    if (result != 0) {
        LOG_F(ERROR, "Failed to uninstall Windows certificate: {}", certName);
        return false;
    }
    LOG_F(INFO, "Windows certificate uninstalled successfully: {}", certName);
    return true;
}

std::string NetworkProxy::viewWindowsCertificateInfo(
    const std::string& certName) const {
    LOG_F(INFO, "Viewing Windows certificate info: {}", certName);
    std::string command = "certutil -store \"Root\" " + certName;
    std::string result;
    char buffer[128];
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        LOG_F(ERROR, "popen() failed while viewing certificate info.");
        throw std::runtime_error("popen() failed!");
    }
    try {
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            result += buffer;
        }
    } catch (...) {
        _pclose(pipe);
        LOG_F(ERROR, "Exception occurred while reading certificate info.");
        throw;
    }
    _pclose(pipe);
    LOG_F(INFO, "Certificate info retrieved successfully.");
    return result;
}

void NetworkProxy::editWindowsHostsFile(
    const std::vector<std::pair<std::string, std::string>>& hostsEntries)
    const {
    LOG_F(INFO, "Editing Windows Hosts file with {} entries.",
          hostsEntries.size());
    std::string hostsPath = "C:\\Windows\\System32\\drivers\\etc\\hosts";
    std::ofstream hostsFile(hostsPath, std::ios_base::app);
    if (!hostsFile.is_open()) {
        LOG_F(ERROR, "Failed to open Windows Hosts file for writing: {}",
              hostsPath);
        throw std::runtime_error(
            "Failed to open Windows Hosts file for writing.");
    }
    for (const auto& entry : hostsEntries) {
        hostsFile << entry.first << " " << entry.second << "\n";
    }
    hostsFile.close();
    LOG_F(INFO, "Windows Hosts file edited successfully.");
}

void NetworkProxy::resetWindowsHostsFile() const {
    LOG_F(INFO, "Resetting Windows Hosts file.");
    std::string hostsPath = "C:\\Windows\\System32\\drivers\\etc\\hosts";
    std::ofstream hostsFile(hostsPath, std::ios_base::trunc);
    if (!hostsFile.is_open()) {
        LOG_F(ERROR, "Failed to open Windows Hosts file for resetting: {}",
              hostsPath);
        throw std::runtime_error(
            "Failed to open Windows Hosts file for resetting.");
    }
    // Optionally, add default localhost entry
    hostsFile << "127.0.0.1   localhost\n";
    hostsFile.close();
    LOG_F(INFO, "Windows Hosts file has been reset successfully.");
}

#else
[[nodiscard]] auto NetworkProxy::setLinuxProxy(const std::string& proxy) const
    -> bool {
    LOG_F(INFO, "Setting Linux proxy: {}", proxy);
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
    LOG_F(INFO, "Proxy environment variables set successfully.");

    if (!dns_.empty()) {
        LOG_F(INFO, "Setting custom DNS: {}", dns_);
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
    LOG_F(INFO, "Disabling Linux proxy settings.");
    if (unsetenv("http_proxy") != 0 || unsetenv("https_proxy") != 0 ||
        unsetenv("no_proxy") != 0) {
        LOG_F(ERROR, "Failed to unset proxy environment variables.");
        return false;
    }
    LOG_F(INFO, "Proxy environment variables unset successfully.");
    return true;
}

auto NetworkProxy::getLinuxCurrentProxy() -> std::string {
    LOG_F(INFO, "Retrieving current Linux proxy settings.");
    char* httpProxy = getenv("http_proxy");
    if (httpProxy != nullptr) {
        LOG_F(INFO, "Current http_proxy: {}", std::string(httpProxy));
        return std::string(httpProxy);
    }
    LOG_F(INFO, "No http_proxy set.");
    return "";
}

auto NetworkProxy::installLinuxCertificate(const std::string& certPath)
    -> bool {
    LOG_F(INFO, "Installing Linux certificate from path: {}", certPath);
    std::string command =
        "sudo cp " + certPath +
        " /usr/local/share/ca-certificates/ && sudo update-ca-certificates";
    int result = atom::system::executeCommandWithStatus(command).second;
    if (result != 0) {
        LOG_F(ERROR, "Failed to install Linux certificate from path: {}",
              certPath);
        return false;
    }
    LOG_F(INFO, "Linux certificate installed successfully from path: {}",
          certPath);
    return true;
}

auto NetworkProxy::uninstallLinuxCertificate(const std::string& certName)
    -> bool {
    LOG_F(INFO, "Uninstalling Linux certificate: {}", certName);
    std::string command = "sudo rm /usr/local/share/ca-certificates/" +
                          certName + " && sudo update-ca-certificates --fresh";
    int result = atom::system::executeCommandWithStatus(command).second;
    if (result != 0) {
        LOG_F(ERROR, "Failed to uninstall Linux certificate: {}", certName);
        return false;
    }
    LOG_F(INFO, "Linux certificate uninstalled successfully: {}", certName);
    return true;
}

auto NetworkProxy::viewLinuxCertificateInfo(const std::string& certName)
    -> std::string {
    LOG_F(INFO, "Viewing Linux certificate info: {}", certName);
    std::string command = "openssl x509 -in /usr/local/share/ca-certificates/" +
                          certName + " -text -noout";
    std::string result = atom::system::executeCommand(command);
    LOG_F(INFO, "Certificate info retrieved successfully.");
    return result;
}

void NetworkProxy::editLinuxHostsFile(
    const std::vector<std::pair<std::string, std::string>>& hostsEntries)
    const {
    LOG_F(INFO, "Editing Linux Hosts file with {} entries.",
          hostsEntries.size());
    std::string hostsPath = "/etc/hosts";
    std::ofstream hostsFile(hostsPath, std::ios_base::app);
    if (!hostsFile.is_open()) {
        LOG_F(ERROR, "Failed to open Linux Hosts file for writing: {}",
              hostsPath);
        throw std::runtime_error(
            "Failed to open Linux Hosts file for writing.");
    }
    for (const auto& entry : hostsEntries) {
        hostsFile << entry.first << " " << entry.second << "\n";
    }
    hostsFile.close();
    LOG_F(INFO, "Linux Hosts file edited successfully.");
}

void NetworkProxy::resetLinuxHostsFile() const {
    LOG_F(INFO, "Resetting Linux Hosts file.");
    std::string hostsPath = "/etc/hosts";
    std::ofstream hostsFile(hostsPath, std::ios_base::trunc);
    if (!hostsFile.is_open()) {
        LOG_F(ERROR, "Failed to open Linux Hosts file for resetting: {}",
              hostsPath);
        throw std::runtime_error(
            "Failed to open Linux Hosts file for resetting.");
    }
    // Optionally, add default localhost entry
    hostsFile << "127.0.0.1   localhost\n";
    hostsFile.close();
    LOG_F(INFO, "Linux Hosts file has been reset successfully.");
}
#endif

}  // namespace lithium::cxxtools