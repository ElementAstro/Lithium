# CMake Build Configuration Generator Documentation

## Overview

This script automates the generation of **CMake build configuration files** for C++ projects. It supports multi-directory project structures, the creation of custom `FindXXX.cmake` files for third-party libraries, and the use of JSON configuration files for specifying project settings. It also offers customizable compiler flags, linker flags, and dependencies management.

---

## Key Features

- **Multi-directory Project Support**: Supports creating `CMakeLists.txt` for each subdirectory in multi-directory projects.
- **Custom `FindXXX.cmake` Generation**: Automatically generates custom `FindXXX.cmake` files for locating third-party libraries.
- **JSON-based Configuration**: Uses a JSON configuration file to specify project settings, streamlining the generation process.
- **Customizable Build Settings**: Configure compiler flags, linker flags, dependencies, and subdirectories.
- **Enhanced Logging**: Comprehensive logging using the `Loguru` library for easier debugging and tracking of operations.
- **Robust Exception Handling**: Handles errors gracefully, with clear messages for the user.
- **Directory Creation**: Automatically creates the necessary directories for saving the generated CMake files.
- **Cross-platform Support**: Handles platform-specific settings for Linux, Windows, and macOS.

---

## Requirements

- **Python 3.x**
- Required Python packages:
  - `loguru`: For logging.
  - `rich`: For beautified console output.
  - `json`: For JSON parsing (standard Python library).
  - `platform`: For detecting the operating system (standard Python library).

To install the required packages, run:

```bash
pip install loguru rich
```

---

## Usage

### Command-Line Arguments

The script supports several commands to generate CMake files:

#### `generate`

Generate the `CMakeLists.txt` and `FindXXX.cmake` files from a JSON configuration.

```bash
python cmake_generator.py generate --json path/to/config.json --output-dir output/
```

- **`--json`**: Path to the JSON configuration file.
- **`--output-dir`**: Directory to save the generated files (default is the current directory).

#### `find`

Generate a `FindXXX.cmake` file for a specified library.

```bash
python cmake_generator.py find --library LibraryName --output-dir cmake/
```

- **`--library`**: Name of the third-party library (e.g., `Boost`, `OpenCV`).
- **`--output-dir`**: Directory to save the `FindXXX.cmake` file (default is `cmake/`).

### Example

1. **Generate `CMakeLists.txt` from JSON Configuration:**

```bash
python cmake_generator.py generate --json project_config.json --output-dir build/
```

2. **Generate `FindBoost.cmake`:**

```bash
python cmake_generator.py find --library Boost --output-dir cmake/
```

---

## JSON Configuration Format

The script uses a JSON file to specify the project's configuration. Here's an example of a typical configuration:

```json
{
  "project_name": "MyProject",
  "version": "1.0",
  "cpp_standard": "11",
  "executable": true,
  "static_library": false,
  "shared_library": false,
  "enable_testing": true,
  "include_dirs": ["include", "libs/boost/include"],
  "sources": "src/*.cpp",
  "compiler_flags": ["-O3", "-Wall"],
  "linker_flags": ["-lpthread"],
  "dependencies": ["Boost", "OpenCV"],
  "subdirs": ["lib", "tests"],
  "install_path": "bin",
  "test_framework": "GoogleTest"
}
```

### Key Configuration Fields

- **`project_name`**: The name of the project.
- **`version`**: The version of the project (default is "1.0").
- **`cpp_standard`**: The C++ standard to use (default is `C++11`).
- **`executable`**: Whether to generate an executable target.
- **`static_library`**: Whether to generate a static library.
- **`shared_library`**: Whether to generate a shared library.
- **`enable_testing`**: Whether to enable testing with `enable_testing()`.
- **`include_dirs`**: List of directories to include.
- **`sources`**: Glob pattern for source files (default is `src/*.cpp`).
- **`compiler_flags`**: Custom compiler flags.
- **`linker_flags`**: Custom linker flags.
- **`dependencies`**: List of third-party dependencies.
- **`subdirs`**: Subdirectories to add (useful for multi-directory projects).
- **`install_path`**: The path for installation (default is `bin`).
- **`test_framework`**: The test framework to be used (optional, e.g., `GoogleTest`).

---

## Platform Detection

The script detects the current operating system and configures the appropriate `CMake` system name:

- **Windows**: `set(CMAKE_SYSTEM_NAME Windows)`
- **Darwin (macOS)**: `set(CMAKE_SYSTEM_NAME Darwin)`
- **Linux**: `set(CMAKE_SYSTEM_NAME Linux)`

If an unsupported OS is detected, a warning is logged, and no OS-specific settings are applied.

---

## Logging

The script uses **Loguru** for logging, and logs are written both to a file (`cmake_generator.log`) and to the console. Logs include:

- **Debug logs**: For tracking the generation of the CMake files and other details.
- **Info logs**: For general information about the process.
- **Error logs**: For capturing issues with file generation or invalid configurations.

Logs are rotated every 10 MB, with logs older than 7 days being compressed and retained.

---

## Error Handling

- **Invalid JSON Format**: If the JSON configuration file is invalid, an error will be raised with details.
- **Missing Fields**: If required fields are missing from the configuration, a `KeyError` will be raised.
- **Unsupported OS**: If the platform is unsupported, no OS-specific settings will be applied.

---

## Conclusion

The **CMake Build Configuration Generator** is a powerful tool for automating the generation of `CMakeLists.txt` and `FindXXX.cmake` files for C++ projects. With support for multi-directory projects, third-party dependencies, and customizable build settings, this script simplifies the configuration of complex C++ projects. Enhanced logging and robust error handling ensure smooth operation and quick debugging.

This script is ideal for C++ developers who need a flexible, automated solution for managing build configurations across different platforms.
