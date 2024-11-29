# Auto Updater Script Documentation

This document provides a comprehensive guide for using the **Auto Updater Script**. This script is designed to automatically check for updates, download them, verify their integrity, and install them for a given application. It supports multi-threaded downloads, file verification, backup of current files, and logging of update history.

---

## Features

- **Update Checking**: Automatically checks for updates from a specified URL.
- **Version Comparison**: Compares the current application version with the latest available version.
- **Multi-threaded Downloads**: Downloads files concurrently using multiple threads for efficiency.
- **File Verification**: Verifies the integrity of downloaded files using SHA-256 hashes.
- **Backup Current Files**: Creates backups of the current installation before applying updates.
- **Custom Actions**: Supports custom actions after downloading and installing updates.
- **Logging**: Maintains a log of update activities and errors for troubleshooting.

---

## Requirements

- Python 3.6 or higher.
- Required libraries: `requests`, `loguru`, `tqdm`, `packaging`.

You can install the required libraries using pip:

```bash
pip install requests loguru tqdm packaging
```

---

## Usage

The script can be executed from the command line with a configuration file that specifies the necessary parameters. The command syntax is as follows:

```bash
python auto_updater.py --config <path_to_config_file>
```

### Command-line Arguments

1. **`--config`**: Path to the JSON configuration file that contains the updater settings.
   - Example: `--config config.json`

---

## Configuration File Format (JSON)

The configuration file should be structured in JSON format and can include the following fields:

```json
{
  "url": "http://example.com/update_info.json",
  "install_dir": "/path/to/application",
  "num_threads": 4,
  "custom_params": {
    "post_download": "function_name", // Optional custom function for post-download actions
    "post_install": "function_name" // Optional custom function for post-install actions
  },
  "current_version": "1.0.0"
}
```

**Fields:**

- **`url`**: The URL to check for updates (should return a JSON response containing version info).
- **`install_dir`**: The directory where the application is currently installed.
- **`num_threads`**: (Optional) Number of threads to use for downloading files. Default is 4.
- **`custom_params`**: (Optional) A dictionary of custom parameters for post-download and post-install actions.
- **`current_version`**: The current version of the application.

---

## How It Works

1. **Initialization**: The `AutoUpdater` class is initialized with the configuration parameters from the JSON file.
2. **Check for Updates**: The script checks for updates by making a GET request to the specified URL. It expects a JSON response containing the latest version, download URL, and expected hash.
3. **Version Comparison**: Compares the current version of the application with the latest version available.
4. **Download Files**: If an update is available, the script downloads the update file using multiple threads.
5. **File Verification**: After downloading, it verifies the integrity of the downloaded file against the expected hash.
6. **Backup Current Installation**: Before applying the update, the current installation is backed up.
7. **Extract and Install**: The downloaded update file is extracted, and the files are moved to the installation directory.
8. **Custom Actions**: Executes any custom actions specified in the configuration after downloading and installing.
9. **Logging**: Logs the entire update process, including any errors encountered.

---

## Key Methods in the AutoUpdater Class

- **`__init__(self, config: Dict[str, Any])`**: Initializes the updater with the provided configuration.
- **`check_for_updates(self) -> bool`**: Checks for updates from the specified URL and returns `True` if updates are available.
- **`compare_versions(self, current_version: str) -> bool`**: Compares the current version with the latest version.
- **`download_file(self, url: str, dest: Path)`**: Downloads a file from the given URL to the specified destination.
- **`verify_file(self, file_path: Path, expected_hash: str) -> bool`**: Verifies the downloaded file's hash against the expected hash.
- **`extract_zip(self, zip_path: Path, extract_to: Path)`**: Extracts a zip file to the specified directory.
- **`move_files(self, src: Path, dest: Path)`**: Moves files from the source directory to the destination directory.
- **`backup_files(self, src: Path, backup_dir: Path)`**: Backs up current files to the specified backup directory.
- **`cleanup(self)`**: Cleans up temporary files and directories.
- **`custom_post_download(self)`**: Executes custom actions after downloading.
- **`custom_post_install(self)`**: Executes custom actions after installing.
- **`log_update(self, current_version: str, new_version: str)`**: Logs the update history.
- **`download_multiple_files(self, urls: List[str], dest_dir: Path)`**: Downloads multiple files concurrently.
- **`update(self, current_version: str)`**: Orchestrates the update process.

---

## Example Usage

### Example Configuration File

```json
{
  "url": "http://example.com/update_info.json",
  "install_dir": "/path/to/application",
  "num_threads": 4,
  "custom_params": {
    "post_download": "my_post_download_function",
    "post_install": "my_post_install_function"
  },
  "current_version": "1.0.0"
}
```

### Running the Updater

```bash
python auto_updater.py --config /path/to/config.json
```

---

## Error Handling

- **Configuration File Not Found**: If the specified configuration file does not exist, an error message is logged, and the script exits.
- **Update Check Failure**: If there is a problem checking for updates (e.g., network issues), an error is logged, and the script exits.
- **Download Errors**: If a download fails, an error is logged, and the script will attempt to retry the download if applicable.
- **File Verification Failure**: If the downloaded file fails verification, the update process is aborted, and an error message is logged.

---

## Logging

The script uses the `loguru` library for logging. Logs are written to `updater.log` in the installation directory, and the log rotation is set to 10 MB with a retention period of 30 days.

---

## Conclusion

The **Auto Updater Script** is a robust solution for automating the update process of applications. It simplifies the management of updates, ensuring that applications stay current with minimal user intervention. By following this documentation, users can easily configure and utilize the updater for their applications.
