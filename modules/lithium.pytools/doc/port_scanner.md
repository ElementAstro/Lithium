# Asynchronous Port Scanner Tool Documentation

This document provides a comprehensive guide on how to use the **Asynchronous Port Scanner Tool**, a Python script designed for scanning specified ports on given IP addresses using TCP or UDP protocols. The tool includes functionalities to scan ports, save results, and handle exceptions with detailed logging.

---

## Key Features

- **Asynchronous Scanning**: Utilizes `asyncio` for efficient port scanning.
- **Protocol Support**: Supports both TCP and UDP protocols.
- **Detailed Logging**: Uses the `Loguru` library for logging operations and errors.
- **Rich Console Output**: Provides beautified terminal outputs using the `Rich` library.
- **Progress Indicators**: Displays progress during scanning operations.
- **Customizable Options**: Allows users to specify port ranges, timeouts, and output formats.

---

## Requirements

- Python 3.7 or higher.
- Required libraries: `paramiko`, `loguru`, `rich`.

Install the required libraries using pip:

```bash
pip install paramiko loguru rich
```

---

## Usage

The script can be executed from the command line with various options. The command syntax is as follows:

```bash
python port_scanner.py <action> [options]
```

### Command-Line Options

- **`<ips>`**: Required. Target IP address(es) to scan.
- **`-p`, `--ports`**: Optional. Port range to scan (default: `1-1024`).
- **`-t`, `--timeout`**: Optional. Timeout for each port scan in seconds (default: `1.0`).
- **`-o`, `--output`**: Optional. Output file to save results (default: `scan_results.txt`).
- **`--protocol`**: Optional. Protocol to use for scanning (`tcp` or `udp`, default: `tcp`).
- **`--quick`**: Optional. Quick scan mode (common ports).
- **`--json`**: Optional. Save results in JSON format.
- **`--verbose`**: Optional. Print detailed scan information.
- **`--exclude`**: Optional. Comma-separated list of ports to exclude from scanning.

---

## Example Usage

### Scan Ports on a Single IP

To scan the first 1024 ports on a single IP address:

```bash
python port_scanner.py 192.168.1.1 -p 1-1024
```

### Scan Ports on Multiple IPs

To scan a specific range of ports on multiple IP addresses:

```bash
python port_scanner.py 192.168.1.1 192.168.1.2 -p 80,443
```

### Quick Scan of Common Ports

To perform a quick scan of common ports on an IP address:

```bash
python port_scanner.py 192.168.1.1 --quick
```

### Save Results in JSON Format

To scan an IP and save results in JSON format:

```bash
python port_scanner.py 192.168.1.1 -o results.json --json
```

### Exclude Specific Ports

To exclude certain ports from the scan:

```bash
python port_scanner.py 192.168.1.1 -p 1-1024 --exclude 22,80
```

---

## Error Handling and Logging

The script uses the `Loguru` library for logging. Logs are written to `port_scanner.log` and the console, providing detailed information about operations, warnings, and errors. This helps in tracking the actions performed by the script and diagnosing issues.

---

## Conclusion

The **Asynchronous Port Scanner Tool** is a powerful utility for scanning ports on remote servers. It simplifies the process of network scanning while providing robust error handling and logging capabilities. By following this documentation, users can effectively utilize the tool for their network management needs.
