# .NET Framework Installer Script Documentation

## Overview

The **.NET Framework Installer Script** is a Python command-line utility designed to check installed .NET Framework versions on a Windows machine, download the required installer if necessary, and handle installation and uninstallation of specific versions. It supports multithreaded downloads, checksum verification for file integrity, and uses the Windows Registry to check for installed versions.

This tool is useful for administrators and developers who need to manage the installation of specific .NET Framework versions on Windows machines.

---

## Features

- **Check Installed Versions**: Query the Windows Registry to check if a specific version of the .NET Framework is installed.
- **Download and Install**: Automatically download the installer for missing .NET Framework versions and run the installer.
- **Multithreaded Downloading**: Use asyncio and aiohttp to download the installer in parallel, speeding up the download process.
- **Checksum Verification**: Optionally verify the downloaded installer file’s integrity using SHA256 or MD5 checksums.
- **Uninstall .NET Framework Versions**: Remove an installed version of the .NET Framework from the system.
- **Logging and Error Handling**: Detailed logging with Loguru and informative messages for all operations.
- **Cross-Platform Usage**: Although designed for Windows, the script supports Linux and macOS environments, but only performs download-related actions for non-Windows systems.

---

## Requirements

- **Python 3.9+**
- **Windows Operating System** (For installation and uninstallation functionalities)
- Required Python libraries:
  - `requests`
  - `tqdm`
  - `aiohttp`
  - `aiofiles`
  - `loguru`

You can install the required libraries using:

```bash
pip install requests tqdm aiohttp aiofiles loguru
```

---

## Usage

### Command-Line Arguments

The script provides the following command-line options:

- **`--list`**: List all installed .NET Framework versions.
- **`--check VERSION`**: Check if a specific .NET Framework version (e.g., `v4.0.30319`) is installed.
- **`--download URL`**: Specify the URL from which to download the .NET Framework installer.
- **`--install FILE_PATH`**: Specify the local file path to the installer executable for installation.
- **`--threads`**: Specify the number of threads to use for multithreaded downloading (default is 4).
- **`--checksum SHA256`**: Specify the expected SHA256 checksum of the downloaded file for verification.
- **`--uninstall VERSION`**: Uninstall a specific .NET Framework version.
- **`--verbose`**: Enable verbose logging.

#### Example Commands

1. **List Installed .NET Framework Versions**:

   ```bash
   python net_framework_installer.py --list
   ```

2. **Check if a Specific Version is Installed**:

   ```bash
   python net_framework_installer.py --check v4.0.30319
   ```

3. **Download and Install a Version**:

   ```bash
   python net_framework_installer.py --check v4.0.30319 --download [URL] --install [INSTALLER_PATH] --threads 4 --checksum [SHA256]
   ```

4. **Uninstall a Specific Version**:
   ```bash
   python net_framework_installer.py --uninstall v4.0.30319
   ```

---

## Class Definitions

### `NetFrameworkInstallerConfig`

This class contains the configuration settings for the installer, including the download directory, number of threads, and log file path.

#### Attributes:

- **`download_dir`**: Directory where downloaded installers will be saved.
- **`num_threads`**: Number of threads to use for downloading the installer.
- **`log_file`**: Path to the log file.

### `NetFrameworkInstaller`

This class handles the main functionality, such as checking installed versions, downloading installers, verifying file checksums, installing/uninstalling .NET Framework versions, and logging.

#### Methods:

- **`setup_logging()`**: Configures logging with Loguru, including rotation and log levels.
- **`verify_file_checksum(file_path, original_checksum, hash_algo)`**: Verifies the checksum of a downloaded file using SHA256 or MD5.
- **`check_dotnet_installed(version)`**: Checks if a specific version of .NET Framework is installed by querying the Windows Registry.
- **`list_installed_dotnets()`**: Lists all installed .NET Framework versions.
- **`download_file_part(session, url, start, end)`**: Asynchronously downloads a part of the file specified by byte range.
- **`download_file(url, filename, num_threads, expected_checksum)`**: Downloads a file using multiple threads asynchronously and verifies the checksum.
- **`install_software(installer_path)`**: Executes the installer from a specified path.
- **`uninstall_dotnet(version)`**: Uninstalls a specific .NET Framework version.
- **`parse_arguments()`**: Parses command-line arguments.
- **`handle_download_and_install(url, installer_path, checksum)`**: Manages the download and installation process.
- **`main()`**: The main function that invokes the appropriate script functionality based on the command-line arguments.

---

## Logging and Error Handling

- **Log Levels**: The script uses Loguru for logging with different log levels. The default level is `INFO`, but it can be set to `DEBUG` for more detailed logs.
- **Log File**: Logs are stored in a file, and older logs are archived and compressed.
- **Error Handling**: The script handles various exceptions such as missing files, registry query failures, checksum mismatches, etc., and logs detailed error messages.

---

## Example Workflow

1. **Check if a specific .NET Framework version is installed**:
   The script queries the Windows Registry to check if a given version of .NET Framework (e.g., `v4.0.30319`) is installed. If it is installed, the script outputs the status. If not, the user is informed that the version is not installed.

2. **Download and install**:
   If the version is not installed, the user can provide a download URL and installer path. The script will download the installer using multiple threads and verify the file's checksum. After the download completes, the installer is executed.

3. **Uninstall .NET Framework**:
   The script can also uninstall a specific version of .NET Framework by deleting its registry entry.

---

## Conclusion

The **.NET Framework Installer Script** simplifies the management of .NET Framework installations on Windows by allowing users to easily check, install, or uninstall specific versions. It features multithreaded downloads, checksum verification for security, and comprehensive logging for troubleshooting. The script is a valuable tool for administrators who need to manage .NET Framework versions across multiple systems.
