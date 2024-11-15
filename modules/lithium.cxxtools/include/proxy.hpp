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

/**
 * @brief Class for managing network proxy settings and certificates.
 *
 * This class provides functionality to set and disable proxy settings,
 * install and uninstall certificates, edit the hosts file, and manage
 * HTTP to HTTPS redirection and custom DNS over HTTPS (DoH) settings.
 */
class NetworkProxy {
public:
    /**
     * @brief Enumeration for proxy modes.
     */
    enum class ProxyMode { Hosts, PAC, System };

    /**
     * @brief Default constructor.
     */
    NetworkProxy() = default;

    /**
     * @brief Sets the network proxy.
     *
     * @param proxy The proxy address to set.
     * @param mode The proxy mode to use (default is System).
     * @param listenIP The IP address to listen on (default is "0.0.0.0").
     * @param dns The custom DNS to use (optional).
     * @return bool True if the proxy was successfully set, false otherwise.
     */
    auto setProxy(const std::string& proxy, ProxyMode mode = ProxyMode::System,
                  const std::string& listenIP = "0.0.0.0",
                  const std::string& dns = "") -> bool;

    /**
     * @brief Disables the network proxy.
     *
     * @return bool True if the proxy was successfully disabled, false
     * otherwise.
     */
    [[nodiscard]] auto disableProxy() const -> bool;

    /**
     * @brief Retrieves the current proxy settings.
     *
     * @return std::string The current proxy settings.
     */
    [[nodiscard]] static auto getCurrentProxy() -> std::string;

    /**
     * @brief Installs a certificate.
     *
     * @param certPath The path to the certificate file.
     * @return bool True if the certificate was successfully installed, false
     * otherwise.
     */
    [[nodiscard]] auto installCertificate(const std::string& certPath) const
        -> bool;

    /**
     * @brief Uninstalls a certificate.
     *
     * @param certName The name of the certificate to uninstall.
     * @return bool True if the certificate was successfully uninstalled, false
     * otherwise.
     */
    [[nodiscard]] auto uninstallCertificate(const std::string& certName) const
        -> bool;

    /**
     * @brief Views information about a certificate.
     *
     * @param certName The name of the certificate to view.
     * @return std::string The certificate information.
     */
    [[nodiscard]] auto viewCertificateInfo(const std::string& certName) const
        -> std::string;

    /**
     * @brief Edits the hosts file with the specified entries.
     *
     * @param hostsEntries A vector of pairs representing the hosts entries to
     * add.
     */
    void editHostsFile(
        const std::vector<std::pair<std::string, std::string>>& hostsEntries);

    /**
     * @brief Resets the hosts file to its default state.
     */
    void resetHostsFile();

    /**
     * @brief Enables or disables HTTP to HTTPS redirection.
     *
     * @param enable True to enable redirection, false to disable.
     */
    void enableHttpToHttpsRedirect(bool enable);

    /**
     * @brief Sets a custom DNS over HTTPS (DoH) URL.
     *
     * @param dohUrl The DoH URL to set.
     */
    void setCustomDoH(const std::string& dohUrl);

private:
    ProxyMode proxyMode_ = ProxyMode::System;  ///< The current proxy mode.
    std::string listenIP_ = "0.0.0.0";         ///< The IP address to listen on.
    std::string dns_;                          ///< The custom DNS to use.
    std::string dohUrl_;                       ///< The custom DoH URL.
    bool httpToHttpsRedirect_ =
        false;  ///< Flag to enable/disable HTTP to HTTPS redirection.

    /**
     * @brief Gets the name of the proxy mode.
     *
     * @param mode The proxy mode.
     * @return std::string The name of the proxy mode.
     */
    static auto getProxyModeName(ProxyMode mode) -> std::string;

#ifdef _WIN32
    /**
     * @brief Sets the Windows proxy settings.
     *
     * @param proxy The proxy address to set.
     * @return bool True if the proxy was successfully set, false otherwise.
     */
    bool setWindowsProxy(const std::string& proxy) const;

    /**
     * @brief Disables the Windows proxy settings.
     *
     * @return bool True if the proxy was successfully disabled, false
     * otherwise.
     */
    bool disableWindowsProxy() const;

    /**
     * @brief Retrieves the current Windows proxy settings.
     *
     * @return std::string The current Windows proxy settings.
     */
    std::string getWindowsCurrentProxy() const;

    /**
     * @brief Installs a Windows certificate.
     *
     * @param certPath The path to the certificate file.
     * @return bool True if the certificate was successfully installed, false
     * otherwise.
     */
    bool installWindowsCertificate(const std::string& certPath) const;

    /**
     * @brief Uninstalls a Windows certificate.
     *
     * @param certName The name of the certificate to uninstall.
     * @return bool True if the certificate was successfully uninstalled, false
     * otherwise.
     */
    bool uninstallWindowsCertificate(const std::string& certName) const;

    /**
     * @brief Views information about a Windows certificate.
     *
     * @param certName The name of the certificate to view.
     * @return std::string The certificate information.
     */
    std::string viewWindowsCertificateInfo(const std::string& certName) const;

    /**
     * @brief Edits the Windows hosts file with the specified entries.
     *
     * @param hostsEntries A vector of pairs representing the hosts entries to
     * add.
     */
    void editWindowsHostsFile(
        const std::vector<std::pair<std::string, std::string>>& hostsEntries)
        const;

    /**
     * @brief Resets the Windows hosts file to its default state.
     */
    void resetWindowsHostsFile() const;

#else
    /**
     * @brief Sets the Linux proxy settings.
     *
     * @param proxy The proxy address to set.
     * @return bool True if the proxy was successfully set, false otherwise.
     */
    [[nodiscard]] auto setLinuxProxy(const std::string& proxy) const -> bool;

    /**
     * @brief Disables the Linux proxy settings.
     *
     * @return bool True if the proxy was successfully disabled, false
     * otherwise.
     */
    static auto disableLinuxProxy() -> bool;

    /**
     * @brief Retrieves the current Linux proxy settings.
     *
     * @return std::string The current Linux proxy settings.
     */
    static auto getLinuxCurrentProxy() -> std::string;

    /**
     * @brief Installs a Linux certificate.
     *
     * @param certPath The path to the certificate file.
     * @return bool True if the certificate was successfully installed, false
     * otherwise.
     */
    static auto installLinuxCertificate(const std::string& certPath) -> bool;

    /**
     * @brief Uninstalls a Linux certificate.
     *
     * @param certName The name of the certificate to uninstall.
     * @return bool True if the certificate was successfully uninstalled, false
     * otherwise.
     */
    static auto uninstallLinuxCertificate(const std::string& certName) -> bool;

    /**
     * @brief Views information about a Linux certificate.
     *
     * @param certName The name of the certificate to view.
     * @return std::string The certificate information.
     */
    static auto viewLinuxCertificateInfo(const std::string& certName)
        -> std::string;

    /**
     * @brief Edits the Linux hosts file with the specified entries.
     *
     * @param hostsEntries A vector of pairs representing the hosts entries to
     * add.
     */
    void editLinuxHostsFile(
        const std::vector<std::pair<std::string, std::string>>& hostsEntries)
        const;

    /**
     * @brief Resets the Linux hosts file to its default state.
     */
    void resetLinuxHostsFile() const;
#endif
};

}  // namespace lithium::cxxtools

#endif  // LITHIUM_CXXTOOLS_PROXY_HPP