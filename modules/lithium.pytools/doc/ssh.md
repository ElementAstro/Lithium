# SSH Client Tool Documentation

This document provides a comprehensive guide on how to use the **SSH Client Tool**, a Python script designed for managing SSH connections and executing commands on remote servers. The tool supports various functionalities, including command execution, file uploads and downloads, directory management, and more.

---

## Key Features

- **SSH Connection Management**: Establishes secure connections to remote servers using SSH.
- **Command Execution**: Execute commands on the remote server and retrieve output.
- **File Transfer**: Upload and download files using SFTP.
- **Directory Management**: Create and list directories on the remote server.
- **Error Handling**: Robust exception handling with detailed logging using Loguru.
- **Flexible Authentication**: Supports password-based and key-based authentication.

---

## Requirements

- Python 3.7 or higher.
- Required libraries: `paramiko`, `loguru`.

Install the required libraries using pip:

```bash
pip install paramiko loguru
```

---

## Usage

The script can be executed from the command line with various options. The command syntax is as follows:

```bash
python ssh_helper.py --hostname <hostname> --port <port> --username <username> [options]
```

### Command-Line Options

- **`--hostname`**: Required. Hostname or IP address of the remote server.
- **`--port`**: Optional. Port number for SSH (default: `22`).
- **`--username`**: Required. Username for SSH login.
- **`--password`**: Optional. Password for SSH (if using password authentication).
- **`--key_file`**: Optional. Path to the private key file (if using key authentication).

### Subcommands

The following subcommands are available:

1. **Execute a Command**

   ```bash
   python ssh_helper.py --hostname <hostname> --username <username> --password <password> exec <command> [--timeout <timeout>]
   ```

   - **`<command>`**: The command to execute on the remote server.
   - **`--timeout`**: Optional. Timeout for command execution.

2. **Upload a File**

   ```bash
   python ssh_helper.py --hostname <hostname> --username <username> --password <password> upload <local_path> <remote_path>
   ```

   - **`<local_path>`**: Path to the local file to upload.
   - **`<remote_path>`**: Path on the remote server where the file will be uploaded.

3. **Download a File**

   ```bash
   python ssh_helper.py --hostname <hostname> --username <username> --password <password> download <remote_path> <local_path>
   ```

   - **`<remote_path>`**: Path on the remote server to download.
   - **`<local_path>`**: Path on the local machine where the file will be saved.

4. **List Directory Contents**

   ```bash
   python ssh_helper.py --hostname <hostname> --username <username> --password <password> list <remote_path>
   ```

   - **`<remote_path>`**: Path on the remote server to list.

5. **Create a Directory**

   ```bash
   python ssh_helper.py --hostname <hostname> --username <username> --password <password> mkdir <remote_path>
   ```

   - **`<remote_path>`**: Path on the remote server where the directory will be created.

6. **Delete a File**

   ```bash
   python ssh_helper.py --hostname <hostname> --username <username> --password <password> delete <remote_path>
   ```

   - **`<remote_path>`**: Path on the remote server of the file to delete.

---

## Example Usage

### Execute a Command

To execute a command on the remote server:

```bash
python ssh_helper.py --hostname example.com --username user --password pass exec "ls -la"
```

### Upload a File

To upload a file to the remote server:

```bash
python ssh_helper.py --hostname example.com --username user --password pass upload /path/to/local/file.txt /path/to/remote/file.txt
```

### Download a File

To download a file from the remote server:

```bash
python ssh_helper.py --hostname example.com --username user --password pass download /path/to/remote/file.txt /path/to/local/file.txt
```

### List Directory Contents

To list the contents of a directory on the remote server:

```bash
python ssh_helper.py --hostname example.com --username user --password pass list /path/to/remote/directory
```

### Create a Directory

To create a directory on the remote server:

```bash
python ssh_helper.py --hostname example.com --username user --password pass mkdir /path/to/remote/new_directory
```

### Delete a File

To delete a file on the remote server:

```bash
python ssh_helper.py --hostname example.com --username user --password pass delete /path/to/remote/file.txt
```

---

## Error Handling and Logging

The script uses the `Loguru` library for logging. Logs are written to the console and provide detailed information about operations, warnings, and errors. This helps in tracking the actions performed by the script and diagnosing issues.

---

## Conclusion

The **SSH Client Tool** is a versatile utility for managing SSH connections and executing commands on remote servers. It simplifies the process of file transfer, command execution, and directory management while providing robust error handling and logging capabilities. By following this documentation, users can effectively utilize the tool for their SSH management needs.
