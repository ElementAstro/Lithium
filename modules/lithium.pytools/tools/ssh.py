import paramiko
from paramiko.ssh_exception import SSHException, AuthenticationException, NoValidConnectionsError
from pathlib import Path
from typing import Union, List, Optional, Tuple
from loguru import logger
import sys
import argparse


# Define custom exception classes
class SSHError(Exception):
    """Base exception class for SSHClient"""


class SSHConnectionError(SSHError):
    """Exception for SSH connection failures"""


class SSHCommandError(SSHError):
    """Exception for SSH command execution failures"""


class SSHPermissionError(SSHError):
    """Exception for SSH permission related errors"""


class SFTPError(SSHError):
    """Exception for SFTP operation failures"""


class SSHClient:
    def __init__(
        self,
        hostname: str,
        port: int,
        username: str,
        password: Optional[str] = None,
        key_file: Optional[str] = None,
    ):
        """
        Initialize the SSH client.
        :param hostname: Hostname or IP address.
        :param port: Port number, typically 22.
        :param username: Username for SSH login.
        :param password: Password for SSH (if using password authentication).
        :param key_file: Path to the private key file (if using key authentication).
        """
        self.hostname = hostname
        self.port = port
        self.username = username
        self.password = password
        self.key_file = key_file
        self.client = None
        self.sftp = None

    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def connect(self):
        """
        Connect to the remote SSH server.
        """
        logger.info(
            f"Attempting to connect to {self.hostname}:{self.port} as {self.username}"
        )
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
            }
            self.client.connect(**connect_params)
            self.sftp = self.client.open_sftp()
            logger.success(
                f"Successfully connected to {self.hostname}:{self.port}")
        except AuthenticationException as e:
            logger.error(f"Authentication failed: {e}")
            raise SSHConnectionError("Authentication failed") from e
        except NoValidConnectionsError as e:
            logger.error(
                f"Unable to connect to {self.hostname}:{self.port}: {e}")
            raise SSHConnectionError(
                f"Unable to connect to {self.hostname}:{self.port}") from e
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
        :param command: The command to execute.
        :param timeout: Optional timeout for command execution.
        :return: A tuple containing standard output and standard error.
        """
        if self.client is None:
            logger.error(
                "Not connected to any SSH server. Call connect() first.")
            raise SSHCommandError("Not connected to any SSH server.")

        logger.info(f"Executing command: {command}")
        try:
            stdin, stdout, stderr = self.client.exec_command(
                command, timeout=timeout)
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
                f"Failed to execute command '{command}'") from e
        except Exception as e:
            logger.error(f"Unexpected error during command execution: {e}")
            raise SSHCommandError(
                "Unexpected error during command execution") from e

    def upload_file(
        self, local_path: Union[str, Path], remote_path: Union[str, Path]
    ):
        """
        Upload a file to the remote SSH server using SFTP.
        :param local_path: Path to the local file.
        :param remote_path: Path on the remote server where the file will be uploaded.
        """
        if self.sftp is None:
            logger.error(
                "SFTP session is not established. Call connect() first.")
            raise SFTPError("SFTP session is not established.")

        logger.info(f"Uploading {local_path} to {remote_path}")
        try:
            self.sftp.put(str(local_path), str(remote_path))
            logger.success(
                f"Successfully uploaded {local_path} to {remote_path}")
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
        :param remote_path: Path on the remote server to download.
        :param local_path: Path on the local machine where the file will be saved.
        """
        if self.sftp is None:
            logger.error(
                "SFTP session is not established. Call connect() first.")
            raise SFTPError("SFTP session is not established.")

        logger.info(f"Downloading {remote_path} to {local_path}")
        try:
            self.sftp.get(str(remote_path), str(local_path))
            logger.success(
                f"Successfully downloaded {remote_path} to {local_path}")
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
        :param remote_path: Path on the remote server to list.
        :return: A list of file and directory names.
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
        :param remote_path: Path on the remote server where the directory will be created.
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
        except FileExistsError:
            logger.warning(f"Remote directory already exists: {remote_path}")
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
        :param remote_path: Path on the remote server of the file to delete.
        """
        if self.sftp is None:
            logger.error(
                "SFTP session is not established. Call connect() first.")
            raise SFTPError("SFTP session is not established.")

        logger.info(f"Deleting remote file: {remote_path}")
        try:
            self.sftp.remove(str(remote_path))
            logger.success(f"Successfully deleted remote file: {remote_path}")
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
        except Exception as e:
            logger.error(f"Error while closing connections: {e}")
            raise SSHError("Error while closing connections") from e


def parse_args():
    """
    Parse command-line arguments.
    :return: Parsed arguments.
    """
    parser = argparse.ArgumentParser(description="SSH Client Tool")
    parser.add_argument("--hostname", required=True,
                        help="Hostname or IP address")
    parser.add_argument("--port", type=int, default=22, help="SSH port number")
    parser.add_argument("--username", required=True, help="SSH username")

    auth_group = parser.add_mutually_exclusive_group(required=True)
    auth_group.add_argument("--password", help="SSH password")
    auth_group.add_argument("--key_file", help="Path to SSH private key file")

    subparsers = parser.add_subparsers(
        dest="command", help="Available commands")

    # Execute command
    exec_parser = subparsers.add_parser(
        "exec", help="Execute a command on the remote server")
    exec_parser.add_argument("cmd", help="Command to execute")
    exec_parser.add_argument("--timeout", type=float,
                             default=None, help="Command timeout in seconds")

    # Upload file
    upload_parser = subparsers.add_parser(
        "upload", help="Upload a file to the remote server")
    upload_parser.add_argument("local_path", help="Path to the local file")
    upload_parser.add_argument("remote_path", help="Path on the remote server")

    # Download file
    download_parser = subparsers.add_parser(
        "download", help="Download a file from the remote server")
    download_parser.add_argument(
        "remote_path", help="Path on the remote server")
    download_parser.add_argument(
        "local_path", help="Path to save the downloaded file locally")

    # List directory
    list_parser = subparsers.add_parser(
        "list", help="List contents of a remote directory")
    list_parser.add_argument("remote_path", help="Remote directory path")

    # Create directory
    mkdir_parser = subparsers.add_parser(
        "mkdir", help="Create a directory on the remote server")
    mkdir_parser.add_argument(
        "remote_path", help="Path of the remote directory to create")

    # Delete file
    delete_parser = subparsers.add_parser(
        "delete", help="Delete a file on the remote server")
    delete_parser.add_argument(
        "remote_path", help="Path of the remote file to delete")

    return parser.parse_args()


def main():
    """
    Main function to execute the SSH client operations based on command-line arguments.
    """
    args = parse_args()

    try:
        with SSHClient(
            hostname=args.hostname,
            port=args.port,
            username=args.username,
            password=args.password,
            key_file=args.key_file,
        ) as ssh:
            if args.command == "exec":
                output, error = ssh.execute_command(
                    args.cmd, timeout=args.timeout)
                if output:
                    print("Output:\n", output)
                if error:
                    print("Error:\n", error)

            elif args.command == "upload":
                ssh.upload_file(args.local_path, args.remote_path)

            elif args.command == "download":
                ssh.download_file(args.remote_path, args.local_path)

            elif args.command == "list":
                contents = ssh.list_remote_directory(args.remote_path)
                print("Directory Contents:")
                for item in contents:
                    print(item)

            elif args.command == "mkdir":
                ssh.create_remote_directory(args.remote_path)

            elif args.command == "delete":
                ssh.delete_remote_file(args.remote_path)

            else:
                logger.error("No valid command provided.")
                sys.exit(1)

    except SSHError as e:
        logger.critical(f"SSH operation failed: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
