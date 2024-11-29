# Executable Binder Script Documentation

## Overview

The **Executable Binder Script** is a Python utility designed to generate PyBind11 bindings for an executable by parsing its help information. This script dynamically parses command-line options, enhances logging, and provides beautified terminal output, making it easier for developers to create Python bindings for their C++ executables.

---

## Features

- **Dynamic Command-Line Parsing**: Automatically retrieves command-line options and descriptions from the executable’s help output.
- **PyBind11 Code Generation**: Generates C++ code that wraps the executable, allowing it to be called from Python.
- **CMake Integration**: Creates a `CMakeLists.txt` file for building the generated PyBind11 module.
- **Enhanced Logging**: Uses the Loguru library for structured logging.
- **Beautiful Terminal Output**: Employs the Rich library for styled console output, including tables for command options.
- **Environment Variable Management**: Provides functionality to set environment variables for the executable.

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

- **C++ Compiler**: A compatible C++ compiler (e.g., GCC, Clang) must be installed to compile the generated bindings.
- **CMake**: CMake must be installed for building the PyBind11 module.

---

## Usage

To run the script, use the following command:

```bash
python exebind.py <executable_path> [--module-name <module_name>] [--output-dir <output_directory>]
```

### Command-Line Arguments

- **`executable_path`**: The path to the executable for which bindings will be generated.
- **`--module-name`**: (Optional) The name of the Python module to generate. If not provided, it defaults to the basename of the executable with `_bindings` appended.
- **`--output-dir`**: (Optional) The output directory for the generated code. Defaults to `bindings`.

---

## Structure of the Script

### Key Functions

1. **`get_executable_info(executable_path: str) -> List[Tuple[str, str]]`**  
   Retrieves the help information of the executable and parses command options and their descriptions.

2. **`generate_pybind11_code(module_name: str, executable_name: str, command_info: List[Tuple[str, str]]) -> str`**  
   Generates the C++ code required to create PyBind11 bindings for the executable.

3. **`generate_cmake_file(module_name: str) -> str`**  
   Creates a `CMakeLists.txt` file to build the PyBind11 module.

4. **`check_pybind11_installed()`**  
   Checks if PyBind11 is installed and installs it if it is not.

5. **`parse_arguments()`**  
   Parses command-line arguments and returns them.

6. **`display_command_options(command_info: List[Tuple[str, str]])`**  
   Displays the parsed command options in a formatted table.

7. **`main()`**  
   The main function that orchestrates the script execution, including parsing arguments, generating bindings, and writing output files.

---

## How It Works

1. **Argument Parsing**  
   The script begins by parsing command-line arguments to retrieve the path to the executable and optional parameters for module name and output directory.

2. **Executable Information Retrieval**  
   It then calls the executable with the `--help` flag to retrieve its command-line options and descriptions. This information is parsed using regular expressions.

3. **Code Generation**  
   Using the parsed command options, the script generates the necessary PyBind11 code to wrap the executable. It also creates a `CMakeLists.txt` file for building the module.

4. **User Confirmation**  
   Before proceeding with file generation, the script prompts the user for confirmation to ensure they wish to continue.

5. **File Writing**  
   Finally, the generated C++ code and CMake file are written to the specified output directory.

---

## Example of Generated Code

The generated PyBind11 bindings will look similar to this:

```cpp
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <array>
#include <memory>

namespace py = pybind11;

// Function to run a command and capture its output
std::string run_command(const std::string& args) {
    std::string command = "executable_name " + args;
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// Wrapper class for the executable
class executable_name_Wrapper {
public:
    // Run the executable with arguments and return output
    std::string run(const std::string& args) {
        return run_command(args);
    }

    // Set environment variables
    void set_env(const std::string& key, const std::string& value) {
        setenv(key.c_str(), value.c_str(), 1);
    }

    // Additional method bindings...
};

// Create Python module
PYBIND11_MODULE(module_name, m) {
    py::class_<executable_name_Wrapper>(m, "executable_name_Wrapper")
        .def(py::init<>())
        .def("run", &executable_name_Wrapper::run, "Run the executable with arguments")
        .def("set_env", &executable_name_Wrapper::set_env, "Set an environment variable")
        // Additional method bindings...
    ;
}
```

The generated `CMakeLists.txt` will look like this:

```cmake
cmake_minimum_required(VERSION 3.14)
project(module_name)

set(CMAKE_CXX_STANDARD 17)
find_package(pybind11 REQUIRED)

pybind11_add_module(module_name bindings.cpp)
```

---

## Error Handling

The script includes robust error handling:

- If the executable fails to run or parse, an error message is logged, and the script exits.
- If PyBind11 is not installed, it will be automatically installed.
- If any unexpected errors occur during execution, they are logged, and the script exits gracefully.

---

## Conclusion

The **Executable Binder Script** simplifies the process of creating Python bindings for C++ executables using PyBind11. By automating the retrieval of command options and generating the required code, it significantly speeds up the development process for developers looking to integrate C++ functionality into Python applications.
