# upload.py
# Advanced File Upload Client with Enhanced Features
# Author: Max Qian <lightapt.com>
# Date: 2024-03-01
# Copyright (C) 2023-2024 Max Qian <lightapt.com>

import argparse
import hashlib
import json
import threading
import time
from pathlib import Path
from typing import List, Optional

import requests
from cryptography.fernet import Fernet
from loguru import logger
from queue import Queue
from rich.console import Console
from rich.progress import Progress, BarColumn, TextColumn, TimeRemainingColumn
from rich.logging import RichHandler

# Configure logger with RichHandler for better output
console = Console()
logger.remove()
logger.add(RichHandler(console=console))

# Global variables
LOG_FILE = "upload_client.log"
console = Console()


# ==== File Handling ====
def calculate_hash(file_path: Path) -> str:
    """
    Calculate the SHA-256 hash of a file.

    :param file_path: Path to the file.
    :return: Hexadecimal hash string.
    """
    sha256 = hashlib.sha256()
    try:
        with file_path.open("rb") as f:
            while chunk := f.read(8192):
                sha256.update(chunk)
        return sha256.hexdigest()
    except Exception as e:
        logger.error(f"Error calculating hash for {file_path}: {e}")
        raise


def encrypt_file(file_path: Path, key: bytes) -> Path:
    """
    Encrypt a file using AES (Fernet).

    :param file_path: Path to the file.
    :param key: Encryption key.
    :return: Path to the encrypted file.
    """
    try:
        with file_path.open("rb") as f:
            data = f.read()

        cipher = Fernet(key)
        encrypted_data = cipher.encrypt(data)

        encrypted_file_path = file_path.with_suffix(file_path.suffix + ".enc")
        with encrypted_file_path.open("wb") as f:
            f.write(encrypted_data)

        logger.debug(f"File encrypted: {encrypted_file_path}")
        return encrypted_file_path
    except Exception as e:
        logger.error(f"Error encrypting file {file_path}: {e}")
        raise


# ==== Upload Logic ====
def upload_file(file_path: Path, server_url: str, retries: int = 3, verify_server: bool = False) -> bool:
    """
    Upload a file to the server with retries and optional server verification.

    :param file_path: Path to the file.
    :param server_url: Server URL.
    :param retries: Number of retry attempts.
    :param verify_server: Whether to verify the server's response.
    :return: True if upload is successful, False otherwise.
    """
    if not file_path.exists():
        logger.error(f"File not found: {file_path}")
        raise FileNotFoundError(f"File not found: {file_path}")

    file_size = file_path.stat().st_size
    for attempt in range(1, retries + 1):
        try:
            with file_path.open("rb") as f:
                with Progress(
                    TextColumn("[progress.description]{task.description}"),
                    BarColumn(),
                    "[progress.percentage]{task.percentage:>3.0f}%",
                    "•",
                    "[green]{task.completed}/{task.total} bytes",
                    "•",
                    TimeRemainingColumn(),
                    console=console,
                ) as progress:
                    task = progress.add_task(
                        f"Uploading {file_path.name}", total=file_size)

                    def upload():
                        response = requests.post(
                            server_url,
                            files={"file": (file_path.name, f)},
                            timeout=60
                        )
                        return response

                    response = upload()
                    progress.update(task, advance=file_size)

            if response.status_code == 200:
                logger.info(f"File uploaded successfully: {file_path}")
                if verify_server:
                    # Verify the server's hash
                    server_hash = response.json().get("hash")
                    local_hash = calculate_hash(file_path)
                    if server_hash != local_hash:
                        logger.error("Server hash does not match local hash")
                        raise ValueError("Server hash mismatch")
                return True
            else:
                logger.warning(
                    f"Upload failed (Status Code: {response.status_code}): {response.text}")
        except Exception as e:
            logger.error(
                f"Upload attempt {attempt} failed for {file_path}: {e}")

        logger.info(f"Retrying upload ({attempt}/{retries})...")
        time.sleep(1)
    logger.error(
        f"Failed to upload file after {retries} attempts: {file_path}")
    return False


# ==== Multi-threaded Upload ====
def upload_worker(queue: Queue, server_url: str, key: Optional[bytes], verify_server: bool):
    """
    Worker function for uploading files in a thread.

    :param queue: Queue containing file paths to upload.
    :param server_url: Server URL.
    :param key: Encryption key, if any.
    :param verify_server: Whether to verify the server's response.
    """
    while not queue.empty():
        file_path: Path = queue.get()
        try:
            original_file_path = file_path
            # Encrypt the file if key is provided
            if key:
                file_path = encrypt_file(file_path, key)
                logger.info(f"File encrypted: {file_path}")

            upload_file(file_path, server_url, verify_server=verify_server)

            # Clean up encrypted file after upload
            if key:
                file_path.unlink()
                logger.debug(f"Encrypted file removed: {file_path}")
        except Exception as e:
            logger.error(f"Error in worker for {file_path}: {e}")
        finally:
            queue.task_done()


def upload_multiple_files(file_paths: List[Path], server_url: str, threads: int = 4, key: Optional[bytes] = None,
                          verify_server: bool = False):
    """
    Upload multiple files using multi-threading.

    :param file_paths: List of file paths to upload.
    :param server_url: Server URL.
    :param threads: Number of threads to use.
    :param key: Encryption key, if any.
    :param verify_server: Whether to verify the server's response.
    """
    queue = Queue()
    for file_path in file_paths:
        queue.put(file_path)

    thread_list = []
    for _ in range(threads):
        thread = threading.Thread(target=upload_worker, args=(
            queue, server_url, key, verify_server))
        thread.start()
        thread_list.append(thread)

    for thread in thread_list:
        thread.join()


# ==== Configuration Loading ====
def load_config(config_file: Path) -> dict:
    """
    Load configuration from a JSON file.

    :param config_file: Path to the configuration file.
    :return: Configuration dictionary.
    """
    try:
        with config_file.open("r") as f:
            config = json.load(f)
        logger.debug(f"Configuration loaded: {config}")
        return config
    except Exception as e:
        logger.error(f"Error loading configuration from {config_file}: {e}")
        raise


# ==== Main Function ====
def main():
    parser = argparse.ArgumentParser(description="Advanced File Upload Client")
    parser.add_argument("--files", nargs="+",
                        help="List of file paths to upload")
    parser.add_argument("--server", required=True, help="Target server URL")
    parser.add_argument("--encrypt", action="store_true",
                        help="Encrypt files before uploading")
    parser.add_argument("--key", help="Path to the encryption key file")
    parser.add_argument("--config", help="Path to the JSON configuration file")
    parser.add_argument(
        "--filter-type", help="Only upload files with the specified extension (e.g., .txt)")
    parser.add_argument("--verify-server", action="store_true",
                        help="Verify server response")
    parser.add_argument("--threads", type=int, default=4,
                        help="Number of concurrent upload threads")
    args = parser.parse_args()

    # Load configuration file if provided
    if args.config:
        try:
            config_path = Path(args.config)
            config = load_config(config_path)
            args.files = config.get("files", args.files)
            args.server = config.get("server", args.server)
            args.encrypt = config.get("encrypt", args.encrypt)
            args.key = config.get("key", args.key)
            args.filter_type = config.get("filter_type", args.filter_type)
            args.threads = config.get("threads", args.threads)
            args.verify_server = config.get(
                "verify_server", args.verify_server)
        except Exception as e:
            logger.error(f"Failed to load configuration: {e}")
            return

    # Validate server URL
    if not args.server:
        logger.error("Server URL is required.")
        parser.print_help()
        return

    # Collect files to upload
    if args.files:
        file_paths = [Path(f) for f in args.files]
    else:
        logger.error("No files specified for upload.")
        parser.print_help()
        return

    # Filter files by extension if specified
    if args.filter_type:
        file_paths = [f for f in file_paths if f.suffix == args.filter_type]
        if not file_paths:
            logger.warning(
                f"No files with extension {args.filter_type} found.")
            return

    # Load encryption key if encryption is enabled
    key = None
    if args.encrypt:
        if args.key:
            key_path = Path(args.key)
            if key_path.exists():
                with key_path.open("rb") as f:
                    key = f.read()
                logger.debug(f"Encryption key loaded from {key_path}")
            else:
                logger.error(f"Encryption key file not found: {key_path}")
                return
        else:
            # Generate a new key if not provided
            key = Fernet.generate_key()
            key_path = Path("encryption.key")
            with key_path.open("wb") as f:
                f.write(key)
            logger.info(
                f"No key provided. Generated and saved new encryption key to {key_path}")

    # Start uploading files
    try:
        upload_multiple_files(file_paths, args.server, threads=args.threads,
                              key=key, verify_server=args.verify_server)
        logger.info("All files uploaded successfully!")
    except Exception as e:
        logger.error(f"Client encountered an error: {e}")


if __name__ == "__main__":
    main()
