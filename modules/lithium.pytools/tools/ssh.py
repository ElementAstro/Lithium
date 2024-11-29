# python
import paramiko
from paramiko.ssh_exception import (
    SSHException,
    AuthenticationException,
    NoValidConnectionsError,
)
from pathlib import Path
from typing import Union, List, Optional, Tuple, Literal
from loguru import logger
from rich.console import Console
from rich.table import Table
from rich.progress import Progress, BarColumn, TextColumn, TimeRemainingColumn
import sys
import argparse
import asyncio
import concurrent.futures


# Define custom exception classes
class SSHError(Exception):
    """Base exception class for SSHClient."""


class SSHConnectionError(SSHError):
    """Exception for SSH connection failures."""


class SSHCommandError(SSHError):
    """Exception for SSH command execution failures."""


class SSHPermissionError(SSHError):
    """Exception for SSH permission related errors."""


class SFTPError(SSHError):
    """Exception for SFTP operation failures."""


class SSHClient:
    """Client class to manage SSH connections and operations."""

    def __init__(
        self,
        hostname: str,
        port: int,
        username: str,
        password: Optional[str] = None,
        key_file: Optional[str] = None,
        timeout: Optional[float] = 10.0,
    ):
        """
        Initialize the SSH client.

        Args:
            hostname (str): Hostname or IP address.
            port (int): Port number, typically 22.
            username (str): Username for SSH login.
            password (Optional[str]): Password for SSH (if using password authentication).
            key_file (Optional[str]): Path to the private key file (if using key authentication).
            timeout (Optional[float]): Connection timeout in seconds.
        """
        self.hostname = hostname
        self.port = port
        self.username = username
        self.password = password
        self.key_file = key_file
        self.timeout = timeout
        self.client: Optional[paramiko.SSHClient] = None
        self.sftp: Optional[paramiko.SFTPClient] = None
        self.console = Console()

    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def setup_logging(self):
        """Configure Loguru for logging."""
        logger.remove()
        logger.add(
            sys.stderr,
            level="INFO",
            format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level}</level> | {message}",
        )
        logger.add(
            "ssh_client.log",
            rotation="5 MB",
            retention="7 days",
            enqueue=True,
            level="DEBUG",
            format="{time} | {level} | {message}",
        )
        logger.debug("Logging is configured.")

    def connect(self):
        """
        Connect to the remote SSH server.
        Raises:
            SSHConnectionError: If connection fails.
        """
        logger.info(
            f"Attempting to connect to {self.hostname}:{self.port} as {self.username}"
        )
        self.setup_logging()
        try:
            self.client = paramiko.SSHClient()
            self.client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            connect_params = {
                "hostname": self.hostname,
                "port": self.port,
                "username": self.username,
                "password": self.password,
                "key_filename": self.key_file,
                "look_for_keys": False,
                "allow_agent": False,
                "timeout": self.timeout,
            }
            self.client.connect(**connect_params)
            self.sftp = self.client.open_sftp()
            logger.success(
                f"Successfully connected to {self.hostname}:{self.port}"
            )
        except AuthenticationException as e:
            logger.error(f"Authentication failed: {e}")
            raise SSHConnectionError("Authentication failed") from e
        except NoValidConnectionsError as e:
            logger.error(
                f"Unable to connect to {self.hostname}:{self.port}: {e}"
            )
            raise SSHConnectionError(
                f"Unable to connect to {self.hostname}:{self.port}"
            ) from e
        except SSHException as e:
            logger.error(f"SSH connection failed: {e}")
            raise SSHConnectionError("SSH connection failed") from e
        except Exception as e:
            logger.error(f"Connection failed: {e}")
            raise SSHConnectionError("Connection failed") from e

    def execute_command(
        self, command: str, timeout: Optional[float] = None
    ) -> Tuple[str, str]:
        """
        Execute a command on the remote SSH server.

        Args:
            command (str): The command to execute.
            timeout (Optional[float]): Optional timeout for command execution.

        Returns:
            Tuple[str, str]: A tuple containing standard output and standard error.

        Raises:
            SSHCommandError: If command execution fails.
        """
        if self.client is None:
            logger.error(
                "Not connected to any SSH server. Call connect() first.")
            raise SSHCommandError("Not connected to any SSH server.")

        logger.info(f"Executing command: {command}")
        try:
            stdin, stdout, stderr = self.client.exec_command(
                command, timeout=timeout
            )
            output = stdout.read().decode("utf-8")
            error = stderr.read().decode("utf-8")
            logger.debug(f"Command output: {output}")
            if error:
                logger.warning(f"Command error output: {error}")
            exit_status = stdout.channel.recv_exit_status()
            if exit_status != 0:
                logger.error(
                    f"Command '{command}' failed with exit status {exit_status}"
                )
                raise SSHCommandError(
                    f"Command '{command}' failed with exit status {exit_status}"
                )
            logger.success(f"Command '{command}' executed successfully.")
            return output, error
        except SSHException as e:
            logger.error(f"Failed to execute command '{command}': {e}")
            raise SSHCommandError(
                f"Failed to execute command '{command}'"
            ) from e
        except Exception as e:
            logger.error(f"Unexpected error during command execution: {e}")
            raise SSHCommandError(
                "Unexpected error during command execution"
            ) from e

    def upload_file(
        self, local_path: Union[str, Path], remote_path: Union[str, Path]
    ):
        """
        Upload a file to the remote SSH server using SFTP.

        Args:
            local_path (Union[str, Path]): Path to the local file.
            remote_path (Union[str, Path]): Path on the remote server where the file will be uploaded.

        Raises:
            SFTPError: If file upload fails.
        """
        if self.sftp is None:
            logger.error(
                "SFTP session is not established. Call connect() first.")
            raise SFTPError("SFTP session is not established.")

        logger.info(f"Uploading {local_path} to {remote_path}")
        try:
            # Use Rich Progress to show upload progress
            file_size = Path(local_path).stat().st_size
            with Progress(
                "[progress.description]{task.description}",
                BarColumn(),
                "[progress.percentage]{task.percentage:>3.0f}%",
                "•",
                TimeRemainingColumn(),
            ) as progress:
                task = progress.add_task(
                    f"Uploading {Path(local_path).name}", total=file_size
                )

                def callback(transferred, total):
                    progress.update(task, completed=transferred)

                self.sftp.put(
                    str(local_path),
                    str(remote_path),
                    callback=callback,
                )
            logger.success(
                f"Successfully uploaded {local_path} to {remote_path}"
            )
            self.console.print(
                f"[bold green]Successfully uploaded {local_path} to {remote_path}[/bold green]"
            )
        except FileNotFoundError as e:
            logger.error(f"Local file not found: {e}")
            raise SFTPError("Local file not found") from e
        except SSHException as e:
            logger.error(f"SFTP upload failed: {e}")
            raise SFTPError("SFTP upload failed") from e
        except Exception as e:
            logger.error(f"Unexpected error during file upload: {e}")
            raise SFTPError("Unexpected error during file upload") from e

    def download_file(
        self, remote_path: Union[str, Path], local_path: Union[str, Path]
    ):
        """
        Download a file from the remote SSH server using SFTP.

        Args:
            remote_path (Union[str, Path]): Path on the remote server to download.
            local_path (Union[str, Path]): Path on the local machine where the file will be saved.

        Raises:
            SFTPError: If file download fails.
        """
        if self.sftp is None:
            logger.error(
                "SFTP session is not established. Call connect() first.")
            raise SFTPError("SFTP session is not established.")

        logger.info(f"Downloading {remote_path} to {local_path}")
        try:
            # Use Rich Progress to show download progress
            file_size = self.sftp.stat(str(remote_path)).st_size
            with Progress(
                "[progress.description]{task.description}",
                BarColumn(),
                "[progress.percentage]{task.percentage:>3.0f}%",
                "•",
                TimeRemainingColumn(),
            ) as progress:
                task = progress.add_task(
                    f"Downloading {Path(remote_path).name}", total=file_size
                )

                def callback(transferred, total):
                    progress.update(task, completed=transferred)

                self.sftp.get(
                    str(remote_path),
                    str(local_path),
                    callback=callback,
                )
            logger.success(
                f"Successfully downloaded {remote_path} to {local_path}"
            )
            self.console.print(
                f"[bold green]Successfully downloaded {remote_path} to {local_path}[/bold green]"
            )
        except FileNotFoundError as e:
            logger.error(f"Remote file not found: {e}")
            raise SFTPError("Remote file not found") from e
        except SSHException as e:
            logger.error(f"SFTP download failed: {e}")
            raise SFTPError("SFTP download failed") from e
        except Exception as e:
            logger.error(f"Unexpected error during file download: {e}")
            raise SFTPError("Unexpected error during file download") from e

    def list_remote_directory(
        self, remote_path: Union[str, Path]
    ) -> List[str]:
        """
        List the contents of a remote directory using SFTP.

        Args:
            remote_path (Union[str, Path]): Path on the remote server to list.

        Returns:
            List[str]: A list of file and directory names.

        Raises:
            SFTPError: If listing fails.
        """
        if self.sftp is None:
            logger.error(
                "SFTP session is not established. Call connect() first.")
            raise SFTPError("SFTP session is not established.")

        logger.info(f"Listing directory: {remote_path}")
        try:
            files = self.sftp.listdir(str(remote_path))
            logger.success(f"Successfully listed directory: {remote_path}")
            logger.debug(f"Directory contents: {files}")

            table = Table(title=f"Contents of {remote_path}")
            table.add_column("Name", style="cyan", no_wrap=True)
            table.add_column("Type", style="magenta")
            table.add_column("Size", style="green")

            for file in files:
                filepath = Path(remote_path) / file
                try:
                    attr = self.sftp.stat(str(filepath))
                    size = attr.st_size
                    mode = attr.st_mode
                    file_type = (
                        "Directory" if paramiko.S_ISDIR(mode) else "File"
                    )
                    table.add_row(file, file_type, str(size))
                except IOError:
                    table.add_row(file, "Unknown", "Unknown")

            self.console.print(table)
            return files
        except FileNotFoundError as e:
            logger.error(f"Remote directory not found: {e}")
            raise SFTPError("Remote directory not found") from e
        except SSHException as e:
            logger.error(f"SFTP list directory failed: {e}")
            raise SFTPError("SFTP list directory failed") from e
        except Exception as e:
            logger.error(f"Unexpected error during listing directory: {e}")
            raise SFTPError("Unexpected error during listing directory") from e

    def create_remote_directory(self, remote_path: Union[str, Path]):
        """
        Create a directory on the remote SSH server using SFTP.

        Args:
            remote_path (Union[str, Path]): Path on the remote server where the directory will be created.

        Raises:
            SFTPError: If directory creation fails.
        """
        if self.sftp is None:
            logger.error(
                "SFTP session is not established. Call connect() first.")
            raise SFTPError("SFTP session is not established.")

        logger.info(f"Creating remote directory: {remote_path}")
        try:
            self.sftp.mkdir(str(remote_path))
            logger.success(
                f"Successfully created remote directory: {remote_path}")
            self.console.print(
                f"[bold green]Successfully created remote directory: {remote_path}[/bold green]"
            )
        except FileExistsError:
            logger.warning(f"Remote directory already exists: {remote_path}")
            self.console.print(
                f"[yellow]Remote directory already exists: {remote_path}[/yellow]"
            )
        except SSHException as e:
            logger.error(f"SFTP mkdir failed: {e}")
            raise SFTPError("SFTP mkdir failed") from e
        except Exception as e:
            logger.error(f"Unexpected error during creating directory: {e}")
            raise SFTPError(
                "Unexpected error during creating directory") from e

    def delete_remote_file(self, remote_path: Union[str, Path]):
        """
        Delete a file on the remote SSH server using SFTP.

        Args:
            remote_path (Union[str, Path]): Path on the remote server of the file to delete.

        Raises:
            SFTPError: If file deletion fails.
        """
        if self.sftp is None:
            logger.error(
                "SFTP session is not established. Call connect() first.")
            raise SFTPError("SFTP session is not established.")

        logger.info(f"Deleting remote file: {remote_path}")
        try:
            self.sftp.remove(str(remote_path))
            logger.success(f"Successfully deleted remote file: {remote_path}")
            self.console.print(
                f"[bold green]Successfully deleted remote file: {remote_path}[/bold green]"
            )
        except FileNotFoundError as e:
            logger.error(f"Remote file not found: {e}")
            raise SFTPError("Remote file not found") from e
        except SSHException as e:
            logger.error(f"SFTP delete failed: {e}")
            raise SFTPError("SFTP delete failed") from e
        except Exception as e:
            logger.error(f"Unexpected error during file deletion: {e}")
            raise SFTPError("Unexpected error during file deletion") from e

    def close(self):
        """
        Close the SSH and SFTP connections.
        """
        logger.info("Closing SSH and SFTP connections.")
        try:
            if self.sftp:
                self.sftp.close()
                logger.debug("SFTP connection closed.")
            if self.client:
                self.client.close()
                logger.debug("SSH connection closed.")
            logger.success("All connections closed successfully.")
            self.console.print(
                "[bold green]All connections closed successfully.[/bold green]")
        except Exception as e:
            logger.error(f"Error while closing connections: {e}")
            raise SSHError("Error while closing connections") from e


def parse_args() -> argparse.Namespace:
    """
    Parse command-line arguments.

    Returns:
        argparse.Namespace: Parsed arguments.
    """
    parser = argparse.ArgumentParser(description="SSH Client Tool")
    parser.add_argument("--hostname", required=True,
                        help="Hostname or IP address")
    parser.add_argument(
        "--port", type=int, default=22, help="SSH port number"
    )
    parser.add_argument("--username", required=True, help="SSH username")

    auth_group = parser.add_mutually_exclusive_group(required=True)
    auth_group.add_argument("--password", help="SSH password")
    auth_group.add_argument(
        "--key_file", help="Path to SSH private key file"
    )

    subparsers = parser.add_subparsers(
        dest="command", help="Available commands")

    # Execute command
    exec_parser = subparsers.add_parser(
        "exec", help="Execute a command on the remote server"
    )
    exec_parser.add_argument("cmd", help="Command to execute")
    exec_parser.add_argument(
        "--timeout", type=float, default=None, help="Command timeout in seconds"
    )

    # Upload file
    upload_parser = subparsers.add_parser(
        "upload", help="Upload a file to the remote server"
    )
    upload_parser.add_argument("local_path", help="Path to the local file")
    upload_parser.add_argument(
        "remote_path", help="Path on the remote server"
    )

    # Download file
    download_parser = subparsers.add_parser(
        "download", help="Download a file from the remote server"
    )
    download_parser.add_argument(
        "remote_path", help="Path on the remote server"
    )
    download_parser.add_argument(
        "local_path", help="Path to save the downloaded file locally"
    )

    # List directory
    list_parser = subparsers.add_parser(
        "list", help="List contents of a remote directory"
    )
    list_parser.add_argument("remote_path", help="Remote directory path")

    # Create directory
    mkdir_parser = subparsers.add_parser(
        "mkdir", help="Create a directory on the remote server"
    )
    mkdir_parser.add_argument(
        "remote_path", help="Path of the remote directory to create"
    )

    # Delete file
    delete_parser = subparsers.add_parser(
        "delete", help="Delete a file on the remote server"
    )
    delete_parser.add_argument(
        "remote_path", help="Path of the remote file to delete"
    )

    return parser.parse_args()


def main():
    """
    Main function to execute the SSH client operations based on command-line arguments.
    """
    args = parse_args()
    config = {
        "hostname": args.hostname,
        "port": args.port,
        "username": args.username,
        "password": args.password,
        "key_file": args.key_file,
    }

    try:
        with SSHClient(**config) as ssh:
            if args.command == "exec":
                output, error = ssh.execute_command(
                    args.cmd, timeout=args.timeout
                )
                if output:
                    console = Console()
                    console.print(
                        "[bold green]Output:[/bold green]\n" + output.strip()
                    )
                if error:
                    console.print(
                        "[bold red]Error:[/bold red]\n" + error.strip()
                    )

            elif args.command == "upload":
                ssh.upload_file(args.local_path, args.remote_path)

            elif args.command == "download":
                ssh.download_file(args.remote_path, args.local_path)

            elif args.command == "list":
                ssh.list_remote_directory(args.remote_path)

            elif args.command == "mkdir":
                ssh.create_remote_directory(args.remote_path)

            elif args.command == "delete":
                ssh.delete_remote_file(args.remote_path)

            else:
                logger.error("No valid command provided.")
                sys.exit(1)

    except SSHError as e:
        logger.critical(f"SSH operation failed: {e}")
        console = Console()
        console.print(f"[bold red]SSH operation failed: {e}[/bold red]")
        sys.exit(1)


if __name__ == "__main__":
    main()
