# Nginx Manager Documentation

This document provides a comprehensive guide on how to use the **Nginx Manager**, a command-line utility designed to manage Nginx server operations. The tool allows users to start, stop, reload, and configure Nginx, along with managing logs and sites.

---

## Key Features

- **Start/Stop/Reload Nginx**: Control the Nginx server's state.
- **Test Configuration**: Validate the Nginx configuration syntax.
- **View Logs**: Access Nginx access and error logs.
- **Clear Logs**: Clear the contents of log files.
- **Backup and Restore Configuration**: Manage backups of the Nginx configuration files.
- **List Sites**: Display available and enabled sites.
- **Version Information**: Retrieve the version of the Nginx server.
- **Enhanced Logging**: Uses the `Loguru` library for detailed logging of operations and errors.
- **Rich Console Output**: Utilizes the `Rich` library for beautified terminal outputs.

---

## Requirements

- Python 3.7 or higher.
- Required libraries: `loguru`, `rich`.

Install the required libraries using pip:

```bash
pip install loguru rich
```

Make sure the Nginx server is installed and accessible on your system.

---

## Usage

The script can be executed from the command line with various commands. The command syntax is as follows:

```bash
python nginx_manager.py [command]
```

### Available Commands

- **start**: Start the Nginx server.
- **stop**: Stop the Nginx server.
- **reload**: Reload the Nginx configuration.
- **restart**: Restart the Nginx server.
- **test**: Test the Nginx configuration syntax.
- **check**: Check Nginx configuration syntax.
- **status**: Show the running status of Nginx.
- **version**: Show Nginx version information.
- **backup**: Backup the Nginx configuration file.
- **restore**: Restore the Nginx configuration from backup.
- **view_logs**: View Nginx logs (access or error).
- **clear_logs**: Clear Nginx logs (access or error).
- **list_sites**: List available and enabled sites.
- **help**: Show help message.

### Example Usage

#### Start Nginx

To start the Nginx server:

```bash
python nginx_manager.py start
```

#### Stop Nginx

To stop the Nginx server:

```bash
python nginx_manager.py stop
```

#### Reload Nginx Configuration

To reload the Nginx configuration:

```bash
python nginx_manager.py reload
```

#### Test Nginx Configuration

To test the Nginx configuration:

```bash
python nginx_manager.py test
```

#### View Access Logs

To view the access logs:

```bash
python nginx_manager.py view_logs access
```

#### Clear Error Logs

To clear the error logs:

```bash
python nginx_manager.py clear_logs error
```

#### Backup Nginx Configuration

To backup the Nginx configuration:

```bash
python nginx_manager.py backup
```

#### List Available Sites

To list available and enabled sites:

```bash
python nginx_manager.py list_sites
```

---

## Error Handling and Logging

The script uses the `Loguru` library for logging. Logs are written to `nginx_manager.log` and the console, providing detailed information about operations, warnings, and errors. This helps in tracking the actions performed by the script and diagnosing issues.

---

## Conclusion

The **Nginx Manager** is a powerful utility for managing Nginx server operations. It simplifies the process of controlling the server and managing configurations while providing robust error handling and logging capabilities. By following this documentation, users can effectively utilize the tool for their Nginx management needs.
