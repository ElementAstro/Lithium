# SFTP Client Command Line Tool Documentation

This document provides a comprehensive guide on how to use the **SFTP Client Command Line Tool**, a Python script designed for managing files and directories on an SFTP server. The tool supports various functionalities, including file uploads, downloads, directory management, and more.

---

## Key Features

- **SFTP Connection Management**: Establish secure connections to SFTP servers using SSH.
- **File Transfer**: Upload and download files using SFTP.
- **Directory Management**: Create and remove directories on the SFTP server.
- **File Information Retrieval**: Get details about files and directories on the server.
- **Resume Uploads**: Resume interrupted file uploads.
- **Error Handling**: Robust exception handling with detailed logging using Loguru.

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
python sftp_client.py --hostname <hostname> --username <username> [options]
```

### Command-Line Options

- **`--hostname`**: Required. Hostname or IP address of the SFTP server.
- **`--username`**: Required. Username for SFTP login.
- **`--password`**: Optional. Password for SFTP (if using password authentication).
- **`--port`**: Optional. Port number for SFTP (default: `22`).
- **`--key-file`**: Optional. Path to the private key file (if using key authentication).

### Subcommands

The following subcommands are available:

1. **Upload a Directory**

   ```bash
   python sftp_client.py --hostname <hostname> --username <username> --password <password> upload-dir <local_dir> <remote_dir>
   ```

   - **`<local_dir>`**: Path to the local directory to upload.
   - **`<remote_dir>`**: Path on the remote server where the directory will be uploaded.

2. **Download a Directory**

   ```bash
   python sftp_client.py --hostname <hostname> --username <username> --password <password> download-dir <remote_dir> <local_dir>
   ```

   - **`<remote_dir>`**: Path on the remote server to download.
   - **`<local_dir>`**: Path on the local machine where the directory will be saved.

3. **Create a Directory**

   ```bash
   python sftp_client.py --hostname <hostname> --username <username> --password <password> mkdir <remote_path>
   ```

   - **`<remote_path>`**: Path on the remote server where the directory will be created.

4. **Remove a Directory**

   ```bash
   python sftp_client.py --hostname <hostname> --username <username> --password <password> rmdir <remote_path>
   ```

   - **`<remote_path>`**: Path on the remote server of the directory to remove.

5. **Get File Info**

   ```bash
   python sftp_client.py --hostname <hostname> --username <username> --password <password> info <remote_path>
   ```

   - **`<remote_path>`**: Path on the remote server of the file or directory to get info about.

6. **Resume Upload**

   ```bash
   python sftp_client.py --hostname <hostname> --username <username> --password <password> resume-upload <local_path> <remote_path>
   ```

   - **`<local_path>`**: Path to the local file to upload.
   - **`<remote_path>`**: Path on the remote server where the file will be uploaded.

7. **List Files**

   ```bash
   python sftp_client.py --hostname <hostname> --username <username> --password <password> list <remote_path>
   ```

   - **`<remote_path>`**: Path on the remote server to list files.

8. **Move a File**

   ```bash
   python sftp_client.py --hostname <hostname> --username <username> --password <password> move <remote_src> <remote_dest>
   ```

   - **`<remote_src>`**: Source remote file path.
   - **`<remote_dest>`**: Destination remote file path.

9. **Delete a File**

   ```bash
   python sftp_client.py --hostname <hostname> --username <username> --password <password> delete <remote_path>
   ```

   - **`<remote_path>`**: Path on the remote server of the file to delete.

---

## Example Usage

### Upload a Directory

To upload a directory to the SFTP server:

```bash
python sftp_client.py --hostname example.com --username user --password pass upload-dir /path/to/local/dir /path/to/remote/dir
```

### Download a Directory

To download a directory from the SFTP server:

```bash
python sftp_client.py --hostname example.com --username user --password pass download-dir /path/to/remote/dir /path/to/local/dir
```

### Create a Directory

To create a directory on the SFTP server:

```bash
python sftp_client.py --hostname example.com --username user --password pass mkdir /path/to/remote/new_directory
```

### Remove a Directory

To remove a directory on the SFTP server:

```bash
python sftp_client.py --hostname example.com --username user --password pass rmdir /path/to/remote/directory
```

### Get File Info

To get information about a file on the SFTP server:

```bash
python sftp_client.py --hostname example.com --username user --password pass info /path/to/remote/file.txt
```

### Resume Upload

To resume an interrupted file upload:

```bash
python sftp_client.py --hostname example.com --username user --password pass resume-upload /path/to/local/file.txt /path/to/remote/file.txt
```

### List Files

To list files in a directory on the SFTP server:

```bash
python sftp_client.py --hostname example.com --username user --password pass list /path/to/remote/directory
```

### Move a File

To move or rename a file on the SFTP server:

```bash
python sftp_client.py --hostname example.com --username user --password pass move /path/to/remote/source.txt /path/to/remote/destination.txt
```

### Delete a File

To delete a file on the SFTP server:

```bash
python sftp_client.py --hostname example.com --username user --password pass delete /path/to/remote/file.txt
```

---

## Error Handling and Logging

The script uses the `Loguru` library for logging. Logs are written to `sftp_client.log` and the console, providing detailed information about operations, warnings, and errors. This helps in tracking the actions performed by the script and diagnosing issues.

---

## Conclusion

The **SFTP Client Command Line Tool** is a versatile utility for managing files and directories on SFTP servers. It simplifies the process of file transfer, command execution, and directory management while providing robust error handling and logging capabilities. By following this documentation, users can effectively utilize the tool for their SFTP management needs.
