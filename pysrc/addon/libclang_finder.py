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
"""

import platform
import os
import glob
import subprocess
import argparse
from clang.cindex import Config
from loguru import logger

CACHE_FILE = "libclang_path_cache.txt"


def cache_libclang_path(path: str):
    """Caches the found libclang path to a file."""
    with open(CACHE_FILE, 'w') as f:
        f.write(path)
    logger.info(f"Cached libclang path: {path}")


def load_cached_libclang_path() -> str | None:
    """Loads the cached libclang path from a file."""
    if os.path.exists(CACHE_FILE):
        with open(CACHE_FILE, 'r') as f:
            path = f.read().strip()
        if os.path.isfile(path):
            logger.info(f"Loaded cached libclang path: {path}")
            return path
    return None


def find_libclang_linux():
    """Searches for the libclang library on Linux systems."""
    possible_paths = [
        '/usr/lib/llvm-*/*/lib/libclang.so',
        '/usr/local/lib/llvm-*/*/lib/libclang.so',
        '/usr/lib/x86_64-linux-gnu/libclang.so',
        '/usr/local/lib/x86_64-linux-gnu/libclang.so',
        '/usr/lib/llvm-*/lib/libclang.so',
        '/usr/local/lib/llvm-*/lib/libclang.so'
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
    """Searches for the libclang library on macOS systems."""
    possible_paths = [
        '/usr/local/opt/llvm/lib/libclang.dylib',
        '/usr/local/lib/libclang.dylib',
        '/Library/Developer/CommandLineTools/usr/lib/libclang.dylib',
        '/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/libclang.dylib'
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
    """Searches for the libclang library on Windows systems."""
    possible_paths = [
        'C:\\Program Files\\LLVM\\bin\\libclang.dll',
        'C:\\Program Files (x86)\\LLVM\\bin\\libclang.dll',
        'C:\\LLVM\\bin\\libclang.dll'
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


def get_libclang_path(custom_path: str | None = None):
    """Determines the appropriate libclang library path based on the operating system."""
    if custom_path and os.path.isfile(custom_path):
        logger.info(f"Using custom libclang path: {custom_path}")
        return custom_path

    cached_path = load_cached_libclang_path()
    if cached_path:
        return cached_path

    system = platform.system()
    logger.info(f"Detected operating system: {system}")
    if system == 'Linux':
        path = find_libclang_linux()
    elif system == 'Darwin':  # macOS
        path = find_libclang_macos()
    elif system == 'Windows':
        path = find_libclang_windows()
    else:
        logger.error("Unsupported operating system")
        raise RuntimeError("Unsupported operating system")

    cache_libclang_path(path)
    return path


def main():
    """Main execution block for setting up libclang path and configuring clang.cindex."""
    parser = argparse.ArgumentParser(description="libclang Path Finder")
    parser.add_argument('--path', type=str, help="Custom path to libclang")
    args = parser.parse_args()

    try:
        libclang_path = get_libclang_path(args.path)
        Config.set_library_file(libclang_path)
        logger.info(f"Successfully set libclang path to: {libclang_path}")
    except Exception as e:
        logger.exception("Failed to set libclang path")


if __name__ == "__main__":
    main()
