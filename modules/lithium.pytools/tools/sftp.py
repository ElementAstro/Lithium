from datetime import datetime
import os
import argparse
import stat
import sys
import paramiko
from paramiko import SFTPClient, SSHException
from loguru import logger

# Configure Loguru logger
logger.add("sftp_client.log", format="{time} {level} {message}", level="INFO")
logger.add(sys.stdout, format="{time} {level} {message}", level="INFO")


class SFTPClientWrapper:
    def __init__(self, hostname, username, password=None, port=22, key_file=None):
        """
        Initialize the SFTP client wrapper.

        :param hostname: The hostname of the SFTP server.
        :param username: The username to connect to the SFTP server.
        :param password: The password to connect to the SFTP server (optional if using key_file).
        :param port: The port to connect to the SFTP server (default is 22).
        :param key_file: The path to the private key file for key-based authentication (optional).
        """
        self.hostname = hostname
        self.username = username
        self.password = password
        self.port = port
        self.key_file = key_file
        self.sftp = None
        self.client = None

    def connect(self):
        """
        Connect to the SFTP server using the provided credentials.
        """
        try:
            self.client = paramiko.SSHClient()
            self.client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            if self.key_file:
                private_key = paramiko.RSAKey.from_private_key_file(
                    self.key_file)
                self.client.connect(
                    hostname=self.hostname,
                    port=self.port,
                    username=self.username,
                    pkey=private_key
                )
            else:
                self.client.connect(
                    hostname=self.hostname,
                    port=self.port,
                    username=self.username,
                    password=self.password
                )
            self.sftp = self.client.open_sftp()
            logger.info(f"Connected to SFTP server: {self.hostname}")
        except (SSHException, Exception) as e:
            logger.error(f"Failed to connect to SFTP server: {e}")
            self.disconnect()
            raise e

    def upload_file(self, local_path, remote_path):
        """
        Upload a single file to the SFTP server.

        :param local_path: The local file path to upload.
        :param remote_path: The remote file path to upload to.
        """
        try:
            self.sftp.put(local_path, remote_path)
            logger.info(f"Uploaded: {local_path} -> {remote_path}")
        except Exception as e:
            logger.error(f"Failed to upload file '{local_path}': {e}")

    def download_file(self, remote_path, local_path):
        """
        Download a single file from the SFTP server.

        :param remote_path: The remote file path to download.
        :param local_path: The local file path to download to.
        """
        try:
            self.sftp.get(remote_path, local_path)
            logger.info(f"Downloaded: {remote_path} -> {local_path}")
        except Exception as e:
            logger.error(f"Failed to download file '{remote_path}': {e}")

    def upload_directory(self, local_dir, remote_dir):
        """
        Recursively upload a directory to the SFTP server.

        :param local_dir: The local directory path to upload.
        :param remote_dir: The remote directory path to upload to.
        """
        try:
            for root, _, files in os.walk(local_dir):
                rel_path = os.path.relpath(root, local_dir)
                rel_path = "" if rel_path == "." else rel_path
                remote_sub_dir = os.path.join(
                    remote_dir, rel_path).replace("\\", "/")
                self.create_directory(remote_sub_dir)
                for file in files:
                    local_file = os.path.join(root, file)
                    remote_file = os.path.join(
                        remote_sub_dir, file).replace("\\", "/")
                    self.upload_file(local_file, remote_file)
            logger.info(f"Uploaded directory: {local_dir} -> {remote_dir}")
        except Exception as e:
            logger.error(f"Failed to upload directory '{local_dir}': {e}")

    def download_directory(self, remote_dir, local_dir):
        """
        Recursively download a directory from the SFTP server.

        :param remote_dir: The remote directory path to download.
        :param local_dir: The local directory path to download to.
        """
        try:
            os.makedirs(local_dir, exist_ok=True)
            for item in self.sftp.listdir_attr(remote_dir):
                remote_item = os.path.join(
                    remote_dir, item.filename).replace("\\", "/")
                local_item = os.path.join(local_dir, item.filename)
                if stat.S_ISDIR(item.st_mode):
                    self.download_directory(remote_item, local_item)
                else:
                    self.download_file(remote_item, local_item)
            logger.info(f"Downloaded directory: {remote_dir} -> {local_dir}")
        except Exception as e:
            logger.error(f"Failed to download directory '{remote_dir}': {e}")

    def create_directory(self, remote_path):
        """
        Create a remote directory on the SFTP server.

        :param remote_path: The remote directory path to create.
        """
        try:
            self.sftp.mkdir(remote_path)
            logger.info(f"Created directory: {remote_path}")
        except IOError:
            logger.warning(
                f"Directory already exists or cannot be created: {remote_path}")
        except Exception as e:
            logger.error(f"Failed to create directory '{remote_path}': {e}")

    def remove_directory(self, remote_path):
        """
        Recursively remove a remote directory from the SFTP server.

        :param remote_path: The remote directory path to remove.
        """
        try:
            for item in self.sftp.listdir_attr(remote_path):
                remote_item = os.path.join(
                    remote_path, item.filename).replace("\\", "/")
                if stat.S_ISDIR(item.st_mode):
                    self.remove_directory(remote_item)
                else:
                    self.sftp.remove(remote_item)
                    logger.info(f"Removed file: {remote_item}")
            self.sftp.rmdir(remote_path)
            logger.info(f"Removed directory: {remote_path}")
        except Exception as e:
            logger.error(f"Failed to remove directory '{remote_path}': {e}")

    def get_file_info(self, remote_path):
        """
        Get information about a remote file or directory.

        :param remote_path: The remote file or directory path.
        """
        try:
            info = self.sftp.stat(remote_path)
            file_type = 'Directory' if stat.S_ISDIR(info.st_mode) else 'File'
            logger.info(f"Information for {remote_path}:")
            logger.info(f"  Type: {file_type}")
            logger.info(f"  Size: {info.st_size} bytes")
            logger.info(
                f"  Last modified: {datetime.fromtimestamp(info.st_mtime)}")
            logger.info(f"  Permissions: {oct(info.st_mode)}")
        except FileNotFoundError:
            logger.error(f"Path not found: {remote_path}")
        except Exception as e:
            logger.error(f"Failed to get file info for '{remote_path}': {e}")

    def resume_upload(self, local_path, remote_path):
        """
        Resume an interrupted file upload to the SFTP server.

        :param local_path: The local file path to upload.
        :param remote_path: The remote file path to upload to.
        """
        try:
            file_size = os.path.getsize(local_path)
            try:
                remote_size = self.sftp.stat(remote_path).st_size
            except FileNotFoundError:
                remote_size = 0

            if remote_size < file_size:
                with open(local_path, "rb") as f:
                    f.seek(remote_size)
                    self.sftp.putfo(f, remote_path, file_size=file_size,
                                    callback=self._progress_callback(remote_size, file_size))
                logger.info(f"Resumed upload: {local_path} -> {remote_path}")
            else:
                logger.info(f"File already fully uploaded: {remote_path}")
        except Exception as e:
            logger.error(f"Failed to resume upload '{local_path}': {e}")

    def _progress_callback(self, initial, total):
        """
        Internal method to provide a progress callback.

        :param initial: The initial amount of data transferred.
        :param total: The total size of the file being transferred.
        """
        def callback(transferred, total_size):
            progress = (initial + transferred) / total
            logger.info(f"Upload progress: {progress*100:.2f}%")
        return callback

    def list_files(self, remote_path):
        """
        List files in a remote directory on the SFTP server.

        :param remote_path: The remote directory path.
        :return: A list of files in the remote directory.
        """
        try:
            files = self.sftp.listdir(remote_path)
            logger.info(f"Files in '{remote_path}': {files}")
            return files
        except Exception as e:
            logger.error(f"Failed to list files in '{remote_path}': {e}")
            return []

    def move_file(self, remote_src, remote_dest):
        """
        Move or rename a remote file on the SFTP server.

        :param remote_src: The source remote file path.
        :param remote_dest: The destination remote file path.
        """
        try:
            self.sftp.rename(remote_src, remote_dest)
            logger.info(f"Moved/Renamed: {remote_src} -> {remote_dest}")
        except Exception as e:
            logger.error(
                f"Failed to move/rename '{remote_src}' to '{remote_dest}': {e}")

    def delete_file(self, remote_path):
        """
        Delete a remote file from the SFTP server.

        :param remote_path: The remote file path to delete.
        """
        try:
            self.sftp.remove(remote_path)
            logger.info(f"Deleted file: {remote_path}")
        except Exception as e:
            logger.error(f"Failed to delete file '{remote_path}': {e}")

    def path_exists(self, remote_path):
        """
        Check if a remote path exists on the SFTP server.

        :param remote_path: The remote path to check.
        :return: True if the path exists, False otherwise.
        """
        try:
            self.sftp.stat(remote_path)
            return True
        except FileNotFoundError:
            return False
        except Exception as e:
            logger.error(f"Error checking if path exists '{remote_path}': {e}")
            return False

    def disconnect(self):
        """
        Disconnect from the SFTP server.
        """
        try:
            if self.sftp:
                self.sftp.close()
                logger.info("SFTP connection closed")
            if self.client:
                self.client.close()
                logger.info("SSH client disconnected")
        except Exception as e:
            logger.error(f"Error while disconnecting: {e}")


def parse_arguments():
    """
    Parse command-line arguments.

    :return: Parsed arguments.
    """
    parser = argparse.ArgumentParser(
        description="SFTP Client Command Line Tool")
    parser.add_argument("hostname", help="SFTP server hostname")
    parser.add_argument("username", help="SFTP username")
    parser.add_argument("--password", help="SFTP password", default=None)
    parser.add_argument("--port", type=int,
                        help="SFTP server port", default=22)
    parser.add_argument(
        "--key-file", help="Path to private key file", default=None)

    subparsers = parser.add_subparsers(
        dest="command", help="Available commands")

    # Upload directory
    upload_dir_parser = subparsers.add_parser(
        "upload-dir", help="Upload a directory to the server")
    upload_dir_parser.add_argument("local_dir", help="Local directory path")
    upload_dir_parser.add_argument("remote_dir", help="Remote directory path")

    # Download directory
    download_dir_parser = subparsers.add_parser(
        "download-dir", help="Download a directory from the server")
    download_dir_parser.add_argument(
        "remote_dir", help="Remote directory path")
    download_dir_parser.add_argument("local_dir", help="Local directory path")

    # Create directory
    mkdir_parser = subparsers.add_parser(
        "mkdir", help="Create a directory on the server")
    mkdir_parser.add_argument("remote_path", help="Remote directory path")

    # Remove directory
    rmdir_parser = subparsers.add_parser(
        "rmdir", help="Remove a directory from the server")
    rmdir_parser.add_argument("remote_path", help="Remote directory path")

    # Get file info
    info_parser = subparsers.add_parser(
        "info", help="Get file or directory info")
    info_parser.add_argument(
        "remote_path", help="Remote file or directory path")

    # Resume upload
    resume_parser = subparsers.add_parser(
        "resume-upload", help="Resume an interrupted file upload")
    resume_parser.add_argument("local_path", help="Local file path")
    resume_parser.add_argument("remote_path", help="Remote file path")

    # List files
    list_parser = subparsers.add_parser(
        "list", help="List files in a remote directory")
    list_parser.add_argument("remote_path", help="Remote directory path")

    # Move file
    move_parser = subparsers.add_parser(
        "move", help="Move or rename a remote file")
    move_parser.add_argument("remote_src", help="Source remote path")
    move_parser.add_argument("remote_dest", help="Destination remote path")

    # Delete file
    delete_parser = subparsers.add_parser(
        "delete", help="Delete a remote file")
    delete_parser.add_argument("remote_path", help="Remote file path")

    return parser.parse_args()


def main():
    """
    Main function to execute the SFTP client operations based on command-line arguments.
    """
    args = parse_arguments()

    client = SFTPClientWrapper(
        hostname=args.hostname,
        username=args.username,
        password=args.password,
        port=args.port,
        key_file=args.key_file
    )

    try:
        client.connect()
        if args.command == "upload-dir":
            client.upload_directory(args.local_dir, args.remote_dir)
        elif args.command == "download-dir":
            client.download_directory(args.remote_dir, args.local_dir)
        elif args.command == "mkdir":
            client.create_directory(args.remote_path)
        elif args.command == "rmdir":
            client.remove_directory(args.remote_path)
        elif args.command == "info":
            client.get_file_info(args.remote_path)
        elif args.command == "resume-upload":
            client.resume_upload(args.local_path, args.remote_path)
        elif args.command == "list":
            client.list_files(args.remote_path)
        elif args.command == "move":
            client.move_file(args.remote_src, args.remote_dest)
        elif args.command == "delete":
            client.delete_file(args.remote_path)
        else:
            logger.warning(
                "Unknown command. Use --help to see available commands.")
    finally:
        client.disconnect()


if __name__ == "__main__":
    main()
