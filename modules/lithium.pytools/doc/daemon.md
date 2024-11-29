# Daemon Process Manager Documentation

## Overview

The **Daemon Process Manager** is a Python tool designed for monitoring and managing target processes in a robust manner. It supports starting, stopping, and monitoring processes while checking their resource usage (CPU and memory). The script features automatic process restarts when resource usage exceeds defined thresholds and provides comprehensive logging and enhanced command-line interface (CLI) output.

---

## Features

- **Start and Stop Daemon Processes**: Easily manage the lifecycle of target processes.
- **Monitor CPU and Memory Usage**: Continuously check resource usage and take action if thresholds are breached.
- **Automatic Process Restarts**: Restart processes automatically based on resource usage thresholds.
- **Comprehensive Logging**: Uses Loguru for detailed logging of operations.
- **Enhanced CLI Output**: Utilizes Rich for beautiful terminal output, including tables and panels.
- **Detailed Configuration**: Configuration parameters are defined using Python dataclasses for clarity and ease of use.

---

## Requirements

- Python 3.x
- Required Python packages:
  - `loguru`: For logging.
  - `rich`: For styled console output.
  - `psutil`: For system and process utilities.

Install the necessary packages using:

```bash
pip install loguru rich psutil
```

---

## Usage

To run the script, use the following command:

```bash
python daemon_manager.py --help
```

### Command-Line Arguments

- **`start`**: Start the daemon process.

  - `--process_name`: (Optional) Name of the process to monitor (default: `python`).
  - `--script_path`: (Optional) Path to the target script (default: `target_script.py`).
  - `--restart_interval`: (Optional) Restart interval in seconds (default: 5).
  - `--cpu_threshold`: (Optional) CPU usage threshold in percentage (default: 80.0).
  - `--memory_threshold`: (Optional) Memory usage threshold in MB (default: 500.0).
  - `--max_restarts`: (Optional) Maximum number of restarts (default: 3).
  - `--monitor_interval`: (Optional) Monitoring interval in seconds (default: 5).

- **`stop`**: Stop the daemon process.

- **`status`**: Check the status of the daemon process.

---

## Structure of the Script

### Key Classes and Functions

1. **`DaemonConfig` Class**  
   A dataclass that holds configuration parameters for the daemon process, including process name, script path, resource thresholds, and monitoring intervals.

2. **`DaemonProcess` Class**  
   The core class responsible for managing the daemon process.

   - **`__init__`**: Initializes the daemon with the specified configuration.
   - **`start_target_process`**: Starts the target process asynchronously.
   - **`is_process_running`**: Checks if the target process is currently running.
   - **`monitor_process_health`**: Monitors CPU and memory usage of the process and handles restarts if thresholds are exceeded.
   - **Various Methods**: Includes methods for restarting, cleaning up, and managing the process.

3. **Utility Functions**:
   - **`write_pid`**: Writes the daemon's process ID (PID) to a PID file.
   - **`read_pid`**: Reads the PID from the PID file.
   - **`is_daemon_running`**: Checks if the daemon is currently running.
   - **`stop_daemon`**: Stops the daemon process.
   - **`start_daemon`**: Starts the daemon process and sets up the environment.
   - **`status_daemon`**: Checks and displays the status of the daemon process.
   - **`parse_arguments`**: Parses command-line arguments.

---

## Example Commands

Here are some examples of how to use the script:

### Start the Daemon

```bash
python daemon_manager.py start --script_path /path/to/your_script.py
```

### Stop the Daemon

```bash
python daemon_manager.py stop
```

### Check the Daemon Status

```bash
python daemon_manager.py status
```

---

## Logging

The script uses Loguru for logging, which provides detailed logs of the daemon's operations, including process management actions and resource usage statistics. Logs are formatted for readability and can be configured to rotate and retain logs for a specified duration.

---

## Error Handling

The script includes robust error handling:

- If the specified target script does not exist, a critical error is logged, and the script exits.
- Each operation is wrapped in try-except blocks to catch and log exceptions.
- User confirmations are prompted for potentially destructive actions (e.g., stopping the daemon).

---

## Conclusion

The **Daemon Process Manager** is a powerful tool for managing and monitoring processes in a daemonized manner. With features for automatic restarts, resource monitoring, and comprehensive logging, it provides an effective solution for ensuring that critical processes remain operational. The script is designed to be user-friendly with enhanced CLI outputs, making it suitable for both developers and system administrators.
