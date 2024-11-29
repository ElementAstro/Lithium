# File Display and Validation CLI Tool Documentation

## Overview

This **File Display and Validation CLI Tool** is designed to pretty-print and validate various types of data files such as JSON, YAML, TOML, XML, CSV, and INI. The tool provides a user-friendly command-line interface to format and display files with beautiful syntax highlighting using the **Rich** library, and it includes error handling and logging through **Loguru**.

It supports multiple file formats, syntax validation, and customized output options. The tool also allows reading from standard input and supports overwriting output files when necessary.

---

## Features

- **Pretty Print Files**: Format and display the content of JSON, YAML, TOML, XML, CSV, and INI files with syntax highlighting.
- **Validate File Syntax**: Validate file contents without displaying them.
- **Read from Standard Input**: Support for reading files via standard input (`stdin`).
- **Rich Output**: Use **Rich** for terminal output, including syntax highlighting and formatted tables.
- **Logging**: Detailed logging with **Loguru**, including support for verbose logging.
- **File Overwriting**: Option to overwrite existing output files.
- **Customizable Options**: Choose indentation level, output format, and more.

---

## Requirements

- Python 3.x
- Required Python packages:
  - `loguru`: For logging.
  - `rich`: For terminal output and syntax highlighting.
  - `yaml`: For parsing YAML files.
  - `toml`: For parsing TOML files.
  - `xml.etree.ElementTree`: For parsing XML files.
  - `csv`: For handling CSV files.
  - `configparser`: For parsing INI files.

Install the required packages using:

```bash
pip install loguru rich pyyaml toml
```

---

## Usage

### Command-Line Arguments

The tool provides several command-line options to process the files.

#### Arguments

- **`files`**: The path(s) to the file(s) or `-` to read from standard input (e.g., `example.json`, `example.yaml`, etc.).
- **`--format`**: Specify the format of the file if the file extension doesn't match the content (e.g., `json`, `yaml`, `toml`, `xml`, `csv`, `ini`).
- **`--output`**: Path to the output file where the content will be saved. If the output file exists, the tool will ask if it should overwrite it.
- **`--overwrite`**: Flag to overwrite the output file if it exists without asking.
- **`--theme`**: Code highlighting theme for syntax highlighting (default is `monokai`).
- **`--indent`**: Indentation level for pretty-printing (default is 4).
- **`--validate`**: Flag to validate the file's syntax without displaying it.
- **`--verbose`**: Enable verbose logging for more detailed output.

#### Examples

1. **Pretty Print a JSON File**:

```bash
python ra.py example.json --format json
```

2. **Validate a YAML File**:

```bash
python ra.py example.yaml --validate
```

3. **Pretty Print Multiple Files and Overwrite Output**:

```bash
python ra.py example1.json example2.yaml --output output.json --overwrite
```

4. **Read from Standard Input**:

```bash
cat example.toml | python ra.py - --format toml
```

---

## Functions

### `pretty_print_json(data: str, indent: Optional[int] = 4) -> None`

Formats and prints JSON data using the specified indentation level.

### `pretty_print_yaml(data: str) -> None`

Formats and prints YAML data.

### `pretty_print_toml(data: str) -> None`

Formats and prints TOML data.

### `pretty_print_xml(data: str) -> None`

Formats and prints XML data.

### `pretty_print_csv(data: str) -> None`

Formats and prints CSV data in a table format using **Rich**.

### `pretty_print_ini(data: str) -> None`

Formats and prints INI data in a table format using **Rich**.

### `validate_file(file_path: Union[Path, str], file_format: str) -> bool`

Validates the syntax of the specified file based on its format (JSON, YAML, TOML, XML, CSV, INI).

### `display_file(file_path: Path, file_format: str, output: Optional[Path] = None, indent: Optional[int] = 4, validate: bool = False) -> None`

Displays the content of the file in a formatted manner based on its format. If `validate` is `True`, it will only validate the file without displaying it.

### `read_stdin() -> str`

Reads data from standard input (useful for piping data into the script).

---

## Logging

The tool uses **Loguru** for logging, with the following features:

- **Log Level**: The log level is set to `DEBUG` by default.
- **Log File**: Logs are stored in `ra.log`.
- **Verbose Mode**: When `--verbose` is enabled, logs are written in more detail, including every step of the file processing.

---

## Example JSON Configuration

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

---

## Conclusion

This **File Display and Validation CLI Tool** simplifies the process of managing and working with various file formats. With support for **JSON**, **YAML**, **TOML**, **XML**, **CSV**, and **INI** files, it makes file validation, formatting, and display accessible through a clean command-line interface. Enhanced with **Rich** for output and **Loguru** for logging, it provides a powerful tool for developers and system administrators alike.
