"""
Auto Updater Script

This script is designed to automatically check for, download, verify, and install updates for a given application.
It supports multi-threaded downloads, file verification, backup of current files, and logging of update history.

Author: Your Name
Date: 2024-06-20
"""

import os
import json
import zipfile
import shutil
import requests
import threading
import argparse
import logging
from tqdm import tqdm
from typing import Any, Dict, Optional
import hashlib
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

logging.basicConfig(level=logging.INFO)

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
        self.url = config['url']
        self.install_dir = Path(config['install_dir'])
        self.num_threads = config.get('num_threads', 4)
        self.custom_params = config.get('custom_params', {})
        self.temp_dir = self.install_dir / "temp"
        self.temp_dir.mkdir(parents=True, exist_ok=True)
        self.latest_version = None
        self.download_url = None

    def check_for_updates(self) -> bool:
        """
        Checks for updates from the given URL.

        Returns
        -------
        bool
            True if updates are available, False otherwise
        """
        try:
            response = requests.get(self.url)
            response.raise_for_status()
            data = response.json()
            self.latest_version = data['version']
            self.download_url = data['download_url']
            logging.info(f"Found update: version {self.latest_version}")
            return True
        except requests.RequestException as e:
            logging.error(f"Failed to check for updates: {e}")
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
        return self.latest_version > current_version

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
        try:
            response = requests.get(url, stream=True)
            response.raise_for_status()
            total_size = int(response.headers.get('content-length', 0))
            chunk_size = 1024
            with open(dest, 'wb') as file, tqdm(
                    desc=dest.name,
                    total=total_size,
                    unit='B',
                    unit_scale=True,
                    unit_divisor=1024,
            ) as bar:
                for data in response.iter_content(chunk_size=chunk_size):
                    file.write(data)
                    bar.update(len(data))
        except requests.RequestException as e:
            logging.error(f"Failed to download file: {e}")

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
        sha256 = hashlib.sha256()
        try:
            with open(file_path, 'rb') as f:
                for chunk in iter(lambda: f.read(4096), b""):
                    sha256.update(chunk)
            file_hash = sha256.hexdigest()
            return file_hash == expected_hash
        except Exception as e:
            logging.error(f"Failed to verify file: {e}")
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
        try:
            with zipfile.ZipFile(zip_path, 'r') as zip_ref:
                zip_ref.extractall(extract_to)
        except zipfile.BadZipFile as e:
            logging.error(f"Failed to extract zip file: {e}")

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
        try:
            for item in src.iterdir():
                s = src / item
                d = dest / item
                if s.is_dir():
                    shutil.copytree(s, d, dirs_exist_ok=True)
                else:
                    shutil.copy2(s, d)
        except Exception as e:
            logging.error(f"Failed to move files: {e}")

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
        try:
            backup_dir.mkdir(parents=True, exist_ok=True)
            for item in src.iterdir():
                s = src / item
                d = backup_dir / item
                if s.is_dir():
                    shutil.copytree(s, d, dirs_exist_ok=True)
                else:
                    shutil.copy2(s, d)
            logging.info("Backup completed successfully.")
        except Exception as e:
            logging.error(f"Failed to backup files: {e}")

    def cleanup(self):
        """
        Cleans up temporary files and directories.
        """
        try:
            shutil.rmtree(self.temp_dir, ignore_errors=True)
        except Exception as e:
            logging.error(f"Failed to clean up: {e}")

    def custom_post_download(self):
        """
        Executes custom post-download actions.
        """
        logging.info("Running custom post-download actions")
        if 'post_download' in self.custom_params:
            self.custom_params['post_download']()

    def custom_post_install(self):
        """
        Executes custom post-install actions.
        """
        logging.info("Running custom post-install actions")
        if 'post_install' in self.custom_params:
            self.custom_params['post_install']()

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
        try:
            with open(self.install_dir / "update_log.txt", 'a') as log_file:
                log_file.write(f"Updated from version {current_version} to {new_version}\n")
        except Exception as e:
            logging.error(f"Failed to log update: {e}")

    def update(self, current_version: str):
        """
        Orchestrates the update process.

        Parameters
        ----------
        current_version : str
            The current version of the application
        """
        if self.check_for_updates() and self.compare_versions(current_version):
            logging.info("Update available. Downloading...")
            zip_path = self.temp_dir / "update.zip"
            self.download_file(self.download_url, zip_path)
            if self.verify_file(zip_path, self.custom_params.get('expected_hash', '')):
                self.custom_post_download()
                logging.info("Download complete. Extracting...")
                self.extract_zip(zip_path, self.temp_dir)
                logging.info("Extraction complete. Backing up current files...")
                backup_dir = self.install_dir / "backup"
                self.backup_files(self.install_dir, backup_dir)
                logging.info("Backup complete. Installing update...")
                self.move_files(self.temp_dir, self.install_dir)
                self.custom_post_install()
                logging.info("Installation complete. Cleaning up...")
                self.cleanup()
                self.log_update(current_version, self.latest_version)
                logging.info("Update installed successfully.")
            else:
                logging.error("File verification failed. Update aborted.")
        else:
            logging.info("No updates available or version is not newer.")

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
    parser.add_argument("--config", type=str, required=True, help="Path to the configuration file")

    args = parser.parse_args()

    with open(args.config, 'r') as f:
        config = json.load(f)

    updater_thread = threading.Thread(target=run_updater, args=(config,))
    updater_thread.start()
    updater_thread.join()

if __name__ == "__main__":
    main()
