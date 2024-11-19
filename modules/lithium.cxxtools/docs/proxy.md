# NetworkProxy Class Documentation

## Overview

The `NetworkProxy` class is designed to manage network proxy settings on both Windows and Linux operating systems. It provides functionalities to set, disable, and retrieve proxy settings, install and uninstall certificates, and manage the hosts file. The class is part of the `lithium::cxxtools` namespace and leverages platform-specific implementations to handle proxy-related operations.

## Class Methods

### `setProxy(const std::string& proxy, NetworkProxy::ProxyMode mode, const std::string& listenIP, const std::string& dns) -> bool`

Sets the network proxy with the specified parameters.

- **Parameters:**

  - `proxy`: The proxy server address.
  - `mode`: The proxy mode (e.g., Hosts, PAC, System).
  - `listenIP`: The IP address to listen on.
  - `dns`: The DNS server address.

- **Returns:** `true` if the proxy was set successfully, `false` otherwise.

### `disableProxy() const -> bool`

Disables the network proxy.

- **Returns:** `true` if the proxy was disabled successfully, `false` otherwise.

### `getCurrentProxy() -> std::string`

Retrieves the current proxy settings.

- **Returns:** A string representing the current proxy server address.

### `installCertificate(const std::string& certPath) const -> bool`

Installs a certificate from the specified path.

- **Parameters:**

  - `certPath`: The path to the certificate file.

- **Returns:** `true` if the certificate was installed successfully, `false` otherwise.

### `uninstallCertificate(const std::string& certName) const -> bool`

Uninstalls a certificate by its name.

- **Parameters:**

  - `certName`: The name of the certificate to uninstall.

- **Returns:** `true` if the certificate was uninstalled successfully, `false` otherwise.

### `viewCertificateInfo(const std::string& certName) const -> std::string`

Retrieves information about a certificate by its name.

- **Parameters:**

  - `certName`: The name of the certificate.

- **Returns:** A string containing the certificate information.

### `editHostsFile(const std::vector<std::pair<std::string, std::string>>& hostsEntries)`

Edits the hosts file with the specified entries.

- **Parameters:**
  - `hostsEntries`: A vector of pairs where each pair represents an IP address and a hostname.

### `resetHostsFile()`

Resets the hosts file to its default state.

### `enableHttpToHttpsRedirect(bool enable)`

Enables or disables HTTP to HTTPS redirection.

- **Parameters:**
  - `enable`: `true` to enable redirection, `false` to disable.

### `setCustomDoH(const std::string& dohUrl)`

Sets a custom DNS-over-HTTPS (DoH) URL.

- **Parameters:**
  - `dohUrl`: The DoH URL to set.

### `getProxyModeName(ProxyMode mode) -> std::string`

Returns the name of the specified proxy mode.

- **Parameters:**

  - `mode`: The proxy mode.

- **Returns:** A string representing the name of the proxy mode.

## Platform-Specific Implementations

### Windows

#### `setWindowsProxy(const std::string& proxy) const -> bool`

Sets the proxy settings on a Windows system using the Windows Registry.

#### `disableWindowsProxy() const -> bool`

Disables the proxy settings on a Windows system using the Windows Registry.

#### `getWindowsCurrentProxy() const -> std::string`

Retrieves the current proxy settings from the Windows Registry.

#### `installWindowsCertificate(const std::string& certPath) const -> bool`

Installs a certificate on a Windows system using the `certutil` command.

#### `uninstallWindowsCertificate(const std::string& certName) const -> bool`

Uninstalls a certificate on a Windows system using the `certutil` command.

#### `viewWindowsCertificateInfo(const std::string& certName) const -> std::string`

Retrieves information about a certificate on a Windows system using the `certutil` command.

#### `editWindowsHostsFile(const std::vector<std::pair<std::string, std::string>>& hostsEntries) const`

Edits the Windows hosts file with the specified entries.

#### `resetWindowsHostsFile() const`

Resets the Windows hosts file to its default state.

### Linux

#### `setLinuxProxy(const std::string& proxy) const -> bool`

Sets the proxy settings on a Linux system using environment variables.

#### `disableLinuxProxy() -> bool`

Disables the proxy settings on a Linux system by unsetting environment variables.

#### `getLinuxCurrentProxy() -> std::string`

Retrieves the current proxy settings from the Linux environment variables.

#### `installLinuxCertificate(const std::string& certPath) -> bool`

Installs a certificate on a Linux system using the `update-ca-certificates` command.

#### `uninstallLinuxCertificate(const std::string& certName) -> bool`

Uninstalls a certificate on a Linux system using the `update-ca-certificates` command.

#### `viewLinuxCertificateInfo(const std::string& certName) -> std::string`

Retrieves information about a certificate on a Linux system using the `openssl` command.

#### `editLinuxHostsFile(const std::vector<std::pair<std::string, std::string>>& hostsEntries) const`

Edits the Linux hosts file with the specified entries.

#### `resetLinuxHostsFile() const`

Resets the Linux hosts file to its default state.

## Dependencies

- **loguru**: A logging library used for logging operations.
- **atom/system/command.hpp**: A utility for executing system commands.

## Usage Example

```cpp
#include "proxy.hpp"

int main() {
    lithium::cxxtools::NetworkProxy proxy;

    // Set proxy
    proxy.setProxy("http://proxy.example.com:8080", lithium::cxxtools::NetworkProxy::ProxyMode::System, "0.0.0.0", "8.8.8.8");

    // Disable proxy
    proxy.disableProxy();

    // Install certificate
    proxy.installCertificate("path/to/certificate.crt");

    // Uninstall certificate
    proxy.uninstallCertificate("certificate.crt");

    // Edit hosts file
    std::vector<std::pair<std::string, std::string>> hostsEntries = {
        {"127.0.0.1", "localhost"},
        {"192.168.1.1", "example.com"}
    };
    proxy.editHostsFile(hostsEntries);

    // Reset hosts file
    proxy.resetHostsFile();

    return 0;
}
```

## Notes

- The class uses platform-specific code to handle proxy settings, certificate management, and hosts file operations.
- Logging is used extensively to provide detailed information about the operations being performed.
- The class is designed to be cross-platform, with separate implementations for Windows and Linux.
