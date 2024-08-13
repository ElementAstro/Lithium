#ifndef LITHIUM_CXXTOOLS_PROXY_HPP
#define LITHIUM_CXXTOOLS_PROXY_HPP

#include <ctime>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <cstdlib>
#endif

namespace lithium::cxxtools {
class NetworkProxy {
public:
    enum class ProxyMode { Hosts, PAC, System };

    NetworkProxy() = default;

    auto setProxy(const std::string& proxy, ProxyMode mode = ProxyMode::System,
                  const std::string& listenIP = "0.0.0.0",
                  const std::string& dns = "") -> bool;

    [[nodiscard]] auto disableProxy() const -> bool;

    [[nodiscard]] static auto getCurrentProxy() -> std::string;

    [[nodiscard]] auto installCertificate(const std::string& certPath) const
        -> bool;

    [[nodiscard]] auto uninstallCertificate(const std::string& certName) const
        -> bool;

    [[nodiscard]] auto viewCertificateInfo(const std::string& certName) const
        -> std::string;

    void editHostsFile(
        const std::vector<std::pair<std::string, std::string>>& hostsEntries);

    void resetHostsFile();

    void enableHttpToHttpsRedirect(bool enable);

    void setCustomDoH(const std::string& dohUrl);

private:
    ProxyMode proxyMode_ = ProxyMode::System;
    std::string listenIP_ = "0.0.0.0";
    std::string dns_;
    std::string dohUrl_;
    bool httpToHttpsRedirect_ = false;

    static auto getProxyModeName(ProxyMode mode) -> std::string;

#ifdef _WIN32
    bool setWindowsProxy(const std::string& proxy) const;

    bool disableWindowsProxy() const;

    std::string getWindowsCurrentProxy() const;

    bool installWindowsCertificate(const std::string& certPath) const;

    bool uninstallWindowsCertificate(const std::string& certName) const;

    std::string viewWindowsCertificateInfo(const std::string& certName) const;

    void editWindowsHostsFile(
        const std::vector<std::pair<std::string, std::string>>& hostsEntries)
        const;

    void resetWindowsHostsFile() const;

#else
    [[nodiscard]] auto setLinuxProxy(const std::string& proxy) const -> bool;

    static auto disableLinuxProxy() -> bool;

    static auto getLinuxCurrentProxy() -> std::string;

    static auto installLinuxCertificate(const std::string& certPath) -> bool;

    static auto uninstallLinuxCertificate(const std::string& certName) -> bool;

    static auto viewLinuxCertificateInfo(const std::string& certName)
        -> std::string;

    void editLinuxHostsFile(
        const std::vector<std::pair<std::string, std::string>>& hostsEntries)
        const;

    void resetLinuxHostsFile() const;
#endif
};
}  // namespace lithium::cxxtools

#endif