# Enhanced FTP Client Script Documentation

## Overview

The **Enhanced FTP Client Script** is a Python-based FTP client that supports asynchronous operations, including FTP, FTPS, and SFTP protocols. It utilizes `asyncio` for non-blocking file transfers and provides enhanced logging with Loguru and beautified terminal output using the Rich library. This script is designed to simplify file management over FTP/SFTP connections while providing a user-friendly command-line interface.

---

## Features

- **Asynchronous File Transfers**: Utilizes `asyncio` and `aioftp` for non-blocking file operations.
- **Protocol Support**: Supports FTP, FTPS, and SFTP protocols.
- **Enhanced Logging**: Uses Loguru for structured logging of operations and errors.
- **Beautiful Terminal Output**: Employs the Rich library for styled console output, including tables and progress bars.
- **Detailed Inline Comments and Docstrings**: Provides clear documentation within the code for better understanding and maintenance.

---

## Requirements

- Python 3.7 or higher
- Required Python packages:
  - `aioftp`: For asynchronous FTP client operations.
  - `asyncssh`: For asynchronous SFTP operations.
  - `loguru`: For logging.
  - `rich`: For styled console output.

Install the necessary packages using:

```bash
pip install aioftp asyncssh loguru rich
```

---

## Usage

To run the script, use the following command:

```bash
python ftp_client.py --help
```

### Command-Line Arguments

- **`--host`**: (Required) Server address (IP or hostname).
- **`--port`**: (Optional) Server port (default: 21 for FTP, 22 for SFTP).
- **`--username`**: (Optional) Username for authentication (default: `anonymous`).
- **`--password`**: (Optional) Password for authentication (default: empty).
- **`--protocol`**: (Optional) Protocol to use, either `ftp` or `sftp` (default: `ftp`).
- **`--secure`**: (Optional) Enable FTPS (for FTP protocol).

### Subcommands

The script supports various subcommands for performing different operations:

- **`ls`**: List files in a remote directory.

  - `--path`: Remote path to list (default: `.`).
  - `--recursive`: List files recursively.

- **`get`**: Download a file from the server.

  - `<remote_path>`: Remote file path.
  - `<local_path>`: Local file path.

- **`put`**: Upload a file to the server.

  - `<local_path>`: Local file path.
  - `<remote_path>`: Remote file path.

- **`rm`**: Delete a file on the server.

  - `<remote_path>`: Remote file path to delete.

- **`mkdir`**: Create a directory on the server.

  - `<remote_path>`: Remote directory path to create.

- **`rmdir`**: Remove a directory on the server.

  - `<remote_path>`: Remote directory path to remove.

- **`rename`**: Rename a file or directory on the server.

  - `<old_path>`: Original remote path.
  - `<new_path>`: New remote path.

- **`pwd`**: Show the current directory on the server.

- **`cd`**: Change the current directory on the server.

  - `<remote_path>`: Remote directory path to change to.

- **`batch`**: Perform batch upload/download operations.
  - `<operations_file>`: Path to a JSON file containing operations.

---

## Structure of the Script

### Key Classes and Functions

1. **`AsyncFTPClient` Class**  
   The main class responsible for handling FTP/SFTP connections and operations.

   - **`__init__`**: Initializes the client with connection parameters.
   - **`connect`**: Establishes a connection to the server based on the specified protocol.
   - **`disconnect`**: Closes the connection to the server.
   - **`list_files`**: Lists files and directories at the specified remote path.
   - **`download_file`**: Downloads a file from the server to the local filesystem.
   - **`upload_file`**: Uploads a file from the local filesystem to the server.
   - **`delete_file`**: Deletes a file on the server.
   - **`make_directory`**: Creates a directory on the server.
   - **`remove_directory`**: Removes a directory on the server.
   - **`rename`**: Renames a file or directory on the server.
   - **`get_current_directory`**: Retrieves the current working directory on the server.
   - **`change_directory`**: Changes the current working directory on the server.
   - **`batch_transfer`**: Performs batch upload/download operations based on a list of operations.

2. **`create_parser` Function**  
   Creates and configures the argument parser for command-line options.

3. **`main` Function**  
   The entry point of the script that handles command parsing and execution of the selected operation.

---

## Example of Batch Operations JSON File

The batch operations file should be in JSON format. Here’s an example structure:

```json
[
  {
    "action": "upload",
    "local": "path/to/local/file.txt",
    "remote": "path/to/remote/file.txt"
  },
  {
    "action": "download",
    "remote": "path/to/remote/file.txt",
    "local": "path/to/local/file.txt"
  }
]
```

To execute batch operations, use the command:

```bash
python ftp_client.py --host <host> --protocol ftp batch operations.json
```

---

## Error Handling

The script includes robust error handling:

- If the connection fails, an error message is logged, and the script exits gracefully.
- Each operation (upload, download, delete, etc.) is wrapped in try-except blocks to catch and log exceptions.
- The user is informed of any operational errors through the console.

---

## Conclusion

The **Enhanced FTP Client Script** is a powerful and user-friendly tool for managing files over FTP and SFTP protocols. With support for asynchronous operations, detailed logging, and a clean command-line interface, it simplifies file transfers and remote file management tasks for developers and system administrators alike.

This script serves as a versatile solution for anyone needing to interact with FTP/SFTP servers programmatically, making it a valuable addition to your toolkit.

---

Feel free to adjust any sections to better align with your preferences or the specifics of your implementation!
