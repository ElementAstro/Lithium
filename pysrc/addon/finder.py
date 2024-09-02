"""
libclang Path Finder

This script automatically locates the path to the libclang library depending on the operating system.
It supports Linux, macOS, and Windows platforms. The script uses the `loguru` library for detailed
logging and provides comprehensive error handling.

Dependencies:
- clang.cindex
- loguru

Usage:
- The script automatically determines the path to `libclang` and sets it for the `clang.cindex.Config`.

Author: [Your Name]
Date: [Date]
"""

import platform
import os
import glob
import subprocess
from clang.cindex import Config
from loguru import logger


def find_libclang_linux():
    """
    Searches for the libclang library on Linux systems.

    This function looks through several common installation paths for libclang on Linux.
    If the library is not found in the predefined paths, a RuntimeError is raised.

    Returns:
        str: The path to the libclang library.

    Raises:
        RuntimeError: If the libclang library is not found.
    """
    possible_paths = [
        '/usr/lib/llvm-*/*/lib/libclang.so',
        '/usr/local/lib/llvm-*/*/lib/libclang.so',
        '/usr/lib/x86_64-linux-gnu/libclang.so',
        '/usr/local/lib/x86_64-linux-gnu/libclang.so'
    ]
    logger.info("Searching for libclang on Linux...")
    for pattern in possible_paths:
        for path in glob.glob(pattern):
            if os.path.isfile(path):
                logger.info(f"Found libclang at {path}")
                return path
    logger.error("libclang not found on Linux")
    raise RuntimeError("libclang not found on Linux")


def find_libclang_macos():
    """
    Searches for the libclang library on macOS systems.

    This function checks common installation paths for libclang on macOS. If the library is not
    found in the predefined paths, it attempts to locate it using the `find` command. A RuntimeError
    is raised if the library cannot be found.

    Returns:
        str: The path to the libclang library.

    Raises:
        RuntimeError: If the libclang library is not found.
    """
    possible_paths = [
        '/usr/local/opt/llvm/lib/libclang.dylib',
        '/usr/local/lib/libclang.dylib'
    ]
    logger.info("Searching for libclang on macOS...")
    for path in possible_paths:
        if os.path.isfile(path):
            logger.info(f"Found libclang at {path}")
            return path
    # Fallback: try using `find` command
    try:
        result = subprocess.run(['find', '/', '-name', 'libclang.dylib'],
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        paths = result.stdout.strip().split('\n')
        if paths:
            logger.info(f"Found libclang at {paths[0]}")
            return paths[0]
    except Exception as e:
        logger.error(f"Error while searching for libclang: {e}")
    logger.error("libclang not found on macOS")
    raise RuntimeError("libclang not found on macOS")


def find_libclang_windows():
    """
    Searches for the libclang library on Windows systems.

    This function checks common installation paths for libclang on Windows. If the library is not found
    in the predefined paths, it attempts to locate it using the `where` command. A RuntimeError is raised
    if the library cannot be found.

    Returns:
        str: The path to the libclang library.

    Raises:
        RuntimeError: If the libclang library is not found.
    """
    possible_paths = [
        'C:\\Program Files\\LLVM\\bin\\libclang.dll',
        'C:\\Program Files (x86)\\LLVM\\bin\\libclang.dll'
    ]
    logger.info("Searching for libclang on Windows...")
    for path in possible_paths:
        if os.path.isfile(path):
            logger.info(f"Found libclang at {path}")
            return path
    # Fallback: try using `where` command
    try:
        result = subprocess.run(
            ['where', 'libclang.dll'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        paths = result.stdout.strip().split('\n')
        if paths:
            logger.info(f"Found libclang at {paths[0]}")
            return paths[0]
    except Exception as e:
        logger.error(f"Error while searching for libclang: {e}")
    logger.error("libclang not found on Windows")
    raise RuntimeError("libclang not found on Windows")


def get_libclang_path():
    """
    Determines the appropriate libclang library path based on the operating system.

    This function detects the current operating system and calls the corresponding function
    to find the libclang library path. If the operating system is not supported, a RuntimeError
    is raised.

    Returns:
        str: The path to the libclang library.

    Raises:
        RuntimeError: If the operating system is unsupported.
    """
    system = platform.system()
    logger.info(f"Detected operating system: {system}")
    if system == 'Linux':
        return find_libclang_linux()
    elif system == 'Darwin':  # macOS
        return find_libclang_macos()
    elif system == 'Windows':
        return find_libclang_windows()
    else:
        logger.error("Unsupported operating system")
        raise RuntimeError("Unsupported operating system")


# Main Execution Block
if __name__ == "__main__":
    """
    Main execution block for setting up libclang path and configuring clang.cindex.

    This block attempts to determine the path to the libclang library using `get_libclang_path()`
    and sets it using `clang.cindex.Config.set_library_file()`. If any errors occur, they are logged.
    """
    try:
        libclang_path = get_libclang_path()
        Config.set_library_file(libclang_path)
        logger.info(f"Successfully set libclang path to: {libclang_path}")
    except Exception as e:
        logger.exception("Failed to set libclang path")
