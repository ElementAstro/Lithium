# Advanced File Upload Client Documentation

This document provides a detailed guide on how to use the **Advanced File Upload Client**. The client supports uploading files to a server, with advanced features like encryption, multi-threaded uploads, file verification, and configuration file support.

---

## Features

- **File Uploading**: Upload files to a server with retry mechanism.
- **Encryption**: Optionally encrypt files before uploading.
- **Multi-threading**: Upload multiple files concurrently using threads.
- **File Hashing**: Verifies file integrity with SHA-256 hashes.
- **Progress Display**: Visual progress bar for file uploads.
- **Server Response Verification**: Optionally verify the file's hash from the server's response.
- **Configuration File**: Load configuration settings from a JSON file.

---

## Requirements

- Python 3.6 or higher.
- Required libraries: `requests`, `cryptography`, `loguru`, `rich`.

Install the required libraries:

```bash
pip install requests cryptography loguru rich
```

---

## Usage

The main program can be run with several command-line options. Here's the syntax:

```bash
python upload.py --files <file_paths> --server <server_url> [options]
```

## Command-line Arguments

1. **`--files`**: List of file paths to upload.
   - Example: `--files file1.txt file2.jpg`
2. **`--server`**: URL of the server to upload the files.

   - Example: `--server http://example.com/upload`

3. **`--encrypt`**: If specified, files will be encrypted before uploading.

4. **`--key`**: Path to the encryption key file.

   - If encryption is enabled but no key is provided, a new encryption key will be generated and saved to a file (`encryption.key`).

5. **`--config`**: Path to a JSON configuration file that contains the parameters for the upload (see **Configuration File Format** below).

6. **`--filter-type`**: Only upload files with the specified extension (e.g., `.txt`).

   - Example: `--filter-type .txt`

7. **`--verify-server`**: Verify the server's response after uploading the file by comparing the server's hash with the local hash.

8. **`--threads`**: Number of concurrent threads for uploading files. Default is 4.
   - Example: `--threads 8`

---

## Configuration File Format (JSON)

You can specify a JSON configuration file to set the parameters for the upload client. The file can contain the following fields:

```json
{
  "files": ["file1.txt", "file2.jpg"],
  "server": "http://example.com/upload",
  "encrypt": true,
  "key": "path/to/encryption.key",
  "filter_type": ".txt",
  "verify_server": true,
  "threads": 4
}
```

**Fields:**

- **`files`**: List of file paths to upload.
- **`server`**: The server URL to upload files to.
- **`encrypt`**: Whether to encrypt the files before uploading.
- **`key`**: Path to the encryption key file.
- **`filter_type`**: File extension to filter by.
- **`verify_server`**: Whether to verify the server response.
- **`threads`**: Number of threads to use for uploading.

---

## How It Works

1. **File Hashing**:  
   The client calculates the SHA-256 hash of each file before uploading. This ensures file integrity during the upload process. The hash is checked on the server if the `--verify-server` flag is set.

2. **Encryption**:  
   If encryption is enabled (`--encrypt`), the file will be encrypted using the AES algorithm (via Fernet). The encryption key can either be provided by the user through the `--key` argument or a new key will be generated. The encrypted file is saved with a `.enc` extension and is uploaded instead of the original.

3. **Multi-threading**:  
   The client supports uploading multiple files concurrently. The number of threads can be specified with the `--threads` argument. This speeds up the process when uploading many files.

4. **Progress Tracking**:  
   A visual progress bar is displayed during the upload, showing the progress of each file in terms of bytes uploaded and time remaining.

5. **Retries**:  
   If an upload fails, the client will automatically retry the upload up to 3 times (default). You can adjust the number of retries using the `--retries` argument in the future.

6. **Server Hash Verification**:  
   After the file is uploaded, if `--verify-server` is enabled, the server will return a hash of the uploaded file. The client compares this hash with the local file hash to ensure that the file has been uploaded correctly.

---

## Functions and Methods

- **`calculate_hash(file_path: Path) -> str`**  
  Computes the SHA-256 hash of the file located at `file_path`.

- **`encrypt_file(file_path: Path, key: bytes) -> Path`**  
  Encrypts the file at `file_path` using the provided encryption key.

- **`upload_file(file_path: Path, server_url: str, retries: int = 3, verify_server: bool = False) -> bool`**  
  Uploads a file to the server, retrying if necessary. Returns `True` if the upload succeeds, `False` if it fails.

- **`upload_worker(queue: Queue, server_url: str, key: Optional[bytes], verify_server: bool)`**  
  A worker function for uploading files in parallel.

- **`upload_multiple_files(file_paths: List[Path], server_url: str, threads: int = 4, key: Optional[bytes] = None, verify_server: bool = False)`**  
  Uploads multiple files concurrently using multiple threads.

- **`load_config(config_file: Path) -> dict`**  
  Loads configuration settings from a JSON file.

---

## Example Usage

## Uploading Files without Encryption

```bash
python upload.py --files file1.txt file2.jpg --server http://example.com/upload
```

## Uploading Files with Encryption

```bash
python upload.py --files file1.txt file2.jpg --server http://example.com/upload --encrypt --key path/to/keyfile
```

## Uploading Files Using a Configuration File

```bash
python upload.py --config config.json
```

## Uploading Files with Specific Extension

```bash
python upload.py --files file1.txt file2.jpg --server http://example.com/upload --filter-type .txt
```

## Uploading Files Using Multiple Threads

```bash
python upload.py --files file1.txt file2.jpg --server http://example.com/upload --threads 8
```

---

## Error Handling

- **File Not Found**: If a specified file doesn't exist, a `FileNotFoundError` is raised.
- **Encryption Key Not Found**: If the encryption key file is not found, an error is logged, and the client exits.
- **Server Response Errors**: If the server returns a status code other than `200`, an error is logged, and the client will retry the upload based on the retry logic.

---

## Logging

The client uses the `loguru` library for logging, providing enhanced logging capabilities such as rich formatting and real-time updates to the console. The log file is saved as `upload_client.log`.

---

## Conclusion

The **Advanced File Upload Client** is a powerful and flexible tool for uploading files to a server, with additional features like encryption, multi-threading, and file integrity verification. The client is easy to use via command-line arguments or a configuration file.
