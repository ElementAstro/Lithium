# SshServer Class Documentation

The `SshServer` class is part of the `atom::connection` namespace and provides functionality for setting up and managing an SSH server. This class allows users to configure various aspects of an SSH server, including port, listening address, authentication methods, and subsystems.

## Table of Contents

1. [Class Overview](#class-overview)
2. [Constructor and Destructor](#constructor-and-destructor)
3. [Public Methods](#public-methods)
   - [start](#start)
   - [stop](#stop)
   - [isRunning](#isrunning)
   - [setPort](#setport)
   - [getPort](#getport)
   - [setListenAddress](#setlistenaddress)
   - [getListenAddress](#getlistenaddress)
   - [setHostKey](#sethostkey)
   - [getHostKey](#gethostkey)
   - [setAuthorizedKeys](#setauthorizedkeys)
   - [getAuthorizedKeys](#getauthorizedkeys)
   - [allowRootLogin](#allowrootlogin)
   - [isRootLoginAllowed](#isrootloginallowed)
   - [setPasswordAuthentication](#setpasswordauthentication)
   - [isPasswordAuthenticationEnabled](#ispasswordauthenticationenabled)
   - [setSubsystem](#setsubsystem)
   - [removeSubsystem](#removesubsystem)
   - [getSubsystem](#getsubsystem)
4. [Usage Examples](#usage-examples)

## Class Overview

```cpp
namespace atom::connection {

class SshServer : public NonCopyable {
public:
    explicit SshServer(const std::filesystem::path& configFile);
    ~SshServer() override;

    // ... (public methods)

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace atom::connection
```

## Constructor and Destructor

### Constructor

```cpp
explicit SshServer(const std::filesystem::path& configFile);
```

Creates a new `SshServer` instance.

- **Parameters:**
  - `configFile`: The path to the configuration file for the SSH server.

### Destructor

```cpp
~SshServer() override;
```

Destroys the `SshServer` instance and cleans up resources.

## Public Methods

### start

```cpp
void start();
```

Starts the SSH server, listening for incoming connections on the configured port and address.

### stop

```cpp
void stop();
```

Stops the SSH server, closing all existing connections and stopping new connection acceptance.

### isRunning

```cpp
ATOM_NODISCARD auto isRunning() const -> bool;
```

Checks if the SSH server is currently running.

- **Returns:** `true` if the server is running, `false` otherwise.

### setPort

```cpp
void setPort(int port);
```

Sets the port on which the SSH server listens for connections.

- **Parameters:**
  - `port`: The port number to listen on.

### getPort

```cpp
ATOM_NODISCARD auto getPort() const -> int;
```

Gets the port on which the SSH server is listening.

- **Returns:** The current listening port.

### setListenAddress

```cpp
void setListenAddress(const std::string& address);
```

Sets the address on which the SSH server listens for connections.

- **Parameters:**
  - `address`: The IP address or hostname for listening.

### getListenAddress

```cpp
ATOM_NODISCARD auto getListenAddress() const -> std::string;
```

Gets the address on which the SSH server is listening.

- **Returns:** The current listening address as a string.

### setHostKey

```cpp
void setHostKey(const std::filesystem::path& keyFile);
```

Sets the host key file used for SSH connections.

- **Parameters:**
  - `keyFile`: The path to the host key file.

### getHostKey

```cpp
ATOM_NODISCARD auto getHostKey() const -> std::filesystem::path;
```

Gets the path to the host key file.

- **Returns:** The current host key file path.

### setAuthorizedKeys

```cpp
void setAuthorizedKeys(const std::vector<std::filesystem::path>& keyFiles);
```

Sets the list of authorized public key files for user authentication.

- **Parameters:**
  - `keyFiles`: A vector of paths to public key files.

### getAuthorizedKeys

```cpp
ATOM_NODISCARD auto getAuthorizedKeys() const -> std::vector<std::filesystem::path>;
```

Gets the list of authorized public key files.

- **Returns:** A vector of paths to authorized public key files.

### allowRootLogin

```cpp
void allowRootLogin(bool allow);
```

Enables or disables root login to the SSH server.

- **Parameters:**
  - `allow`: `true` to permit root login, `false` to deny it.

### isRootLoginAllowed

```cpp
ATOM_NODISCARD auto isRootLoginAllowed() const -> bool;
```

Checks if root login is allowed.

- **Returns:** `true` if root login is permitted, `false` otherwise.

### setPasswordAuthentication

```cpp
void setPasswordAuthentication(bool enable);
```

Enables or disables password authentication for the SSH server.

- **Parameters:**
  - `enable`: `true` to enable password authentication, `false` to disable it.

### isPasswordAuthenticationEnabled

```cpp
ATOM_NODISCARD auto isPasswordAuthenticationEnabled() const -> bool;
```

Checks if password authentication is enabled.

- **Returns:** `true` if password authentication is enabled, `false` otherwise.

### setSubsystem

```cpp
void setSubsystem(const std::string& name, const std::string& command);
```

Sets a subsystem for handling a specific command.

- **Parameters:**
  - `name`: The name of the subsystem.
  - `command`: The command that the subsystem will execute.

### removeSubsystem

```cpp
void removeSubsystem(const std::string& name);
```

Removes a previously set subsystem by name.

- **Parameters:**
  - `name`: The name of the subsystem to remove.

### getSubsystem

```cpp
ATOM_NODISCARD auto getSubsystem(const std::string& name) const -> std::string;
```

Gets the command associated with a subsystem by name.

- **Parameters:**
  - `name`: The name of the subsystem.
- **Returns:** The command associated with the subsystem.

## Usage Examples

Here are some examples demonstrating how to use the `SshServer` class:

### Basic Server Setup and Start

```cpp
#include "sshserver.hpp"
#include <iostream>

int main() {
    try {
        atom::connection::SshServer server("/path/to/config/file.conf");

        server.setPort(2222);
        server.setListenAddress("0.0.0.0");
        server.setHostKey("/path/to/host_key");

        server.start();

        std::cout << "SSH server started on " << server.getListenAddress()
                  << ":" << server.getPort() << std::endl;

        // Keep the server running
        while (server.isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

### Configuring Authentication Methods

```cpp
#include "sshserver.hpp"
#include <iostream>

int main() {
    try {
        atom::connection::SshServer server("/path/to/config/file.conf");

        // Set up SFTP subsystem
        server.setSubsystem("sftp", "/usr/lib/openssh/sftp-server");

        // Set up a custom subsystem
        server.setSubsystem("my-custom-subsystem", "/path/to/custom/script.sh");

        server.start();

        std::cout << "SSH server started with configured subsystems" << std::endl;

        // Later, if we want to remove a subsystem
        server.removeSubsystem("my-custom-subsystem");

        // Keep the server running
        while (server.isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

### Dynamic Configuration Changes

```cpp
#include "sshserver.hpp"
#include <iostream>
#include <thread>

void configurationThread(atom::connection::SshServer& server) {
    while (server.isRunning()) {
        // Periodically update server configuration
        std::this_thread::sleep_for(std::chrono::minutes(5));

        // Change listening port
        server.setPort(2223);

        // Update host key
        server.setHostKey("/path/to/new/host_key");

        // Toggle root login permission
        server.allowRootLogin(!server.isRootLoginAllowed());

        std::cout << "Server configuration updated" << std::endl;
    }
}

int main() {
    try {
        atom::connection::SshServer server("/path/to/config/file.conf");
        server.start();

        std::thread config_thread(configurationThread, std::ref(server));

        // Main thread keeps the server running
        while (server.isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        config_thread.join();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

### Implementing a Simple SSH Server Monitor

```cpp
#include "sshserver.hpp"
#include <iostream>
#include <chrono>
#include <thread>

class SshServerMonitor {
public:
    SshServerMonitor(atom::connection::SshServer& server) : server_(server) {}

    void run() {
        while (true) {
            if (server_.isRunning()) {
                std::cout << "SSH Server Status: Running" << std::endl;
                std::cout << "Listening on: " << server_.getListenAddress() << ":" << server_.getPort() << std::endl;
                std::cout << "Root login: " << (server_.isRootLoginAllowed() ? "Allowed" : "Denied") << std::endl;
                std::cout << "Password auth: " << (server_.isPasswordAuthenticationEnabled() ? "Enabled" : "Disabled") << std::endl;
            } else {
                std::cout << "SSH Server Status: Stopped" << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }

private:
    atom::connection::SshServer& server_;
};

int main() {
    try {
        atom::connection::SshServer server("/path/to/config/file.conf");
        server.start();

        SshServerMonitor monitor(server);
        std::thread monitor_thread(&SshServerMonitor::run, &monitor);

        // Keep the main thread alive
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        monitor_thread.join();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

### Graceful Shutdown Handling

```cpp
#include "sshserver.hpp"
#include <iostream>
#include <csignal>

atom::connection::SshServer* g_server = nullptr;

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";

    if (g_server && g_server->isRunning()) {
        std::cout << "Stopping SSH server gracefully..." << std::endl;
        g_server->stop();
    }

    exit(signum);
}

int main() {
    try {
        atom::connection::SshServer server("/path/to/config/file.conf");
        g_server = &server;

        // Register signal handler
        signal(SIGINT, signalHandler);

        server.start();

        std::cout << "SSH server started. Press Ctrl+C to stop." << std::endl;

        // Keep the server running
        while (server.isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

These examples demonstrate various ways to use and integrate the `SshServer` class in different scenarios, including:

1. Basic server setup and configuration
2. Configuring authentication methods
3. Setting up and managing subsystems
4. Implementing dynamic configuration changes
5. Creating a simple SSH server monitor
6. Handling graceful shutdown with signal interrupts

When using the `SshServer` class in your projects, consider the following best practices:

1. Always use secure and up-to-date host keys.
2. Regularly update and rotate authorized keys for better security.
3. Limit root login access and use principle of least privilege.
4. Implement proper logging and monitoring for security and troubleshooting.
5. Use strong password policies if password authentication is enabled.
6. Regularly update the SSH server software and apply security patches.
7. Implement rate limiting and fail2ban-like mechanisms to prevent brute-force attacks.
8. Use firewalls to restrict SSH access to trusted IP addresses when possible.

By following these practices and leveraging the `SshServer` class effectively, you can build secure and robust SSH server applications tailored to your specific needs.
