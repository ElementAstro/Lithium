# WiFi Hotspot Manager Documentation

## Overview

The **WiFi Hotspot Manager** is a cross-platform tool that enables users to easily create, configure, and manage WiFi hotspots on Linux and Windows. The tool supports various authentication methods (WPA-PSK, WPA2, WPA3), allows for the saving and loading of hotspot profiles, and provides monitoring capabilities for connected clients. Additionally, the tool supports asynchronous operations and includes enhanced logging with Loguru and beautiful terminal output using Rich.

---

## Features

- **Create and Manage WiFi Hotspots**: Start and stop hotspots with customizable configurations.
- **Multiple Authentication Methods**: Supports WPA-PSK, WPA2, and WPA3 for security.
- **Profile Management**: Save and load hotspot configurations for easy reuse.
- **Monitor Connected Clients**: View the clients currently connected to the hotspot.
- **Asynchronous Operations**: Use asyncio for non-blocking execution of hotspot commands.
- **Logging**: Detailed logging with Loguru to capture all activities and errors.
- **CLI Enhancements**: Use Rich for terminal output, including formatted tables and progress indicators.

---

## Requirements

- Python 3.x
- Required Python packages:
  - `loguru`: For logging.
  - `rich`: For beautified console output.
  - `asyncio`: For asynchronous operations.
  - `nmcli` (for Linux): Command-line interface for managing network connections.
  - `netsh` (for Windows): Command-line interface for managing network settings.

To install the required packages, use the following:

```bash
pip install loguru rich
```

---

## Usage

The script offers a command-line interface (CLI) for managing the hotspot. Here's how to use it:

### General Commands

```bash
python hotspot.py --help
```

### Available Commands

- **`start`**: Start a WiFi hotspot with the specified configuration.
  - `--name`: Hotspot name (SSID).
  - `--password`: Security key (password) for the hotspot.
  - `--authentication`: Authentication type (`wpa-psk`, `wpa2`, `wpa3`).
  - `--encryption`: Encryption method (`aes`, `tkip`).
  - `--channel`: Channel number for the hotspot (1-14).
  - `--max-clients`: Maximum number of clients allowed to connect.
  - `--band`: WiFi frequency band (`bg`, `a`, `ac`).
  - `--hidden`: Whether to hide the SSID.

Example:

```bash
python hotspot.py start --name MyHotspot --password secret --authentication wpa2 --channel 11 --max-clients 5 --band ac
```

- **`stop`**: Stop the running hotspot.

Example:

```bash
python hotspot.py stop
```

- **`status`**: Check the status of the hotspot (whether it’s running or stopped).

Example:

```bash
python hotspot.py status
```

- **`list-clients`**: List the clients currently connected to the hotspot.

Example:

```bash
python hotspot.py list-clients
```

- **`save-profile`**: Save the current hotspot configuration as a profile.
  - `--profile`: Profile name.
  - Other hotspot configuration options like `--name`, `--authentication`, `--encryption`, etc.

Example:

```bash
python hotspot.py save-profile --profile myProfile --name MyHotspot --authentication wpa2 --encryption aes --channel 11
```

- **`load-profile`**: Load a previously saved hotspot profile and start the hotspot.
  - `--profile`: Profile name to load.

Example:

```bash
python hotspot.py load-profile --profile myProfile
```

---

## Platform Support

The WiFi Hotspot Manager supports the following platforms:

- **Linux**: Uses `nmcli` to manage connections and create hotspots.
- **Windows**: Uses `netsh` for hotspot management.

If running on an unsupported platform, the script will raise a `NotImplementedError`.

---

## Configuration

### `HotspotConfig` Class

This class stores the configuration for the hotspot, including:

- **`name`**: SSID (name) of the hotspot.
- **`password`**: Password for the hotspot.
- **`authentication`**: Type of authentication (WPA, WPA2, WPA3).
- **`encryption`**: Type of encryption (AES, TKIP).
- **`channel`**: Channel number for the WiFi network.
- **`max_clients`**: Maximum number of connected clients allowed.
- **`band`**: WiFi frequency band (`bg`, `a`, `ac`).
- **`hidden`**: Whether the SSID is hidden or visible.

Example:

```python
config = HotspotConfig(
    name="MyHotspot",
    password="securepassword",
    authentication=AuthType.WPA2,
    encryption=EncryptionType.AES,
    channel=11,
    max_clients=10,
    band="ac",
    hidden=False
)
```

---

## Asynchronous Operations

The script uses **asyncio** for running commands asynchronously. This allows operations such as starting or stopping the hotspot to run without blocking the main thread. The script supports non-blocking execution for commands like:

- Starting the hotspot.
- Stopping the hotspot.
- Checking hotspot status.
- Listing connected clients.

This approach ensures that the application can handle multiple commands concurrently and efficiently.

---

## Saving and Loading Profiles

### Saving Profiles

The script allows saving hotspot configurations as profiles. This makes it easy to reuse configurations. Profiles are saved in a JSON file located at `~/.hotspot/config.json`.

### Loading Profiles

You can load previously saved profiles and start the hotspot with those settings.

---

## Logging

The script uses **Loguru** for logging:

- Logs are written to the console and also saved to a log file (`hotspot.log`).
- Logs are rotated every 500 MB and retained for 10 days.
- Logs include debug, info, error, and critical levels for detailed tracking.

---

## Error Handling

The script includes detailed error handling:

- **Platform not supported**: If the script is run on an unsupported platform, an exception will be raised.
- **Missing password**: If the password is not provided when starting the hotspot, the script will raise a `ValueError`.
- **Command failures**: If the shell commands used to start/stop the hotspot fail, an error message is logged.

---

## Conclusion

The **WiFi Hotspot Manager** is a powerful, cross-platform tool for managing WiFi hotspots, supporting multiple authentication methods, and providing an easy-to-use interface for managing configurations, starting and stopping hotspots, and monitoring connected clients.

This tool is ideal for anyone who needs to quickly set up a WiFi hotspot on Linux or Windows, with the flexibility to save and load hotspot configurations for repeated use. With enhanced CLI output and comprehensive logging, it provides a user-friendly experience for managing hotspots in both home and professional environments.
