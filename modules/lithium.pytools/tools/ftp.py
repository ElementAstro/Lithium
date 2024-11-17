import os
import sys
import argparse
import threading
import queue
from typing import Optional, List
from datetime import datetime
from ftplib import FTP, error_perm, all_errors
from tqdm import tqdm
from concurrent.futures import ThreadPoolExecutor
from loguru import logger

# Set up logging
logger.add("ftp_client.log", format="{time} {level} {message}", level="INFO")
logger.add(sys.stdout, format="{time} {level} {message}", level="INFO")


class FTPClient:
    def __init__(self, host: str, username: str = 'anonymous',
                 password: str = '', port: int = 21, timeout: int = 30):
        self.host = host
        self.username = username
        self.password = password
        self.port = port
        self.timeout = timeout
        self.ftp: Optional[FTP] = None
        self._is_connected = False

    def connect(self) -> bool:
        try:
            self.ftp = FTP()
            self.ftp.connect(self.host, self.port, self.timeout)
            self.ftp.login(self.username, self.password)
            self._is_connected = True
            logger.info(f"Connected to FTP server: {self.host}")
            return True
        except all_errors as e:
            logger.error(f"Connection failed: {e}")
            return False

    def __enter__(self):
        if not self._is_connected:
            self.connect()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.disconnect()

    def disconnect(self):
        if self.ftp and self._is_connected:
            try:
                self.ftp.quit()
                logger.info("FTP connection closed")
            except all_errors as e:
                logger.error(f"Error while disconnecting: {e}")
            finally:
                self._is_connected = False

    def list_files(self, path: str = '.', recursive: bool = False) -> List[dict]:
        files = []
        try:
            if recursive:
                self._list_recursive(path, files)
            else:
                entries = []
                self.ftp.dir(path, entries.append)
                for entry in entries:
                    parts = entry.split(maxsplit=8)
                    if len(parts) < 9:
                        continue
                    name = parts[8]
                    file_info = {
                        'name': name,
                        'size': parts[4],
                        'date': ' '.join(parts[5:8]),
                        'permissions': parts[0],
                        'type': 'dir' if parts[0].startswith('d') else 'file'
                    }
                    files.append(file_info)
            return files
        except error_perm as e:
            logger.error(f"Failed to list files: {e}")
            return []

    def _list_recursive(self, path: str, files: list):
        try:
            current_files = self.list_files(path)
            for file_info in current_files:
                full_path = f"{path}/{file_info['name']}"
                file_info['path'] = full_path
                files.append(file_info)
                if file_info['type'] == 'dir':
                    self._list_recursive(full_path, files)
        except error_perm as e:
            logger.error(f"Failed to list files recursively: {e}")

    def download_file(self, remote_filename: str, local_filename: Optional[str] = None,
                      callback: Optional[callable] = None, resume: bool = False):
        local_filename = local_filename or os.path.basename(remote_filename)
        try:
            existing_size = os.path.getsize(
                local_filename) if resume and os.path.exists(local_filename) else 0
            total_size = self.ftp.size(remote_filename)
            if resume and existing_size < total_size:
                with open(local_filename, 'ab') as f:
                    with tqdm(total=total_size, unit='B', unit_scale=True, desc=f"Downloading {remote_filename}", initial=existing_size) as pbar:
                        def _callback(data):
                            f.write(data)
                            pbar.update(len(data))
                            if callback:
                                callback(len(data))

                        self.ftp.retrbinary(
                            f"RETR {remote_filename}", _callback, rest=existing_size)
            else:
                with open(local_filename, 'wb') as f:
                    with tqdm(total=total_size, unit='B', unit_scale=True, desc=f"Downloading {remote_filename}") as pbar:
                        def _callback(data):
                            f.write(data)
                            pbar.update(len(data))
                            if callback:
                                callback(len(data))

                        self.ftp.retrbinary(
                            f"RETR {remote_filename}", _callback)
            logger.info(f"File {remote_filename} downloaded successfully")
            return True
        except all_errors as e:
            logger.error(f"Error downloading file: {e}")
            return False

    def upload_file(self, local_filename: str, remote_filename: Optional[str] = None,
                    callback: Optional[callable] = None):
        remote_filename = remote_filename or os.path.basename(local_filename)
        try:
            file_size = os.path.getsize(local_filename)
            with open(local_filename, 'rb') as f:
                with tqdm(total=file_size, unit='B', unit_scale=True,
                          desc=f"Uploading {local_filename}") as pbar:
                    def _callback(data):
                        pbar.update(len(data))
                        if callback:
                            callback(len(data))

                    self.ftp.storbinary(
                        f"STOR {remote_filename}", f, 8192, _callback)
            logger.info(f"File {local_filename} uploaded successfully")
            return True
        except all_errors as e:
            logger.error(f"Error uploading file: {e}")
            return False

    def batch_transfer(self, operations: List[dict], max_workers: int = 5):
        with ThreadPoolExecutor(max_workers=max_workers) as executor:
            futures = []
            for op in operations:
                if op['type'] == 'upload':
                    futures.append(
                        executor.submit(self.upload_file, op['source'], op.get('dest')))
                elif op['type'] == 'download':
                    futures.append(
                        executor.submit(self.download_file, op['source'], op.get('dest')))

            for future in futures:
                try:
                    future.result()
                except Exception as e:
                    logger.error(f"Transfer operation failed: {e}")

    def delete_file(self, filename: str):
        try:
            self.ftp.delete(filename)
            logger.info(f"File deleted: {filename}")
            return True
        except all_errors as e:
            logger.error(f"Error deleting file: {e}")
            return False

    def change_directory(self, path: str):
        try:
            self.ftp.cwd(path)
            logger.info(f"Changed directory to: {path}")
            return True
        except all_errors as e:
            logger.error(f"Error changing directory: {e}")
            return False

    def make_directory(self, dirname: str):
        try:
            self.ftp.mkd(dirname)
            logger.info(f"Directory created: {dirname}")
            return True
        except all_errors as e:
            logger.error(f"Error creating directory: {e}")
            return False

    def get_current_directory(self) -> str:
        try:
            return self.ftp.pwd()
        except all_errors as e:
            logger.error(f"Failed to get current directory: {e}")
            return ""

    def rename_file(self, from_name: str, to_name: str) -> bool:
        try:
            self.ftp.rename(from_name, to_name)
            logger.info(f"File renamed: {from_name} -> {to_name}")
            return True
        except all_errors as e:
            logger.error(f"Error renaming file: {e}")
            return False


def create_parser():
    parser = argparse.ArgumentParser(
        description='FTP client command line tool')
    parser.add_argument('--host', required=True, help='FTP server address')
    parser.add_argument('--port', type=int, default=21, help='FTP server port')
    parser.add_argument('--username', default='anonymous', help='Username')
    parser.add_argument('--password', default='', help='Password')

    subparsers = parser.add_subparsers(
        dest='command', help='Available commands')

    # ls command
    ls_parser = subparsers.add_parser('ls', help='List files')
    ls_parser.add_argument('--path', default='.', help='Path to list')
    ls_parser.add_argument(
        '--recursive', action='store_true', help='List files recursively')

    # get command
    get_parser = subparsers.add_parser('get', help='Download file')
    get_parser.add_argument('remote_file', help='Remote file path')
    get_parser.add_argument('--local-file', help='Local file path')
    get_parser.add_argument(
        '--resume', action='store_true', help='Resume download')

    # put command
    put_parser = subparsers.add_parser('put', help='Upload file')
    put_parser.add_argument('local_file', help='Local file path')
    put_parser.add_argument('--remote-file', help='Remote file path')

    # rm command
    rm_parser = subparsers.add_parser('rm', help='Delete file')
    rm_parser.add_argument('filename', help='Filename to delete')

    # cd command
    cd_parser = subparsers.add_parser('cd', help='Change directory')
    cd_parser.add_argument('path', help='Target directory path')

    # mkdir command
    mkdir_parser = subparsers.add_parser('mkdir', help='Create directory')
    mkdir_parser.add_argument('dirname', help='Directory name to create')

    # pwd command
    subparsers.add_parser('pwd', help='Show current directory')

    # rename command
    rename_parser = subparsers.add_parser('rename', help='Rename file')
    rename_parser.add_argument('from_name', help='Original filename')
    rename_parser.add_argument('to_name', help='New filename')

    # batch command
    batch_parser = subparsers.add_parser(
        'batch', help='Batch upload/download files')
    batch_parser.add_argument(
        'operations_file', help='Path to operations list file')

    return parser


def main():
    parser = create_parser()
    args = parser.parse_args()

    if not args.command:
        parser.print_help()
        return

    with FTPClient(args.host, args.username, args.password, args.port) as client:
        if not client._is_connected:
            return

        if args.command == 'ls':
            files = client.list_files(args.path, args.recursive)
            for file in files:
                print(
                    f"{file['permissions']} {file['size']:>8} {file['date']} {file['name']}")

        elif args.command == 'get':
            client.download_file(
                args.remote_file, args.local_file, resume=args.resume)

        elif args.command == 'put':
            client.upload_file(args.local_file, args.remote_file)

        elif args.command == 'rm':
            client.delete_file(args.filename)

        elif args.command == 'cd':
            client.change_directory(args.path)

        elif args.command == 'mkdir':
            client.make_directory(args.dirname)

        elif args.command == 'pwd':
            print(client.get_current_directory())

        elif args.command == 'rename':
            client.rename_file(args.from_name, args.to_name)

        elif args.command == 'batch':
            try:
                with open(args.operations_file, 'r') as f:
                    operations = [eval(line.strip())
                                  for line in f if line.strip()]
                client.batch_transfer(operations)
            except Exception as e:
                logger.error(f"Batch operation failed: {e}")


if __name__ == '__main__':
    main()
