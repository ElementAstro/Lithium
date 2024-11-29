# Compiler Helper Script Documentation

## Overview

The **Compiler Helper Script** is a Python utility that assists in detecting available C++ compilers (such as GCC, Clang, MSVC, Intel C++ Compiler), selecting the desired C++ version, and compiling or linking source files with specified options. It also supports loading additional compile/link options from JSON files, enhanced logging with Loguru, and beautiful terminal output with Rich.

This script aims to simplify the process of compiling C++ code and allows for flexible compiler and version selection.

---

## Features

- **Detect Available C++ Compilers**: Detects and lists installed compilers (GCC, Clang, MSVC, Intel C++ Compiler).
- **Supports Multiple C++ Versions**: Allows compiling with C++ versions from C++98 to C++23.
- **Compile and Link**: Facilitates both compiling individual source files and linking them into an executable.
- **Load Compile/Link Flags from JSON**: Optionally load additional flags from a JSON configuration file.
- **Logging**: Utilizes the Loguru library for enhanced logging.
- **Beautiful Terminal Output**: Uses the Rich library for clear, styled terminal output, including tables and progress indicators.
- **Error Handling**: Robust exception handling with detailed error messages and tracebacks.

---

## Requirements

- Python 3.x
- Required Python packages:
  - `loguru`: For logging.
  - `rich`: For styled console output.

Install the necessary packages using:

```bash
pip install loguru rich
```

Additionally, the script assumes the availability of common C++ compilers like GCC, Clang, MSVC, or Intel C++ Compiler installed on your system.

---

## Usage

To run the script, use the following command:

```bash
python compiler_helper.py source1.cpp source2.cpp -o output --compiler GCC --cpp_version c++20 --link --flags -O3
```

### Command-Line Arguments

- **`source_files`**: The C++ source files to compile or link (one or more).
- **`-o` / `--output`**: Specifies the output file (either an object file or executable).
- **`--link`**: If provided, the script will link the object files into an executable.
- **`--compiler`**: Choose the compiler to use. Valid options: `GCC`, `Clang`, `MSVC`, `Intel C++ Compiler`.
- **`--cpp_version`**: The C++ version to use for compilation (e.g., `c++17`, `c++20`).
- **`--flags`**: Additional flags for compilation or linking.
- **`--compile-flags`**: Additional compilation flags.
- **`--link-flags`**: Additional linking flags.
- **`--json-options`**: Path to a JSON file containing additional compile/link flags.
- **`--show-info`**: Display system and script information, and exit.

---

## Structure of the Script

### Key Classes

1. **`CppVersion` (Enum)**  
   Represents supported C++ versions. The versions include `CPP98`, `CPP03`, `CPP11`, `CPP14`, `CPP17`, `CPP20`, `CPP23`, and `CPP2A`.

2. **`CompilerType` (Enum)**  
   Represents different compiler types: `GCC`, `CLANG`, `MSVC`, `INTEL`.

3. **`Compiler` (Class)**  
   Represents a C++ compiler and its associated flags. Each compiler has a name, command, supported C++ flags, and compile/link flags.

   Key methods:

   - **`compile()`**: Compiles the source files into object files or executables.
   - **`link()`**: Links object files into an executable.

4. **`setup_logging()`**  
   Configures Loguru for logging, both to a file and the console.

5. **`detect_compilers()`**  
   Detects available compilers on the system (GCC, Clang, MSVC, Intel).

6. **`select_compiler()`**  
   Allows the user to select a compiler from the detected compilers.

7. **`select_cpp_version()`**  
   Prompts the user to select a C++ version for the chosen compiler.

8. **`load_options_from_json()`**  
   Loads additional compile/link flags from a specified JSON file.

9. **`display_system_info()`**  
   Displays system information (OS, Python version) and script version.

---

## How It Works

1. **Compiler Detection**  
   The script first detects installed C++ compilers using `shutil.which()` for GCC, Clang, MSVC, and Intel C++ compilers. If no compilers are found, the script exits with an error message.

2. **Compiler Selection**  
   If multiple compilers are available, the user is prompted to select one. If only one compiler is available, it is automatically selected.

3. **C++ Version Selection**  
   The user can either specify a C++ version using the `--cpp_version` option or be prompted to select one from the available versions for the selected compiler.

4. **Compilation and Linking**  
   Once the compiler and C++ version are selected, the script compiles the source files into object files or an executable. If the `--link` option is specified, the script will link object files into an executable.

5. **Flags**  
   Additional flags for compilation and linking can be specified either via the `--flags`, `--compile-flags`, `--link-flags` options or through a JSON file.

---

## Example JSON Configuration File

Here’s an example of how the JSON file for additional flags (`json-options`) should be structured:

```json
{
  "compile_flags": ["-O2", "-std=c++17"],
  "link_flags": ["-pthread"]
}
```

To use this configuration, pass the `--json-options` argument with the path to the JSON file:

```bash
python compiler_helper.py source1.cpp source2.cpp -o output --json-options config.json
```

---

## Logging

The script uses Loguru for logging. Logs are saved in the `compiler_helper.log` file and also printed to the console. The logging includes detailed information about the compilation and linking processes, as well as errors and warnings.

### Log File Configuration

- **Rotation**: Logs are rotated when they reach 5 MB.
- **Retention**: Logs are kept for 7 days.
- **Compression**: Older logs are compressed in ZIP format.

---

## Error Handling

The script includes robust error handling:

- If no suitable compiler is detected, the script exits with an error.
- If compilation or linking fails, the script exits with the error details.
- If invalid arguments are provided (e.g., unsupported C++ version), the script will inform the user and exit.

---

## Conclusion

This **Compiler Helper Script** simplifies the process of compiling and linking C++ code. It automatically detects compilers, supports multiple C++ versions, and allows customization via command-line arguments and JSON configuration files. With logging and beautiful terminal output, it provides a user-friendly experience for developers.
