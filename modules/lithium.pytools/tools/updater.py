# python
"""
Auto Updater Script

This script is designed to automatically check for, download, verify, and install updates for a given application.
It supports multi-threaded downloads, file verification, backup of current files, and logging of update history.

Author: Max Qian
Date: 2024-06-20
"""

import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass, field
import datetime
import hashlib
import json
import shutil
import sys
from pathlib import Path
from typing import Any, Dict, Optional, List
import zipfile

import requests
from loguru import logger
from rich.console import Console
from rich.progress import Progress, BarColumn, TextColumn, TimeRemainingColumn
from packaging import version


class AutoUpdaterError(Exception):
    """Base exception class for AutoUpdater."""


class UpdateCheckError(AutoUpdaterError):
    """Exception for update check failures."""


class DownloadError(AutoUpdaterError):
    """Exception for download failures."""


class VerificationError(AutoUpdaterError):
    """Exception for file verification failures."""


class ExtractionError(AutoUpdaterError):
    """Exception for extraction failures."""


class BackupError(AutoUpdaterError):
    """Exception for backup failures."""


class InstallationError(AutoUpdaterError):
    """Exception for installation failures."""


class CleanupError(AutoUpdaterError):
    """Exception for cleanup failures."""


@dataclass
class AutoUpdaterConfig:
    """Configuration for the AutoUpdater."""
    url: str
    install_dir: Path
    num_threads: int = 4
    custom_params: Dict[str, Any] = field(default_factory=dict)
    log_file: Path = field(default=Path("updater.log"))
    log_level: str = "DEBUG"


class AutoUpdater:
    """
    A class used to represent the AutoUpdater.

    Attributes
    ----------
    config : AutoUpdaterConfig
        Configuration parameters for the updater
    latest_version : Optional[str]
        The latest version available for download
    download_url : Optional[str]
        The URL to download the latest version
    expected_hash : Optional[str]
        The expected SHA-256 hash of the downloaded file
    temp_dir : Path
        Temporary directory for download and extraction
    console : Console
        Rich console for enhanced terminal output
    """

    def __init__(self, config: AutoUpdaterConfig):
        """
        Initializes the AutoUpdater with the given configuration.

        Parameters
        ----------
        config : AutoUpdaterConfig
            Configuration parameters for the updater
        """
        self.config = config
        self.latest_version: Optional[str] = None
        self.download_url: Optional[str] = None
        self.expected_hash: Optional[str] = None
        self.temp_dir: Path = self.config.install_dir / "temp"
        self.temp_dir.mkdir(parents=True, exist_ok=True)
        self.console = Console()

        self.setup_logging()
        self._validate_executable()

    def setup_logging(self):
        """Configure Loguru for logging."""
        logger.remove()
        logger.add(
            self.config.log_file,
            rotation="10 MB",
            retention="30 days",
            compression="zip",
            enqueue=True,
            encoding="utf-8",
            format=(
                "<green>{time:YYYY-MM-DD HH:mm:ss}</green> | "
                "<level>{level}</level> | {message}"
            ),
            level=self.config.log_level,
        )
        logger.add(
            sys.stderr,
            level="INFO",
            format="<level>{message}</level>",
        )
        logger.debug("AutoUpdater initialized and logging configured.")

    def _validate_executable(self):
        """Validate that required executables exist."""
        # Example: Validate 'unzip' is available if needed
        if not shutil.which("unzip"):
            logger.error("Required executable 'unzip' not found.")
            raise AutoUpdaterError("Required executable 'unzip' not found.")
        logger.info("All required executables are available.")

    def check_for_updates(self) -> bool:
        """
        Checks for updates from the given URL.

        Returns
        -------
        bool
            True if updates are available, False otherwise
        """
        logger.debug(f"Checking for updates at URL: {self.config.url}")
        try:
            response = requests.get(self.config.url, timeout=10)
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

        Raises
        ------
        DownloadError
            If the download fails
        """
        logger.debug(f"Starting download from {url} to {dest}")
        try:
            with requests.get(url, stream=True, timeout=30) as response:
                response.raise_for_status()
                total_size = int(response.headers.get('content-length', 0))
                chunk_size = 1024  # 1 KB

                with open(dest, 'wb') as file, Progress(
                    "[progress.description]{task.description}",
                    BarColumn(),
                    TextColumn(
                        "[progress.percentage]{task.percentage:>3.0f}%"),
                    TimeRemainingColumn(),
                ) as progress:
                    task = progress.add_task(
                        f"Downloading {dest.name}", total=total_size)
                    for chunk in response.iter_content(chunk_size=chunk_size):
                        if chunk:
                            file.write(chunk)
                            progress.update(task, advance=len(chunk))
            logger.info(f"Downloaded file to {dest}")
        except requests.RequestException as e:
            logger.error(f"Failed to download file from {url}: {e}")
            raise DownloadError(f"Failed to download file from {url}") from e

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

        Raises
        ------
        ExtractionError
            If extraction fails
        """
        logger.debug(f"Extracting {zip_path} to {extract_to}")
        try:
            with zipfile.ZipFile(zip_path, 'r') as zip_ref, Progress(
                "[progress.description]{task.description}",
                BarColumn(),
                TextColumn("[progress.percentage]{task.percentage:>3.0f}%"),
                TimeRemainingColumn(),
            ) as progress:
                files = zip_ref.namelist()
                task = progress.add_task("Extracting files", total=len(files))
                for file in files:
                    zip_ref.extract(file, extract_to)
                    progress.advance(task)
            logger.info(f"Extraction complete: {extract_to}")
        except zipfile.BadZipFile as e:
            logger.error(f"Failed to extract zip file {zip_path}: {e}")
            raise ExtractionError(
                f"Failed to extract zip file {zip_path}") from e
        except Exception as e:
            logger.error(f"An error occurred while extracting {zip_path}: {e}")
            raise ExtractionError(
                f"An error occurred while extracting {zip_path}") from e

    def move_files(self, src: Path, dest: Path):
        """
        Moves files from source to destination.

        Parameters
        ----------
        src : pathlib.Path
            The source directory
        dest : pathlib.Path
            The destination directory

        Raises
        ------
        InstallationError
            If moving files fails
        """
        logger.debug(f"Moving files from {src} to {dest}")
        try:
            for item in src.iterdir():
                s = item
                d = dest / item.name
                if s.is_dir():
                    shutil.move(str(s), str(d))
                else:
                    shutil.move(str(s), str(d))
            logger.info(f"Files moved from {src} to {dest}")
        except Exception as e:
            logger.error(f"Failed to move files from {src} to {dest}: {e}")
            raise InstallationError(
                f"Failed to move files from {src} to {dest}") from e

    def backup_files(self, src: Path, backup_dir: Path):
        """
        Backs up current files to the specified backup directory.

        Parameters
        ----------
        src : pathlib.Path
            The source directory
        backup_dir : pathlib.Path
            The backup directory

        Raises
        ------
        BackupError
            If backup fails
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
            raise BackupError(
                f"Failed to backup files from {src} to {backup_dir}") from e

    def cleanup(self):
        """
        Cleans up temporary files and directories.

        Raises
        ------
        CleanupError
            If cleanup fails
        """
        logger.debug(f"Cleaning up temporary directory {self.temp_dir}")
        try:
            shutil.rmtree(self.temp_dir, ignore_errors=True)
            logger.info("Cleanup completed successfully.")
        except Exception as e:
            logger.error(f"Failed to clean up temporary files: {e}")
            raise CleanupError("Failed to clean up temporary files") from e

    def custom_post_download(self):
        """
        Executes custom post-download actions.

        Raises
        ------
        AutoUpdaterError
            If custom post-download actions fail
        """
        logger.info("Executing custom post-download actions.")
        try:
            if (
                'post_download' in self.config.custom_params and
                callable(self.config.custom_params['post_download'])
            ):
                self.config.custom_params['post_download']()
                logger.debug(
                    "Custom post-download action executed successfully."
                )
        except Exception as e:
            logger.error(f"Custom post-download action failed: {e}")
            raise AutoUpdaterError(
                "Custom post-download action failed.") from e

    def custom_post_install(self):
        """
        Executes custom post-install actions.

        Raises
        ------
        AutoUpdaterError
            If custom post-install actions fail
        """
        logger.info("Executing custom post-install actions.")
        try:
            if (
                'post_install' in self.config.custom_params and
                callable(self.config.custom_params['post_install'])
            ):
                self.config.custom_params['post_install']()
                logger.debug(
                    "Custom post-install action executed successfully."
                )
        except Exception as e:
            logger.error(f"Custom post-install action failed: {e}")
            raise AutoUpdaterError("Custom post-install action failed.") from e

    def log_update(self, current_version: str, new_version: str):
        """
        Logs the update history.

        Parameters
        ----------
        current_version : str
            The current version of the application
        new_version : str
            The new version of the application

        Raises
        ------
        AutoUpdaterError
            If logging fails
        """
        logger.debug(
            f"Logging update from version {current_version} to {new_version}"
        )
        try:
            log_entry = (
                f"Updated from version {current_version} "
                f"to {new_version} on {datetime.datetime.now().isoformat()}\n"
            )
            with open(
                self.config.install_dir / "update_log.txt",
                'a',
                encoding='utf-8'
            ) as log_file:
                log_file.write(log_entry)
            logger.info("Update history logged successfully.")
        except Exception as e:
            logger.error(f"Failed to log update history: {e}")
            raise AutoUpdaterError("Failed to log update history.") from e

    def download_multiple_files(self, urls: List[str], dest_dir: Path):
        """
        Downloads multiple files concurrently.

        Parameters
        ----------
        urls : List[str]
            List of URLs to download
        dest_dir : pathlib.Path
            Directory to save the downloaded files

        Raises
        ------
        DownloadError
            If any of the downloads fail
        """
        logger.debug(
            f"Starting multi-threaded download of {len(urls)} files to {dest_dir}"
        )
        dest_dir.mkdir(parents=True, exist_ok=True)
        try:
            with ThreadPoolExecutor(max_workers=self.config.num_threads) as executor, Progress(
                "[progress.description]{task.description}",
                BarColumn(),
                TextColumn("[progress.percentage]{task.percentage:>3.0f}%"),
                TimeRemainingColumn(),
            ) as progress:
                tasks = {
                    executor.submit(
                        self.download_file, url, dest_dir / Path(url).name
                    ): url
                    for url in urls
                }
                for future in as_completed(tasks):
                    url = tasks[future]
                    try:
                        future.result()
                    except Exception as e:
                        logger.error(f"Error downloading {url}: {e}")
                        raise DownloadError(f"Error downloading {url}") from e
            logger.info("All files downloaded successfully.")
        except Exception as e:
            logger.error(f"Failed to download multiple files: {e}")
            raise DownloadError("Failed to download multiple files.") from e

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
                    backup_dir = self.config.install_dir / "backup"
                    self.backup_files(self.config.install_dir, backup_dir)

                    logger.info("Installing the update.")
                    self.move_files(self.temp_dir, self.config.install_dir)

                    self.custom_post_install()

                    logger.info("Cleaning up temporary files.")
                    self.cleanup()

                    self.log_update(current_version, self.latest_version)

                    logger.success("Update installed successfully.")
                    self.console.print(
                        f"[bold green]Updated to version {self.latest_version} successfully.[/bold green]"
                    )
                else:
                    logger.error("File verification failed. Aborting update.")
                    self.console.print(
                        "[bold red]Error: Downloaded file failed verification.[/bold red]"
                    )
            else:
                logger.info(
                    "No updates available or current version is up-to-date."
                )
                self.console.print(
                    "[bold blue]No updates available.[/bold blue]")
        except KeyboardInterrupt:
            logger.warning("Update process interrupted by user.")
            self.console.print(
                "\n[bold yellow]Update process interrupted by user.[/bold yellow]")
            self.cleanup()
            sys.exit(1)
        except AutoUpdaterError as e:
            logger.exception(f"An error occurred during the update: {e}")
            self.console.print(f"[bold red]An error occurred: {e}[/bold red]")
            self.cleanup()
            sys.exit(1)
        except Exception as e:
            logger.exception(
                f"An unexpected error occurred during the update: {e}")
            self.console.print(
                f"[bold red]An unexpected error occurred: {e}[/bold red]")
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
    updater_config = AutoUpdaterConfig(
        url=config['url'],
        install_dir=Path(config['install_dir']),
        num_threads=config.get('num_threads', 4),
        custom_params=config.get('custom_params', {}),
        log_file=Path(config.get('log_file', 'updater.log')),
        log_level=config.get('log_level', 'DEBUG')
    )
    updater = AutoUpdater(updater_config)
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
        help="Path to the configuration file (JSON format)"
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
