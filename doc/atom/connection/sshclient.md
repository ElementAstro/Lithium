# SSHClient Class Documentation

## Overview

The `SSHClient` class provides functionality for SSH client connection and various file operations on a remote server.

### Class Definition

```cpp
class SSHClient {
public:
    explicit SSHClient(const std::string &host, int port = 22);
    ~SSHClient();

    void Connect(const std::string &username, const std::string &password, int timeout = 10);
    bool IsConnected();
    void Disconnect();
    void ExecuteCommand(const std::string &command, std::vector<std::string> &output);
    void ExecuteCommands(const std::vector<std::string> &commands, std::vector<std::vector<std::string>> &output);
    bool FileExists(const std::string &remote_path);
    void CreateDirectory(const std::string &remote_path, int mode = S_NORMAL);
    void RemoveFile(const std::string &remote_path);
    void RemoveDirectory(const std::string &remote_path);
    std::vector<std::string> ListDirectory(const std::string &remote_path);
    void Rename(const std::string &old_path, const std::string &new_path);
    void GetFileInfo(const std::string &remote_path, sftp_attributes &attrs);
    void DownloadFile(const std::string &remote_path, const std::string &local_path);
    void UploadFile(const std::string &local_path, const std::string &remote_path);

private:
    std::string m_host;
    int m_port;
    ssh_session m_ssh_session;
    sftp_session m_sftp_session;
};
```

## Constructor

### Description

Creates an instance of the `SSHClient` class with the specified host and port.

### Example

```cpp
SSHClient ssh("example.com", 22);
```

## Connect

### Description

Connects to the SSH server using provided credentials.

### Example

```cpp
ssh.Connect("username", "password");
```

## ExecuteCommand

### Description

Executes a single command on the SSH server and retrieves the output.

### Example

```cpp
std::vector<std::string> output;
ssh.ExecuteCommand("ls -l", output);
```

## ListDirectory

### Description

Lists the contents of a directory on the remote server.

### Example

```cpp
std::vector<std::string> contents = ssh.ListDirectory("/path/to/directory");
```

## DownloadFile

### Description

Downloads a file from the remote server to a local destination.

### Example

```cpp
ssh.DownloadFile("/path/to/remote/file.txt", "/path/to/local/file.txt");
```

## UploadFile

### Description

Uploads a local file to the remote server.

### Example

```cpp
ssh.UploadFile("/path/to/local/file.txt", "/path/to/remote/file.txt");
```
