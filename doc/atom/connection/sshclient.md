# SSHClient Class Documentation

The `SSHClient` class is part of the `atom::connection` namespace and provides functionality for SSH client connections and file operations. This class allows users to connect to SSH servers, execute commands, and perform various file operations on remote servers.

## Table of Contents

1. [Class Overview](#class-overview)
2. [Constructor and Destructor](#constructor-and-destructor)
3. [Public Methods](#public-methods)
   - [connect](#connect)
   - [isConnected](#isconnected)
   - [disconnect](#disconnect)
   - [executeCommand](#executecommand)
   - [executeCommands](#executecommands)
   - [fileExists](#fileexists)
   - [createDirectory](#createdirectory)
   - [removeFile](#removefile)
   - [removeDirectory](#removedirectory)
   - [listDirectory](#listdirectory)
   - [rename](#rename)
   - [getFileInfo](#getfileinfo)
   - [downloadFile](#downloadfile)
   - [uploadFile](#uploadfile)
   - [uploadDirectory](#uploaddirectory)
4. [Usage Examples](#usage-examples)

## Class Overview

```cpp
namespace atom::connection {

class SSHClient {
public:
    explicit SSHClient(const std::string &host, int port = DEFAULT_SSH_PORT);
    ~SSHClient();

    // ... (public methods)

private:
    std::string host_;
    int port_;
    ssh_session ssh_session_;
    sftp_session sftp_session_;
};

}  // namespace atom::connection
```

## Constructor and Destructor

### Constructor

```cpp
explicit SSHClient(const std::string &host, int port = DEFAULT_SSH_PORT);
```

Creates a new `SSHClient` instance.

- **Parameters:**
  - `host`: The hostname or IP address of the SSH server.
  - `port`: The port number of the SSH server (default is 22).

### Destructor

```cpp
~SSHClient();
```

Destroys the `SSHClient` instance and cleans up resources.

## Public Methods

### connect

```cpp
void connect(const std::string &username, const std::string &password, int timeout = DEFAULT_TIMEOUT);
```

Connects to the SSH server.

- **Parameters:**
  - `username`: The username for authentication.
  - `password`: The password for authentication.
  - `timeout`: The connection timeout in seconds (default is 10 seconds).
- **Throws:** `std::runtime_error` if connection or authentication fails.

### isConnected

```cpp
[[nodiscard]] auto isConnected() const -> bool;
```

Checks if the SSH client is connected to the server.

- **Returns:** `true` if connected, `false` otherwise.

### disconnect

```cpp
void disconnect();
```

Disconnects from the SSH server.

### executeCommand

```cpp
void executeCommand(const std::string &command, std::vector<std::string> &output);
```

Executes a single command on the SSH server.

- **Parameters:**
  - `command`: The command to execute.
  - `output`: Output vector to store the command output.
- **Throws:** `std::runtime_error` if command execution fails.

### executeCommands

```cpp
void executeCommands(const std::vector<std::string> &commands, std::vector<std::vector<std::string>> &output);
```

Executes multiple commands on the SSH server.

- **Parameters:**
  - `commands`: Vector of commands to execute.
  - `output`: Vector of vectors to store the command outputs.
- **Throws:** `std::runtime_error` if any command execution fails.

### fileExists

```cpp
[[nodiscard]] auto fileExists(const std::string &remote_path) const -> bool;
```

Checks if a file exists on the remote server.

- **Parameters:**
  - `remote_path`: The path of the remote file.
- **Returns:** `true` if the file exists, `false` otherwise.

### createDirectory

```cpp
void createDirectory(const std::string &remote_path, int mode = DEFAULT_MODE);
```

Creates a directory on the remote server.

- **Parameters:**
  - `remote_path`: The path of the remote directory.
  - `mode`: The permissions of the directory (default is `S_NORMAL`).
- **Throws:** `std::runtime_error` if directory creation fails.

### removeFile

```cpp
void removeFile(const std::string &remote_path);
```

Removes a file from the remote server.

- **Parameters:**
  - `remote_path`: The path of the remote file.
- **Throws:** `std::runtime_error` if file removal fails.

### removeDirectory

```cpp
void removeDirectory(const std::string &remote_path);
```

Removes a directory from the remote server.

- **Parameters:**
  - `remote_path`: The path of the remote directory.
- **Throws:** `std::runtime_error` if directory removal fails.

### listDirectory

```cpp
auto listDirectory(const std::string &remote_path) const -> std::vector<std::string>;
```

Lists the contents of a directory on the remote server.

- **Parameters:**
  - `remote_path`: The path of the remote directory.
- **Returns:** Vector of strings containing the names of the directory contents.
- **Throws:** `std::runtime_error` if listing directory fails.

### rename

```cpp
void rename(const std::string &old_path, const std::string &new_path);
```

Renames a file or directory on the remote server.

- **Parameters:**
  - `old_path`: The current path of the remote file or directory.
  - `new_path`: The new path of the remote file or directory.
- **Throws:** `std::runtime_error` if renaming fails.

### getFileInfo

```cpp
void getFileInfo(const std::string &remote_path, sftp_attributes &attrs);
```

Retrieves file information for a remote file.

- **Parameters:**
  - `remote_path`: The path of the remote file.
  - `attrs`: Attribute struct to store the file information.
- **Throws:** `std::runtime_error` if getting file information fails.

### downloadFile

```cpp
void downloadFile(const std::string &remote_path, const std::string &local_path);
```

Downloads a file from the remote server.

- **Parameters:**
  - `remote_path`: The path of the remote file.
  - `local_path`: The path of the local destination file.
- **Throws:** `std::runtime_error` if file download fails.

### uploadFile

```cpp
void uploadFile(const std::string &local_path, const std::string &remote_path);
```

Uploads a file to the remote server.

- **Parameters:**
  - `local_path`: The path of the local source file.
  - `remote_path`: The path of the remote destination file.
- **Throws:** `std::runtime_error` if file upload fails.

### uploadDirectory

```cpp
void uploadDirectory(const std::string &local_path, const std::string &remote_path);
```

Uploads a directory to the remote server.

- **Parameters:**
  - `local_path`: The path of the local source directory.
  - `remote_path`: The path of the remote destination directory.
- **Throws:** `std::runtime_error` if directory upload fails.

## Usage Examples

Here are some examples demonstrating how to use the `SSHClient` class:

### Connecting to an SSH Server and Executing a Command

```cpp
#include "sshclient.hpp"
#include <iostream>

int main() {
    try {
        atom::connection::SSHClient client("example.com");
        client.connect("username", "password");

        if (client.isConnected()) {
            // Upload a file
            client.uploadFile("/path/to/local/file.txt", "/remote/path/file.txt");
            std::cout << "File uploaded successfully" << std::endl;

            // Download a file
            client.downloadFile("/remote/path/downloaded_file.txt", "/path/to/local/downloaded_file.txt");
            std::cout << "File downloaded successfully" << std::endl;

            client.disconnect();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

### Managing Remote Directories

```cpp
#include "sshclient.hpp"
#include <iostream>
#include <vector>

int main() {
    try {
        atom::connection::SSHClient client("example.com");
        client.connect("username", "password");

        if (client.isConnected()) {
            // Create a new directory
            client.createDirectory("/remote/path/new_directory");
            std::cout << "Directory created successfully" << std::endl;

            // List directory contents
            std::vector<std::string> contents = client.listDirectory("/remote/path");
            std::cout << "Directory contents:" << std::endl;
            for (const auto& item : contents) {
                std::cout << item << std::endl;
            }

            // Rename a directory
            client.rename("/remote/path/old_name", "/remote/path/new_name");
            std::cout << "Directory renamed successfully" << std::endl;

            // Remove a directory
            client.removeDirectory("/remote/path/to_be_removed");
            std::cout << "Directory removed successfully" << std::endl;

            client.disconnect();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

### Executing Multiple Commands

```cpp
#include "sshclient.hpp"
#include <iostream>
#include <vector>

int main() {
    try {
        atom::connection::SSHClient client("example.com");
        client.connect("username", "password");

        if (client.isConnected()) {
            std::vector<std::string> commands = {
                "echo 'Hello, World!'",
                "ls -l /home",
                "df -h"
            };

            std::vector<std::vector<std::string>> outputs;
            client.executeCommands(commands, outputs);

            for (size_t i = 0; i < commands.size(); ++i) {
                std::cout << "Output of command '" << commands[i] << "':" << std::endl;
                for (const auto& line : outputs[i]) {
                    std::cout << line << std::endl;
                }
                std::cout << std::endl;
            }

            client.disconnect();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

### Checking File Existence and Getting File Information

```cpp
#include "sshclient.hpp"
#include <iostream>
#include <iomanip>

int main() {
    try {
        atom::connection::SSHClient client("example.com");
        client.connect("username", "password");

        if (client.isConnected()) {
            std::string remote_file = "/remote/path/file.txt";

            if (client.fileExists(remote_file)) {
                std::cout << "File exists: " << remote_file << std::endl;

                sftp_attributes attrs;
                client.getFileInfo(remote_file, attrs);

                std::cout << "File information:" << std::endl;
                std::cout << "Size: " << attrs.size << " bytes" << std::endl;
                std::cout << "Owner: " << attrs.owner << std::endl;
                std::cout << "Group: " << attrs.group << std::endl;
                std::cout << "Permissions: " << std::setfill('0') << std::setw(4) << std::oct << attrs.permissions << std::endl;
                std::cout << "Last access time: " << attrs.atime << std::endl;
                std::cout << "Last modification time: " << attrs.mtime << std::endl;

                sftp_attributes_free(attrs);
            } else {
                std::cout << "File does not exist: " << remote_file << std::endl;
            }

            client.disconnect();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

### Uploading a Directory

```cpp
#include "sshclient.hpp"
#include <iostream>

int main() {
    try {
        atom::connection::SSHClient client("example.com");
        client.connect("username", "password");

        if (client.isConnected()) {
            std::string local_dir = "/path/to/local/directory";
            std::string remote_dir = "/remote/path/directory";

            client.uploadDirectory(local_dir, remote_dir);
            std::cout << "Directory uploaded successfully" << std::endl;

            client.disconnect();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

These examples demonstrate various use cases of the `SSHClient` class, including file and directory operations, command execution, and retrieving file information. Remember to handle exceptions appropriately in your applications and ensure proper error handling and resource management.

When using the `SSHClient` class in your projects, make sure to:

1. Include proper error handling and logging.
2. Use secure practices for storing and handling credentials.
3. Implement appropriate timeout mechanisms for network operations.
4. Consider using key-based authentication instead of password authentication for improved security.
5. Be mindful of the permissions and ownership of files and directories you create or modify on the remote server.

By following these best practices and utilizing the `SSHClient` class effectively, you can build robust applications that interact with remote servers securely and efficiently.
