#!/usr/bin/env python3
"""
@file         net_framework_installer.py
@brief        A command-line utility to check for installed .NET Framework versions on Windows,
              download installers, and run them with multithreading and file verification capabilities.

@details      This script uses the Windows Registry to determine installed .NET Framework versions
              and provides functionality to download and execute installer files for missing versions
              using the asyncio library for concurrent downloads and checksum verification for file integrity.

              Usage:
                python net_framework_installer.py --list
                python net_framework_installer.py --check v4.0.30319
                python net_framework_installer.py --check v4.0.30319 --download [URL] --install [FILE_PATH] --threads 4 --checksum [SHA256]
                python net_framework_installer.py --uninstall v4.0.30319

@requires     - Python 3.7+
              - Windows operating system
              - `requests` Python library
              - `tqdm` Python library

@note         This script must be run with administrative privileges to install or uninstall .NET Framework versions.

@version      2.0
@date         2024-04-27
"""

import argparse
import hashlib
import subprocess
import os
from sys import platform, exit
import sys
import requests
import asyncio
import aiohttp
import aiofiles
from pathlib import Path
from typing import Optional
from loguru import logger
from tqdm import tqdm

# Define cache directory
CACHE_DIR = Path("./cache")
CACHE_DIR.mkdir(parents=True, exist_ok=True)


def setup_logging(log_file: Optional[Path] = None) -> None:
    """
    Configure Loguru for logging.

    Args:
        log_file (Optional[Path]): Path to the log file. If None, logs are only displayed on stderr.
    """
    logger.remove()
    logger.add(
        "net_framework_installer.log",
        rotation="10 MB",
        retention="7 days",
        compression="zip",
        enqueue=True,
        encoding="utf-8",
        format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level}</level> | {message}",
        level="DEBUG",
    )
    logger.add(
        sys.stderr,
        level="INFO",
        format="<level>{message}</level>",
    )
    if log_file:
        logger.add(
            log_file,
            rotation="10 MB",
            retention="7 days",
            compression="zip",
            enqueue=True,
            encoding="utf-8",
            format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level}</level> | {message}",
            level="DEBUG",
        )
    logger.debug("Logging is configured.")


def verify_file_checksum(file_path: Path, original_checksum: str, hash_algo: str = 'sha256') -> bool:
    """
    Verify the file checksum.

    Args:
        file_path (Path): The path to the file whose checksum is to be verified.
        original_checksum (str): The expected checksum to verify against.
        hash_algo (str): The hashing algorithm to use (default is SHA-256).

    Returns:
        bool: True if the checksum matches, False otherwise.
    """
    try:
        logger.debug(f"Verifying checksum for file: {file_path}")
        _hash = hashlib.new(hash_algo)
        with file_path.open("rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                _hash.update(chunk)
        calculated_checksum = _hash.hexdigest()
        logger.debug(f"Calculated checksum: {calculated_checksum}")
        return calculated_checksum.lower() == original_checksum.lower()
    except Exception as e:
        logger.error(f"Failed to verify checksum for {file_path}: {e}")
        return False


def check_dotnet_installed(version: str) -> bool:
    """
    Checks if a specific version of the .NET Framework is installed by querying the Windows Registry.

    Args:
        version (str): A string representing the .NET Framework version to check (e.g., 'v4\\Client').

    Returns:
        bool: True if the specified version is installed, False otherwise.
    """
    try:
        logger.debug(
            f"Checking if .NET Framework version {version} is installed.")
        result = subprocess.run(
            ["reg", "query",
                f"HKLM\\SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\{version}"],
            capture_output=True, text=True, check=True
        )
        is_installed = version in result.stdout
        logger.info(
            f".NET Framework {version} installation status: {is_installed}")
        return is_installed
    except subprocess.CalledProcessError:
        logger.info(f".NET Framework {version} is not installed.")
        return False
    except Exception as e:
        logger.exception(
            f"Error checking .NET Framework version {version}: {e}")
        return False


def list_installed_dotnets() -> None:
    """
    Lists all installed .NET Framework versions by querying the Windows Registry under the NDP key.

    Prints each found version directly to the standard output.
    """
    try:
        logger.debug("Listing all installed .NET Framework versions.")
        result = subprocess.run(
            ["reg", "query", "HKLM\\SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\"],
            capture_output=True, text=True, check=True
        )
        installed_versions = [line.strip()
                              for line in result.stdout.splitlines() if "v" in line]
        if installed_versions:
            print("Installed .NET Framework versions:")
            for version in installed_versions:
                print(version)
            logger.info("Listed all installed .NET Framework versions.")
        else:
            print("No .NET Framework versions found.")
            logger.info("No .NET Framework versions found.")
    except subprocess.CalledProcessError:
        print("Failed to query the registry for installed .NET Framework versions.")
        logger.error(
            "Failed to query the registry for installed .NET Framework versions.")
    except Exception as e:
        print("An unexpected error occurred while listing .NET Framework versions.")
        logger.exception(f"Error listing .NET Framework versions: {e}")


async def download_file_part(session: aiohttp.ClientSession, url: str, start: int, end: int) -> bytes:
    """
    Asynchronously download a part of a file specified by byte range.

    Args:
        session (aiohttp.ClientSession): The HTTP session to use for the request.
        url (str): The URL from which to download the file.
        start (int): The starting byte of the file part.
        end (int): The ending byte of the file part.

    Returns:
        bytes: The downloaded content of the file part.
    """
    headers = {'Range': f'bytes={start}-{end}'}
    async with session.get(url, headers=headers) as response:
        response.raise_for_status()
        content = await response.read()
        logger.debug(f"Downloaded bytes {start}-{end} from {url}")
        return content


async def download_file(url: str, filename: Path, num_threads: int = 4, expected_checksum: Optional[str] = None) -> None:
    """
    Download a file using multiple threads asynchronously and optionally verify its checksum.

    Args:
        url (str): The URL from which to download the file.
        filename (Path): The filename where the downloaded file will be saved.
        num_threads (int): The number of threads to use for downloading the file.
        expected_checksum (Optional[str]): The expected checksum of the downloaded file for verification purposes.

    Raises:
        ValueError: If checksum verification fails.
    """
    try:
        logger.info(
            f"Starting download from {url} using {num_threads} threads.")
        async with aiohttp.ClientSession() as session:
            async with session.head(url) as head:
                if head.status != 200:
                    logger.error(
                        f"Failed to retrieve headers from {url}. Status code: {head.status}")
                    raise ValueError(
                        f"Failed to retrieve headers from {url}. Status code: {head.status}")
                total_size = int(head.headers.get('Content-Length', 0))
                if total_size == 0:
                    logger.error(
                        "Cannot determine the size of the file to download.")
                    raise ValueError(
                        "Cannot determine the size of the file to download.")

            part_size = total_size // num_threads
            tasks = []
            for i in range(num_threads):
                start_byte = i * part_size
                end_byte = start_byte + part_size - 1 if i < num_threads - 1 else total_size - 1
                tasks.append(download_file_part(
                    session, url, start_byte, end_byte))

            logger.debug("Creating download tasks.")
            parts = await asyncio.gather(*tasks)

            logger.debug(f"Writing downloaded parts to {filename}.")
            async with aiofiles.open(filename, 'wb') as f:
                for part in parts:
                    await f.write(part)

            logger.info(f"Downloaded {filename} successfully.")

            if expected_checksum:
                if verify_file_checksum(filename, expected_checksum):
                    logger.info("File checksum verified successfully.")
                else:
                    logger.error("File checksum verification failed.")
                    await aiofiles.os.remove(filename)
                    raise ValueError("Checksum verification failed")
    except Exception as e:
        logger.exception(f"Failed to download file {url}: {e}")
        raise


def install_software(installer_path: Path) -> None:
    """
    Executes a software installer from a specified path.

    Args:
        installer_path (Path): The path to the executable installer file.
    """
    try:
        if platform != "win32":
            logger.error("This script only supports Windows.")
            print("This script only supports Windows.")
            return

        logger.info(f"Starting installation using installer: {installer_path}")
        subprocess.run(["start", "/wait", str(installer_path)],
                       shell=True, check=True)
        logger.success(f"Installer {installer_path} executed successfully.")
        print(f"Installer {installer_path} started.")
    except subprocess.CalledProcessError as e:
        logger.error(f"Installer {installer_path} failed with error: {e}")
        print(f"Installer {installer_path} failed to start.")
    except Exception as e:
        logger.exception(f"Unexpected error during installation: {e}")
        print(
            f"An unexpected error occurred while installing {installer_path}.")


def uninstall_dotnet(version: str) -> None:
    """
    Uninstall a specific version of the .NET Framework.

    Args:
        version (str): A string representing the .NET Framework version to uninstall (e.g., 'v4\\Client').
    """
    try:
        logger.info(f"Uninstalling .NET Framework version: {version}")
        result = subprocess.run(
            ["reg", "delete",
                f"HKLM\\SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\{version}", "/f"],
            capture_output=True, text=True, check=True
        )
        if result.returncode == 0:
            logger.success(
                f".NET Framework {version} uninstalled successfully.")
            print(f".NET Framework {version} uninstalled successfully.")
        else:
            logger.error(f"Failed to uninstall .NET Framework {version}.")
            print(f"Failed to uninstall .NET Framework {version}.")
    except subprocess.CalledProcessError:
        logger.error(f"Failed to uninstall .NET Framework {version}.")
        print(f"Failed to uninstall .NET Framework {version}.")
    except Exception as e:
        logger.exception(
            f"Error during uninstallation of .NET Framework {version}: {e}")
        print(
            f"An unexpected error occurred while uninstalling .NET Framework {version}.")


def parse_arguments() -> argparse.Namespace:
    """
    Parse command-line arguments.

    Returns:
        argparse.Namespace: Parsed arguments.
    """
    parser = argparse.ArgumentParser(
        description="Check and install .NET Framework versions."
    )
    parser.add_argument("--check", metavar="VERSION",
                        help="Check if a specific .NET Framework version is installed.")
    parser.add_argument("--list", action="store_true",
                        help="List all installed .NET Framework versions.")
    parser.add_argument("--download", metavar="URL",
                        help="URL to download the .NET Framework installer from.")
    parser.add_argument("--install", metavar="FILE",
                        help="Path to the .NET Framework installer to run.")
    parser.add_argument("--threads", type=int, default=4,
                        help="Number of threads to use for downloading.")
    parser.add_argument("--checksum", metavar="SHA256",
                        help="Expected SHA256 checksum of the downloaded file.")
    parser.add_argument("--uninstall", metavar="VERSION",
                        help="Uninstall a specific .NET Framework version.")
    return parser.parse_args()


def main():
    """
    Main function to parse command-line arguments and invoke script functionality.
    """
    setup_logging()
    args = parse_arguments()

    if args.list:
        list_installed_dotnets()

    if args.check:
        logger.debug(
            f"Checking installation status for .NET Framework version: {args.check}")
        if check_dotnet_installed(args.check):
            print(f".NET Framework {args.check} is already installed.")
            logger.info(f".NET Framework {args.check} is already installed.")
        else:
            print(f".NET Framework {args.check} is not installed.")
            logger.info(f".NET Framework {args.check} is not installed.")
            if args.download and args.install:
                installer_path = Path(args.install)
                loop = asyncio.get_event_loop()
                try:
                    loop.run_until_complete(download_file(
                        args.download, installer_path, num_threads=args.threads, expected_checksum=args.checksum))
                    install_software(installer_path)
                except Exception as e:
                    logger.error(f"Installation process failed: {e}")
                    print(f"Failed to install .NET Framework {args.check}.")
    elif args.download or args.install:
        logger.warning(
            "Download and install options should be used in conjunction with the --check argument.")
        print("Download and install options should be used in conjunction with the --check argument.")

    if args.uninstall:
        uninstall_dotnet(args.uninstall)

    if not any([args.list, args.check, args.uninstall]):
        parser = argparse.ArgumentParser(
            description="No action specified. Use --help to see available options."
        )
        parser.print_help()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        logger.warning("Operation interrupted by user.")
        print("Operation interrupted by user.")
        exit(0)
    except Exception as e:
        logger.exception(f"An unexpected error occurred: {e}")
        print(f"An unexpected error occurred: {e}")
        exit(1)
