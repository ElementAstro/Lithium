"""
Auto Updater Script

This script is designed to automatically check for, download, verify, and install updates for a given application.
It supports multi-threaded downloads, file verification, backup of current files, and logging of update history.

Author: Max Qian
Date: 2024-06-20
"""

import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import datetime
import hashlib
import json
import os
import shutil
import sys
from pathlib import Path
from typing import Any, Dict, Optional, List
import zipfile

import requests
from loguru import logger
from tqdm import tqdm


class AutoUpdater:
    """
    A class used to represent the AutoUpdater.

    Attributes
    ----------
    url : str
        The URL to check for updates
    install_dir : pathlib.Path
        The directory where the application is installed
    num_threads : int
        Number of threads to use for downloading
    custom_params : dict
        Custom parameters for post-download and post-install actions
    temp_dir : pathlib.Path
        Temporary directory for download and extraction
    latest_version : str
        The latest version available for download
    download_url : str
        The URL to download the latest version

    Methods
    -------
    check_for_updates()
        Checks for updates from the given URL
    compare_versions(current_version: str) -> bool
        Compares the current version with the latest version
    download_file(url: str, dest: pathlib.Path)
        Downloads a file from the given URL to the specified destination
    verify_file(file_path: pathlib.Path, expected_hash: str) -> bool
        Verifies the downloaded file's hash
    extract_zip(zip_path: pathlib.Path, extract_to: pathlib.Path)
        Extracts a zip file to the specified directory
    move_files(src: pathlib.Path, dest: pathlib.Path)
        Moves files from source to destination
    backup_files(src: pathlib.Path, backup_dir: pathlib.Path)
        Backs up current files to the specified backup directory
    cleanup()
        Cleans up temporary files and directories
    custom_post_download()
        Executes custom post-download actions
    custom_post_install()
        Executes custom post-install actions
    log_update(current_version: str, new_version: str)
        Logs the update history
    update(current_version: str)
        Orchestrates the update process
    """

    def __init__(self, config: Dict[str, Any]):
        """
        Initializes the AutoUpdater with the given configuration.

        Parameters
        ----------
        config : dict
            Configuration parameters for the updater
        """
        self.url: str = config['url']
        self.install_dir: Path = Path(config['install_dir'])
        self.num_threads: int = config.get('num_threads', 4)
        self.custom_params: Dict[str, Any] = config.get('custom_params', {})
        self.temp_dir: Path = self.install_dir / "temp"
        self.temp_dir.mkdir(parents=True, exist_ok=True)
        self.latest_version: Optional[str] = None
        self.download_url: Optional[str] = None
        self.expected_hash: Optional[str] = None

        # Configure Loguru
        logger.remove()
        logger.add(
            self.install_dir / "updater.log",
            rotation="10 MB",
            retention="30 days",
            compression="zip",
            enqueue=True,
            encoding="utf-8",
            format=(
                "<green>{time:YYYY-MM-DD HH:mm:ss}</green> | "
                "<level>{level}</level> | {message}"
            ),
            level="DEBUG",
        )
        logger.add(
            sys.stderr,
            level="INFO",
            format="<level>{message}</level>",
        )
        logger.debug("AutoUpdater initialized and logging configured.")

    def check_for_updates(self) -> bool:
        """
        Checks for updates from the given URL.

        Returns
        -------
        bool
            True if updates are available, False otherwise
        """
        logger.debug(f"Checking for updates at URL: {self.url}")
        try:
            response = requests.get(self.url, timeout=10)
            response.raise_for_status()
            data = response.json()
            self.latest_version = data.get('version')
            self.download_url = data.get('download_url')
            self.expected_hash = data.get('expected_hash')

            if not self.latest_version or not self.download_url:
                logger.error("Update information is incomplete.")
                return False

            logger.info(f"Found update: version {self.latest_version}")
            return True
        except requests.RequestException as e:
            logger.error(f"Failed to check for updates: {e}")
            return False
        except json.JSONDecodeError as e:
            logger.error(f"Invalid JSON response: {e}")
            return False

    def compare_versions(self, current_version: str) -> bool:
        """
        Compares the current version with the latest version.

        Parameters
        ----------
        current_version : str
            The current version of the application

        Returns
        -------
        bool
            True if the latest version is newer than the current version, False otherwise
        """
        logger.debug(
            f"Comparing versions. Current: {current_version}, Latest: {self.latest_version}"
        )
        from packaging import version
        try:
            return version.parse(self.latest_version) > version.parse(current_version)
        except Exception as e:
            logger.error(f"Version comparison failed: {e}")
            return False

    def download_file(self, url: str, dest: Path):
        """
        Downloads a file from the given URL to the specified destination.

        Parameters
        ----------
        url : str
            The URL to download the file from
        dest : pathlib.Path
            The destination path to save the downloaded file
        """
        logger.debug(f"Starting download from {url} to {dest}")
        try:
            response = requests.get(url, stream=True, timeout=30)
            response.raise_for_status()
            total_size = int(response.headers.get('content-length', 0))
            chunk_size = 8192  # 8 KB

            with open(dest, 'wb') as file, tqdm(
                desc=dest.name,
                total=total_size,
                unit='iB',
                unit_scale=True,
                unit_divisor=1024,
            ) as bar:
                for chunk in response.iter_content(chunk_size=chunk_size):
                    if chunk:
                        file.write(chunk)
                        bar.update(len(chunk))
            logger.info(f"Downloaded file to {dest}")
        except requests.RequestException as e:
            logger.error(f"Failed to download file from {url}: {e}")
            raise

    def verify_file(self, file_path: Path, expected_hash: str) -> bool:
        """
        Verifies the downloaded file's hash.

        Parameters
        ----------
        file_path : pathlib.Path
            The path to the downloaded file
        expected_hash : str
            The expected SHA-256 hash of the file

        Returns
        -------
        bool
            True if the file's hash matches the expected hash, False otherwise
        """
        logger.debug(
            f"Verifying file {file_path} with expected hash {expected_hash}"
        )
        sha256 = hashlib.sha256()
        try:
            with open(file_path, 'rb') as f:
                for chunk in iter(lambda: f.read(4096), b""):
                    sha256.update(chunk)
            file_hash = sha256.hexdigest()
            if file_hash.lower() == expected_hash.lower():
                logger.info("File verification succeeded.")
                return True
            else:
                logger.error(
                    f"File verification failed. Expected {expected_hash}, got {file_hash}"
                )
                return False
        except Exception as e:
            logger.error(f"Failed to verify file {file_path}: {e}")
            return False

    def extract_zip(self, zip_path: Path, extract_to: Path):
        """
        Extracts a zip file to the specified directory.

        Parameters
        ----------
        zip_path : pathlib.Path
            The path to the zip file
        extract_to : pathlib.Path
            The directory to extract the zip file to
        """
        logger.debug(f"Extracting {zip_path} to {extract_to}")
        try:
            with zipfile.ZipFile(zip_path, 'r') as zip_ref:
                zip_ref.extractall(extract_to)
            logger.info(f"Extraction complete: {extract_to}")
        except zipfile.BadZipFile as e:
            logger.error(f"Failed to extract zip file {zip_path}: {e}")
            raise
        except Exception as e:
            logger.error(f"An error occurred while extracting {zip_path}: {e}")
            raise

    def move_files(self, src: Path, dest: Path):
        """
        Moves files from source to destination.

        Parameters
        ----------
        src : pathlib.Path
            The source directory
        dest : pathlib.Path
            The destination directory
        """
        logger.debug(f"Moving files from {src} to {dest}")
        try:
            for item in src.iterdir():
                s = item
                d = dest / item.name
                if s.is_dir():
                    shutil.copytree(s, d, dirs_exist_ok=True)
                else:
                    shutil.copy2(s, d)
            logger.info(f"Files moved from {src} to {dest}")
        except Exception as e:
            logger.error(f"Failed to move files from {src} to {dest}: {e}")
            raise

    def backup_files(self, src: Path, backup_dir: Path):
        """
        Backs up current files to the specified backup directory.

        Parameters
        ----------
        src : pathlib.Path
            The source directory
        backup_dir : pathlib.Path
            The backup directory
        """
        logger.debug(f"Backing up files from {src} to {backup_dir}")
        try:
            backup_dir.mkdir(parents=True, exist_ok=True)
            for item in src.iterdir():
                s = item
                d = backup_dir / item.name
                if s.is_dir():
                    shutil.copytree(s, d, dirs_exist_ok=True)
                else:
                    shutil.copy2(s, d)
            logger.info("Backup completed successfully.")
        except Exception as e:
            logger.error(
                f"Failed to backup files from {src} to {backup_dir}: {e}"
            )
            raise

    def cleanup(self):
        """
        Cleans up temporary files and directories.
        """
        logger.debug(f"Cleaning up temporary directory {self.temp_dir}")
        try:
            shutil.rmtree(self.temp_dir, ignore_errors=True)
            logger.info("Cleanup completed successfully.")
        except Exception as e:
            logger.error(f"Failed to clean up temporary files: {e}")

    def custom_post_download(self):
        """
        Executes custom post-download actions.
        """
        logger.info("Executing custom post-download actions.")
        try:
            if (
                'post_download' in self.custom_params and
                callable(self.custom_params['post_download'])
            ):
                self.custom_params['post_download']()
                logger.debug(
                    "Custom post-download action executed successfully."
                )
        except Exception as e:
            logger.error(f"Custom post-download action failed: {e}")
            raise

    def custom_post_install(self):
        """
        Executes custom post-install actions.
        """
        logger.info("Executing custom post-install actions.")
        try:
            if (
                'post_install' in self.custom_params and
                callable(self.custom_params['post_install'])
            ):
                self.custom_params['post_install']()
                logger.debug(
                    "Custom post-install action executed successfully."
                )
        except Exception as e:
            logger.error(f"Custom post-install action failed: {e}")
            raise

    def log_update(self, current_version: str, new_version: str):
        """
        Logs the update history.

        Parameters
        ----------
        current_version : str
            The current version of the application
        new_version : str
            The new version of the application
        """
        logger.debug(
            f"Logging update from version {current_version} to {new_version}"
        )
        try:
            log_entry = (
                f"Updated from version {current_version} "
                f"to {new_version} on {datetime.datetime.now()}\n"
            )
            with open(
                self.install_dir / "update_log.txt",
                'a',
                encoding='utf-8'
            ) as log_file:
                log_file.write(log_entry)
            logger.info("Update history logged successfully.")
        except Exception as e:
            logger.error(f"Failed to log update history: {e}")
            raise

    def download_multiple_files(self, urls: List[str], dest_dir: Path):
        """
        Downloads multiple files concurrently.

        Parameters
        ----------
        urls : List[str]
            List of URLs to download
        dest_dir : pathlib.Path
            Directory to save the downloaded files
        """
        logger.debug(
            f"Starting multi-threaded download of {len(urls)} files to {dest_dir}"
        )
        dest_dir.mkdir(parents=True, exist_ok=True)
        try:
            with ThreadPoolExecutor(max_workers=self.num_threads) as executor:
                future_to_url = {
                    executor.submit(
                        self.download_file, url, dest_dir / Path(url).name
                    ): url
                    for url in urls
                }
                for future in as_completed(future_to_url):
                    url = future_to_url[future]
                    try:
                        future.result()
                    except Exception as e:
                        logger.error(f"Error downloading {url}: {e}")
            logger.info("All files downloaded successfully.")
        except Exception as e:
            logger.error(f"Failed to download multiple files: {e}")
            raise

    def update(self, current_version: str):
        """
        Orchestrates the update process.

        Parameters
        ----------
        current_version : str
            The current version of the application
        """
        logger.debug(
            f"Starting update process. Current version: {current_version}"
        )
        try:
            if self.check_for_updates() and self.compare_versions(current_version):
                logger.info(
                    f"Update available: {self.latest_version}. Proceeding with update."
                )

                zip_path = self.temp_dir / "update.zip"
                self.download_file(self.download_url, zip_path)

                if (
                    self.expected_hash and
                    self.verify_file(zip_path, self.expected_hash)
                ):
                    self.custom_post_download()

                    logger.info(
                        "Verifying the integrity of the downloaded update."
                    )

                    self.extract_zip(zip_path, self.temp_dir)

                    logger.info("Backing up current installation.")
                    backup_dir = self.install_dir / "backup"
                    self.backup_files(self.install_dir, backup_dir)

                    logger.info("Installing the update.")
                    self.move_files(self.temp_dir, self.install_dir)

                    self.custom_post_install()

                    logger.info("Cleaning up temporary files.")
                    self.cleanup()

                    self.log_update(current_version, self.latest_version)

                    logger.success("Update installed successfully.")
                    print(
                        f"Updated to version {self.latest_version} successfully."
                    )
                else:
                    logger.error("File verification failed. Aborting update.")
                    print("Error: Downloaded file failed verification.")
            else:
                logger.info(
                    "No updates available or current version is up-to-date."
                )
                print("No updates available.")
        except KeyboardInterrupt:
            logger.warning("Update process interrupted by user.")
            print("\nUpdate process interrupted by user.")
            self.cleanup()
            sys.exit(1)
        except Exception as e:
            logger.exception(
                f"An unexpected error occurred during the update: {e}"
            )
            print(f"An unexpected error occurred: {e}")
            self.cleanup()
            sys.exit(1)


def run_updater(config: Dict[str, Any]):
    """
    Runs the updater with the provided configuration.

    Parameters
    ----------
    config : dict
        Configuration parameters for the updater
    """
    updater = AutoUpdater(config)
    current_version = config.get('current_version', '0.0.0')
    updater.update(current_version)


def main():
    """
    The main entry point for the script. Parses the configuration file and starts the updater.
    """
    parser = argparse.ArgumentParser(description="Auto updater script")
    parser.add_argument(
        "--config",
        type=str,
        required=True,
        help="Path to the configuration file"
    )

    args = parser.parse_args()

    config_path = Path(args.config)
    if not config_path.is_file():
        logger.error(f"Configuration file not found: {config_path}")
        print(f"Error: Configuration file not found: {config_path}")
        sys.exit(1)

    try:
        with open(config_path, 'r', encoding='utf-8') as f:
            config = json.load(f)
        logger.debug(f"Configuration loaded from {config_path}")
    except json.JSONDecodeError as e:
        logger.error(f"Invalid JSON in configuration file: {e}")
        print(f"Error: Invalid JSON in configuration file: {e}")
        sys.exit(1)
    except Exception as e:
        logger.error(f"Failed to read configuration file: {e}")
        print(f"Error: Failed to read configuration file: {e}")
        sys.exit(1)

    run_updater(config)


if __name__ == "__main__":
    main()
