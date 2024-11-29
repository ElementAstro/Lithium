#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Enhanced FTP Client Script

This script provides an improved FTP client with support for asynchronous operations,
FTPS, SFTP protocols, enhanced logging with loguru, and beautified terminal output using rich.

Features:
- Asynchronous file transfers with asyncio and aioftp
- Support for FTP, FTPS, and SFTP protocols
- Enhanced logging with loguru
- Beautiful terminal output with rich
- Detailed inline comments and docstrings

Usage:
    python ftp_client.py --help

Author:
    Max Qian <lightapt.com>

License:
    GPL-3.0-or-later
"""

import sys
import asyncio
import argparse
import logging
from pathlib import Path
from typing import Optional, List

from loguru import logger
from rich.console import Console
from rich.table import Table
from rich.progress import Progress, BarColumn, TextColumn, TimeRemainingColumn
from rich.prompt import Prompt
from rich.logging import RichHandler

import aioftp  # Asynchronous FTP client library
import asyncssh  # Asynchronous SSH client library for SFTP

# Configure loguru and rich logging
logger.remove()
console = Console()
logger.add(RichHandler(console=console), level="INFO", format="{message}")

# Set asyncio event loop policy for Windows compatibility
if sys.platform.startswith('win'):
    asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())


class AsyncFTPClient:
    """
    Asynchronous FTP client supporting FTP, FTPS, and SFTP protocols.
    """

    def __init__(self, host: str, port: int = 21, username: str = 'anonymous',
                 password: str = '', protocol: str = 'ftp', secure: bool = False):
        self.host = host
        self.port = port
        self.username = username
        self.password = password
        self.protocol = protocol.lower()
        self.secure = secure
        self.connection = None

    async def connect(self):
        """
        Establish a connection to the server based on the specified protocol.
        """
        logger.info(
            f"Connecting to {self.protocol.upper()} server: {self.host}:{self.port}")
        if self.protocol == 'ftp':
            tls = self.secure
            self.connection = aioftp.ClientContext(
                self.host, self.port, self.username, self.password, ssl=tls)
            await self.connection.ensure_connected()
        elif self.protocol == 'sftp':
            self.connection = await asyncssh.connect(
                self.host, port=self.port, username=self.username, password=self.password)
        else:
            logger.error(f"Unsupported protocol: {self.protocol}")
            sys.exit(1)
        logger.info("Connection established successfully.")

    async def disconnect(self):
        """
        Close the connection to the server.
        """
        if self.connection:
            if self.protocol == 'ftp':
                await self.connection.quit()
            elif self.protocol == 'sftp':
                self.connection.close()
                await self.connection.wait_closed()
            logger.info("Disconnected from server.")

    async def list_files(self, path: str = '.', recursive: bool = False) -> List[dict]:
        """
        List files and directories at the specified path.

        Args:
            path (str): Remote directory path.
            recursive (bool): Whether to list files recursively.

        Returns:
            List[dict]: List of file information dictionaries.
        """
        files = []
        if self.protocol == 'ftp':
            async for path_obj, info in self.connection.list(
                    path, recursive=recursive):
                file_info = {
                    'name': path_obj.name,
                    'path': str(path_obj),
                    'type': 'dir' if info['type'] == 'dir' else 'file',
                    'size': info.get('size', 0),
                }
                files.append(file_info)
        elif self.protocol == 'sftp':
            async with self.connection.start_sftp_client() as sftp:
                async for item in sftp.scandir(path):
                    file_info = {
                        'name': item.filename,
                        'path': f"{path}/{item.filename}",
                        'type': 'dir' if item.longname.startswith('d') else 'file',
                        'size': item.attrs.size,
                    }
                    files.append(file_info)
        return files

    async def download_file(self, remote_path: str, local_path: str):
        """
        Download a file from the server.

        Args:
            remote_path (str): Remote file path.
            local_path (str): Local file path.
        """
        logger.info(f"Downloading {remote_path} to {local_path}")
        if self.protocol == 'ftp':
            size = await self.connection.stat(remote_path)
            total_size = size['size']
            with open(local_path, 'wb') as f, Progress(
                TextColumn("[progress.description]{task.description}"),
                BarColumn(),
                "[progress.percentage]{task.percentage:>3.1f}%",
                "•",
                "[green]{task.completed} of {task.total} bytes",
                "•",
                TimeRemainingColumn(),
                console=console,
            ) as progress:
                task = progress.add_task("Downloading...", total=total_size)
                async with self.connection.download_stream(remote_path) as stream:
                    async for block in stream.iter_by_block():
                        f.write(block)
                        progress.update(task, advance=len(block))
        elif self.protocol == 'sftp':
            async with self.connection.start_sftp_client() as sftp:
                size = (await sftp.stat(remote_path)).size
                with open(local_path, 'wb') as f, Progress(
                    TextColumn("[progress.description]{task.description}"),
                    BarColumn(),
                    "[progress.percentage]{task.percentage:>3.1f}%",
                    "•",
                    "[green]{task.completed} of {task.total} bytes",
                    "•",
                    TimeRemainingColumn(),
                    console=console,
                ) as progress:
                    task = progress.add_task("Downloading...", total=size)
                    async with sftp.open(remote_path, 'rb') as remote_file:
                        while True:
                            data = await remote_file.read(32768)
                            if not data:
                                break
                            f.write(data)
                            progress.update(task, advance=len(data))
        logger.info("Download completed successfully.")

    async def upload_file(self, local_path: str, remote_path: str):
        """
        Upload a file to the server.

        Args:
            local_path (str): Local file path.
            remote_path (str): Remote file path.
        """
        logger.info(f"Uploading {local_path} to {remote_path}")
        size = Path(local_path).stat().st_size
        if self.protocol == 'ftp':
            with open(local_path, 'rb') as f, Progress(
                TextColumn("[progress.description]{task.description}"),
                BarColumn(),
                "[progress.percentage]{task.percentage:>3.1f}%",
                "•",
                "[green]{task.completed} of {task.total} bytes",
                "•",
                TimeRemainingColumn(),
                console=console,
            ) as progress:
                task = progress.add_task("Uploading...", total=size)
                async with self.connection.upload_stream(remote_path) as stream:
                    while True:
                        data = f.read(32768)
                        if not data:
                            break
                        await stream.write(data)
                        progress.update(task, advance=len(data))
        elif self.protocol == 'sftp':
            async with self.connection.start_sftp_client() as sftp:
                with open(local_path, 'rb') as f, Progress(
                    TextColumn("[progress.description]{task.description}"),
                    BarColumn(),
                    "[progress.percentage]{task.percentage:>3.1f}%",
                    "•",
                    "[green]{task.completed} of {task.total} bytes",
                    "•",
                    TimeRemainingColumn(),
                    console=console,
                ) as progress:
                    task = progress.add_task("Uploading...", total=size)
                    async with sftp.open(remote_path, 'wb') as remote_file:
                        while True:
                            data = f.read(32768)
                            if not data:
                                break
                            await remote_file.write(data)
                            progress.update(task, advance=len(data))
        logger.info("Upload completed successfully.")

    async def delete_file(self, remote_path: str):
        """
        Delete a file on the server.

        Args:
            remote_path (str): Remote file path.
        """
        if self.protocol == 'ftp':
            await self.connection.remove_file(remote_path)
        elif self.protocol == 'sftp':
            async with self.connection.start_sftp_client() as sftp:
                await sftp.remove(remote_path)
        logger.info(f"Deleted file: {remote_path}")

    async def make_directory(self, remote_path: str):
        """
        Create a directory on the server.

        Args:
            remote_path (str): Remote directory path.
        """
        if self.protocol == 'ftp':
            await self.connection.make_directory(remote_path)
        elif self.protocol == 'sftp':
            async with self.connection.start_sftp_client() as sftp:
                await sftp.mkdir(remote_path)
        logger.info(f"Created directory: {remote_path}")

    async def remove_directory(self, remote_path: str):
        """
        Remove a directory on the server.

        Args:
            remote_path (str): Remote directory path.
        """
        if self.protocol == 'ftp':
            await self.connection.remove_directory(remote_path)
        elif self.protocol == 'sftp':
            async with self.connection.start_sftp_client() as sftp:
                await sftp.rmdir(remote_path)
        logger.info(f"Removed directory: {remote_path}")

    async def rename(self, old_path: str, new_path: str):
        """
        Rename a file or directory on the server.

        Args:
            old_path (str): Original path.
            new_path (str): New path.
        """
        if self.protocol == 'ftp':
            await self.connection.rename(old_path, new_path)
        elif self.protocol == 'sftp':
            async with self.connection.start_sftp_client() as sftp:
                await sftp.rename(old_path, new_path)
        logger.info(f"Renamed {old_path} to {new_path}")

    async def get_current_directory(self) -> str:
        """
        Get the current working directory on the server.

        Returns:
            str: Current directory path.
        """
        if self.protocol == 'ftp':
            path = await self.connection.get_current_directory()
        elif self.protocol == 'sftp':
            # SFTP does not have a concept of current directory in the same way
            path = "."
        logger.info(f"Current directory: {path}")
        return path

    async def change_directory(self, remote_path: str):
        """
        Change the current working directory on the server.

        Args:
            remote_path (str): Remote directory path.
        """
        if self.protocol == 'ftp':
            await self.connection.change_directory(remote_path)
            logger.info(f"Changed directory to: {remote_path}")
        elif self.protocol == 'sftp':
            # SFTP does not support changing directory in the same way
            logger.warning("SFTP does not support changing current directory.")

    async def batch_transfer(self, operations: List[dict]):
        """
        Perform batch upload/download operations.

        Args:
            operations (List[dict]): List of operation dictionaries.
        """
        tasks = []
        for op in operations:
            if op['action'] == 'upload':
                tasks.append(self.upload_file(op['local'], op['remote']))
            elif op['action'] == 'download':
                tasks.append(self.download_file(op['remote'], op['local']))
        await asyncio.gather(*tasks)
        logger.info("Batch transfer completed.")


def create_parser() -> argparse.ArgumentParser:
    """
    Create an argument parser for command-line options.

    Returns:
        argparse.ArgumentParser: Configured argument parser.
    """
    parser = argparse.ArgumentParser(
        description='Enhanced FTP client command line tool')
    parser.add_argument('--host', required=True, help='Server address')
    parser.add_argument('--port', type=int, help='Server port')
    parser.add_argument('--username', default='anonymous', help='Username')
    parser.add_argument('--password', default='', help='Password')
    parser.add_argument('--protocol', default='ftp',
                        choices=['ftp', 'sftp'], help='Protocol to use')
    parser.add_argument('--secure', action='store_true',
                        help='Enable FTPS (for FTP protocol)')

    subparsers = parser.add_subparsers(
        dest='command', help='Available commands')

    # ls command
    ls_parser = subparsers.add_parser('ls', help='List files')
    ls_parser.add_argument('--path', default='.', help='Remote path to list')
    ls_parser.add_argument(
        '--recursive', action='store_true', help='List files recursively')

    # get command
    get_parser = subparsers.add_parser('get', help='Download file')
    get_parser.add_argument('remote_path', help='Remote file path')
    get_parser.add_argument('local_path', help='Local file path')

    # put command
    put_parser = subparsers.add_parser('put', help='Upload file')
    put_parser.add_argument('local_path', help='Local file path')
    put_parser.add_argument('remote_path', help='Remote file path')

    # rm command
    rm_parser = subparsers.add_parser('rm', help='Delete file')
    rm_parser.add_argument('remote_path', help='Remote file path to delete')

    # mkdir command
    mkdir_parser = subparsers.add_parser('mkdir', help='Create directory')
    mkdir_parser.add_argument(
        'remote_path', help='Remote directory path to create')

    # rmdir command
    rmdir_parser = subparsers.add_parser('rmdir', help='Remove directory')
    rmdir_parser.add_argument(
        'remote_path', help='Remote directory path to remove')

    # rename command
    rename_parser = subparsers.add_parser(
        'rename', help='Rename file or directory')
    rename_parser.add_argument('old_path', help='Original remote path')
    rename_parser.add_argument('new_path', help='New remote path')

    # pwd command
    subparsers.add_parser('pwd', help='Show current directory')

    # cd command
    cd_parser = subparsers.add_parser('cd', help='Change directory')
    cd_parser.add_argument(
        'remote_path', help='Remote directory path to change to')

    # batch command
    batch_parser = subparsers.add_parser(
        'batch', help='Batch upload/download files')
    batch_parser.add_argument(
        'operations_file', help='Path to operations JSON file')

    return parser


async def main():
    parser = create_parser()
    args = parser.parse_args()

    if not args.command:
        parser.print_help()
        return

    client = AsyncFTPClient(
        host=args.host,
        port=args.port or (22 if args.protocol == 'sftp' else 21),
        username=args.username,
        password=args.password,
        protocol=args.protocol,
        secure=args.secure
    )

    try:
        await client.connect()
        if args.command == 'ls':
            files = await client.list_files(args.path, args.recursive)
            table = Table(show_header=True, header_style="bold blue")
            table.add_column("Type")
            table.add_column("Size", justify="right")
            table.add_column("Path")
            for file in files:
                icon = "[blue][DIR][/blue]" if file['type'] == 'dir' else "[green][FILE][/green]"
                size = str(file['size'])
                path = file['path']
                table.add_row(icon, size, path)
            console.print(table)
        elif args.command == 'get':
            await client.download_file(args.remote_path, args.local_path)
        elif args.command == 'put':
            await client.upload_file(args.local_path, args.remote_path)
        elif args.command == 'rm':
            await client.delete_file(args.remote_path)
        elif args.command == 'mkdir':
            await client.make_directory(args.remote_path)
        elif args.command == 'rmdir':
            await client.remove_directory(args.remote_path)
        elif args.command == 'rename':
            await client.rename(args.old_path, args.new_path)
        elif args.command == 'pwd':
            await client.get_current_directory()
        elif args.command == 'cd':
            await client.change_directory(args.remote_path)
        elif args.command == 'batch':
            import json
            with open(args.operations_file, 'r') as f:
                operations = json.load(f)
            await client.batch_transfer(operations)
        else:
            logger.error(f"Unknown command: {args.command}")
    except Exception as e:
        logger.exception(f"An error occurred: {e}")
    finally:
        await client.disconnect()


if __name__ == '__main__':
    asyncio.run(main())
