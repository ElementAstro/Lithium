# CoreRunner: C++ Core Dump and Analysis Tool

## Overview

**CoreRunner** is a Python-based utility designed to facilitate the setup, compilation, and execution of C++ programs, with a focus on core dump analysis. It integrates with `g++` for compilation, `gdb` for debugging, and provides robust logging and exception handling. The tool is particularly useful for developers who need to automate the process of compiling, running, and analyzing core dumps of C++ programs.

## Features

- **Compilation**: Compiles C++ source files using `g++` with customizable flags and standards.
- **Execution**: Runs the compiled C++ program and captures core dumps if the program crashes.
- **Core Dump Analysis**: Automatically analyzes core dump files using `gdb` with user-defined commands.
- **Logging**: Uses `loguru` for detailed logging, with options to log to both the console and a file.
- **Environment Validation**: Checks for the presence of necessary tools (`g++`, `gdb`) and validates the environment before execution.
- **Customizable**: Allows users to specify compilation flags, GDB commands, core dump directory, and more via command-line arguments.

## Installation

To use **CoreRunner**, ensure you have Python 3.x installed on your system. Additionally, you need to have `g++` and `gdb` installed.

### Dependencies

- Python 3.x
- `g++`
- `gdb`
- `loguru` (Python package)
- `rich` (Python package)

You can install the required Python packages using pip:

```bash
pip install loguru rich
```

## Usage

### Command-Line Arguments

**CoreRunner** accepts the following command-line arguments:

- `source`: The C++ source file to compile and run.
- `-o`, `--output`: The output executable name (default: `a.out`).
- `-d`, `--core-dir`: Directory to search for core dumps (default: `/tmp`).
- `-p`, `--core-pattern`: Core pattern for dump files (default: `/tmp/core.%e.%p`).
- `-u`, `--ulimit`: Set core dump size to unlimited (default: `False`).
- `-f`, `--flags`: Additional flags for `g++` compilation (e.g., `-O2 -Wall`).
- `-s`, `--std`: C++ standard to use (e.g., `c++11`, `c++14`, `c++17`, `c++20`).
- `-g`, `--gdb-commands`: GDB commands for core dump analysis (default: `["-ex", "bt", "-ex", "quit"]`).
- `-a`, `--auto-analyze`: Automatically analyze core dump if program crashes (default: `False`).
- `-l`, `--log-file`: Log file to write logs to.

### Example Command

```bash
python core_runner.py my_program.cpp -o my_program -d /var/cores -p /var/cores/core.%e.%p -u -f -O2 -Wall -s c++17 -g -ex "bt full" -ex "quit" -a -l /var/logs/core_runner.log
```

### Workflow

1. **Setup Logging**: Configures the `loguru` logger to output logs to the console and optionally to a specified log file.
2. **Environment Validation**: Checks if `g++` and `gdb` are available in the system's PATH.
3. **Set Ulimit**: Sets the core dump size to unlimited if specified.
4. **Set Core Pattern**: Sets the core pattern for core dump files if run with root privileges.
5. **Compile C++ Program**: Compiles the provided C++ source file using `g++` with the specified flags and standard.
6. **Run C++ Program**: Executes the compiled program. If the program crashes, it captures the core dump.
7. **Analyze Core Dump**: Automatically analyzes the core dump using `gdb` with the specified commands if the program crashes and `auto-analyze` is enabled.

## Code Structure

### CoreRunner Class

The `CoreRunner` class is the core of the utility, handling the setup, compilation, and execution of the C++ program. It includes the following methods:

- `__init__`: Initializes the class with command-line arguments and sets up logging and environment validation.
- `setup_logging`: Configures the `loguru` logger.
- `validate_environment`: Validates the presence of `g++` and `gdb`.
- `set_ulimit`: Sets the core dump size using `ulimit`.
- `set_core_pattern`: Sets the core pattern for core dump files.
- `compile_cpp_program`: Compiles the C++ program using `g++`.
- `run_cpp_program`: Runs the compiled C++ program and handles crashes.
- `find_latest_core_file`: Finds the latest core dump file in the specified directory.
- `analyze_core_dump`: Analyzes the core dump file using `gdb`.
- `run`: Executes the full workflow, including setup, compilation, and program execution.

### Helper Functions

- `configure_logging`: Configures the `loguru` logger based on the provided log file.
- `parse_arguments`: Parses command-line arguments using `argparse`.
- `main`: The main function that initializes the `CoreRunner` and runs the workflow.

## Logging

**CoreRunner** uses the `loguru` library for logging, providing detailed logs with timestamps, log levels, and messages. Logs can be directed to both the console and a specified log file, with options for rotation, retention, and compression.

## Exception Handling

The tool includes robust exception handling to manage errors during compilation, execution, and core dump analysis. It logs detailed error messages and exits gracefully in case of failures.

## Conclusion

**CoreRunner** is a powerful tool for automating the process of compiling, running, and analyzing C++ programs, especially in scenarios where core dump analysis is crucial. Its integration with `g++` and `gdb`, along with detailed logging and exception handling, makes it a valuable asset for developers working with C++ code.
