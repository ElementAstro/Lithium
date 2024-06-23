#!/usr/bin/env python3
"""
@file         net_framework_installer.py
@brief        A command-line utility to check for installed .NET Framework versions on Windows,
              download installers, and run them with multithreading and file verification capabilities.

@details      This script uses the Windows Registry to determine installed .NET Framework versions
              and provides functionality to download and execute installer files for missing versions
              using multithreaded downloads and checksum verification for file integrity.

              Usage:
                python net_framework_installer.py --list
                python net_framework_installer.py --check v4.0.30319
                python net_framework_installer.py --check v4.0.30319 --download [URL] --install [FILE_PATH] --threads 4 --checksum [SHA256]

@requires     - Python 3.x
              - Windows operating system
              - `requests` Python library

@author       Your Name
@date         Date of creation or last modification
"""

import argparse
import hashlib
import subprocess
import threading
import os
from sys import platform
import requests

def verify_file_checksum(file_path, original_checksum, hash_algo='sha256'):
    """
    Verify the file checksum.

    @param file_path        The path to the file whose checksum is to be verified.
    @param original_checksum The expected checksum to verify against.
    @param hash_algo        The hashing algorithm to use (default is SHA-256).

    @return True if the checksum matches, False otherwise.
    """
    _hash = hashlib.new(hash_algo)
    with open(file_path, 'rb') as f:
        for chunk in iter(lambda: f.read(4096), b""):
            _hash.update(chunk)
    return _hash.hexdigest() == original_checksum

def check_dotnet_installed(version):
    """
    Checks if a specific version of the .NET Framework is installed by querying the Windows Registry.

    @param version: A string representing the .NET Framework version to check (e.g., 'v4\\Client').
    @return: True if the specified version is installed, False otherwise.
    """
    try:
        result = subprocess.run(
            ["reg", "query", f"HKLM\\SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\{version}"],
            capture_output=True, text=True, check=True
        )
        return result.returncode == 0 and version in result.stdout
    except subprocess.CalledProcessError:
        return False

def list_installed_dotnets():
    """
    Lists all installed .NET Framework versions by querying the Windows Registry under the NDP key.

    Prints each found version directly to the standard output.
    """
    try:
        result = subprocess.run(
            ["reg", "query", "HKLM\\SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\"],
            capture_output=True, text=True, check=True
        )
        if result.returncode == 0:
            print("Installed .NET Framework versions:")
            for line in result.stdout.splitlines():
                if "v" in line:
                    print(line.strip())
    except subprocess.CalledProcessError:
        print("Failed to query the registry for installed .NET Framework versions.")

def download_file_part(url, start, end, filename, idx, results):
    """
    Download a part of a file specified by byte range.

    @param url              The URL from which to download the file.
    @param start            The starting byte of the file part.
    @param end              The ending byte of the file part.
    @param filename         The filename where the downloaded data will be temporarily stored.
    @param idx              The index of the thread (used for storing results in the correct order).
    @param results          A shared list to store results from each thread.
    """
    headers = {'Range': f'bytes={start}-{end}'}
    response = requests.get(url, headers=headers, stream=True, timeout=10)
    response.raise_for_status()
    results[idx] = response.content

def download_file(url, filename, num_threads=4, expected_checksum=None):
    """
    Download a file using multiple threads and optionally verify its checksum.

    @param url              The URL from which to download the file.
    @param filename         The filename where the downloaded file will be saved.
    @param num_threads      The number of threads to use for downloading the file.
    @param expected_checksum Optional; the expected checksum of the downloaded file for verification purposes.

    @return None
    """
    response = requests.head(url, timeout=10)
    total_size = int(response.headers.get('content-length', 0))
    part_size = total_size // num_threads
    results = [None] * num_threads

    threads = []
    for i in range(num_threads):
        start = i * part_size
        end = start + part_size - 1 if i < num_threads - 1 else total_size - 1
        args = (url, start, end, filename, i, results)
        thread = threading.Thread(target=download_file_part, args=args)
        threads.append(thread)
        thread.start()

    for thread in threads:
        thread.join()

    with open(filename, 'wb') as f:
        for content in results:
            f.write(content)

    print(f"Downloaded {filename}")
    if expected_checksum:
        if verify_file_checksum(filename, expected_checksum):
            print("File checksum verified successfully.")
        else:
            print("File checksum verification failed.")
            os.remove(filename)
            raise ValueError("Checksum verification failed")

def install_software(installer_path : str):
    """
    Executes a software installer from a specified path.

    @param installer_path: The path to the executable installer file.
    """
    if platform == "win32":  # Ensure this is run on Windows
        subprocess.run(["start", installer_path], shell=True, check=True)
        print(f"Installer {installer_path} started.")
    else:
        print("This script only supports Windows.")

def main():
    """
    Main function to parse command-line arguments and invoke script functionality.
    """
    parser = argparse.ArgumentParser(description="Check and install .NET Framework versions.")
    parser.add_argument("--check", metavar="VERSION", help="Check if a specific .NET Framework version is installed.")
    parser.add_argument("--list", action="store_true", help="List all installed .NET Framework versions.")
    parser.add_argument("--download", metavar="URL", help="URL to download the .NET Framework installer from.")
    parser.add_argument("--install", metavar="FILE", help="Path to the .NET Framework installer to run.")

    args = parser.parse_args()

    if args.list:
        list_installed_dotnets()

    if args.check:
        if check_dotnet_installed(args.check):
            print(f".NET Framework {args.check} is already installed.")
        else:
            print(f".NET Framework {args.check} is not installed.")
            if args.download and args.install:
                download_file(args.download, args.install)
                install_software(args.install)

if __name__ == "__main__":
    main()