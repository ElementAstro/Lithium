# libclang Path Finder Documentation

## Overview

The **libclang Path Finder** is a Python tool designed to help locate the `libclang` shared library on various operating systems (Linux, macOS, Windows). It allows users to configure the `libclang` path for use with Python libraries like `clang.cindex`. This tool supports clearing cache files, adding custom search patterns, listing all found `libclang` libraries, and more.

The script includes functionality to:

- Search for `libclang` on different operating systems.
- Select and cache the appropriate `libclang` path.
- List available `libclang` libraries.
- Handle logging and error reporting.
- Support both custom paths and automatic searching.

---

## Features

- **Cross-platform support**: Detects `libclang` paths for Linux, macOS, and Windows.
- **Custom search patterns**: Allows adding custom glob patterns for more flexible search.
- **Cache management**: Caches the found `libclang` path for faster subsequent accesses.
- **Verbose logging**: Option to enable detailed logs for debugging.
- **Interactive selection**: In case of multiple results, users can choose which `libclang` path to use.
- **Error handling**: Provides meaningful error messages when `libclang` cannot be found.

---

## Requirements

- **Python 3.x**
- Required Python packages:
  - `clang`: For using `libclang`.
  - `loguru`: For logging.
  - `rich`: For beautified console output.

Install the required packages using:

```bash
pip install clang loguru rich
```

---

## Usage

### Command-Line Arguments

- **`--path`**: Specifies a custom path to the `libclang` library.
- **`--clear-cache`**: Clears the cached `libclang` path (if it exists).
- **`--search-patterns`**: Adds additional glob patterns for searching `libclang`.
- **`--cache-file`**: Path to the cache file for storing the `libclang` path (default is `libclang_path_cache.txt`).
- **`--log-file`**: Path to the log file (default is `libclang_finder.log`).
- **`--list`**: Lists all found `libclang` libraries.
- **`--verbose`**: Enables verbose logging, providing more detailed output.

#### Examples

1. **Specify a custom libclang path**:

   ```bash
   python libclang_finder.py --path /custom/path/libclang.so
   ```

2. **Clear the cached libclang path**:

   ```bash
   python libclang_finder.py --clear-cache
   ```

3. **Add additional search patterns**:

   ```bash
   python libclang_finder.py --search-patterns "/opt/llvm/lib/libclang.so*"
   ```

4. **List all found libclang libraries**:

   ```bash
   python libclang_finder.py --list
   ```

5. **Enable verbose logging**:

   ```bash
   python libclang_finder.py --verbose
   ```

---

## Functions

### `clear_cache_file()`

Clears the cache file that stores the `libclang` path.

### `cache_libclang_path(path: Path)`

Caches the found `libclang` path to a file for later use.

### `load_cached_libclang_path() -> Optional[Path]`

Loads the cached `libclang` path, if it exists and is valid.

### `find_libclang_linux() -> List[Path]`

Searches for `libclang` on Linux by using predefined glob patterns.

### `find_libclang_macos() -> List[Path]`

Searches for `libclang` on macOS using predefined glob patterns.

### `find_libclang_windows() -> List[Path]`

Searches for `libclang` on Windows using predefined glob patterns.

### `search_paths(patterns: List[str]) -> List[Path]`

Searches for `libclang` using the provided glob patterns and any custom patterns specified by the user.

### `select_libclang_path(paths: List[Path]) -> Optional[Path]`

If multiple `libclang` paths are found, this function allows the user to select one interactively.

### `get_libclang_path() -> Path`

Determines the appropriate `libclang` path, either from a custom path, cache, or by searching the system.

### `configure_clang()`

Configures the `libclang` path for use by `clang.cindex`.

### `list_libclang_versions() -> List[Path]`

Lists all available `libclang` libraries found on the system.

---

## Logging

The tool uses **Loguru** for logging, which allows the user to track the execution flow and debug any issues.

- **Log level**: The log level is `INFO` by default, but it can be set to `DEBUG` with the `--verbose` flag.
- **Log file**: Logs are stored in `libclang_finder.log`.
- **Cache file**: The cached `libclang` path is stored in `libclang_path_cache.txt`.

---

## Error Handling

- **Unsupported OS**: If the script detects an unsupported operating system, it will raise a `RuntimeError` and display an error message.
- **Missing libclang**: If no `libclang` path is found, the tool will notify the user and raise a `RuntimeError`.

---

## Example Workflow

1. **Search for `libclang`**:
   The script first checks for a custom path to `libclang`. If no custom path is provided, it searches the system based on the OS and any additional search patterns specified.

2. **Select the correct `libclang` path**:
   If multiple paths are found, the user is prompted to choose one. Once selected, it is cached for future use.

3. **Configure `clang.cindex`**:
   After finding the `libclang` path, the tool configures `clang.cindex` with the selected path, making it ready for use.

---

## Conclusion

The **libclang Path Finder** script simplifies the process of locating and configuring the `libclang` library on various platforms. It offers powerful search functionality, caching, and a user-friendly interface for selecting the appropriate library path. Whether you're working with `clang.cindex` or other tools that require `libclang`, this tool streamlines the setup process.
