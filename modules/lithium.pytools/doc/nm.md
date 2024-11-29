# NM Tool for Analyzing Symbols in Binary Files Documentation

This document provides a comprehensive guide on how to use the **NM Tool**, a command-line utility designed for analyzing symbols in binary files. The tool leverages the `nm` command to retrieve, filter, search, count, and export symbols from binaries, enhancing functionality with robust exception handling and detailed logging.

---

## Key Features

- **Symbol Retrieval**: Load symbols from binaries using the `nm` command.
- **Filtering and Searching**: Filter symbols by type or name using regular expressions.
- **Count Symbols**: Count the number of symbols by type.
- **Export Symbols**: Export symbols to various formats (e.g., TXT, CSV, JSON, XML).
- **Display Symbol Information**: Present symbols in a formatted table using Rich for enhanced terminal output.
- **Error Handling**: Custom exceptions for better error management.
- **Logging**: Detailed logging of operations and errors using the `Loguru` library.

---

## Requirements

- Python 3.7 or higher.
- Required libraries: `loguru`, `rich`.

Install the required libraries using pip:

```bash
pip install loguru rich
```

Make sure the `nm` command is available on your system (usually found in Unix-like environments).

---

## Usage

The script can be executed from the command line with various options. The command syntax is as follows:

```bash
python nm_tool.py <binary> [options]
```

### Command-Line Options

- **`<binary>`**: Required. Path to the binary file to analyze.
- **`-f`, `--filter`**: Optional. Filter symbols by type (e.g., `T`, `D`, `B`).
- **`-s`, `--search`**: Optional. Search for a specific symbol by name.
- **`-a`, `--address`**: Optional. Find symbol by address.
- **`-d`, `--detailed`**: Optional. Display detailed output for symbols.
- **`-c`, `--count`**: Optional. Count symbols by type.
- **`-e`, `--export`**: Optional. Export symbols to a file (supports TXT, CSV, JSON, XML).
- **`-v`, `--verbose`**: Optional. Increase output verbosity.
- **`--demangle`**: Optional. Demangle symbol names.
- **`--extern`**: Optional. Display only external symbols.
- **`--size`**: Optional. Display symbols with sizes.
- **`--pattern`**: Optional. Search symbols matching a regex pattern.

---

## Example Usage

### Analyze a Binary File

To analyze a binary file and retrieve its symbols:

```bash
python nm_tool.py /path/to/binary
```

### Filter Symbols by Type

To filter symbols by type (e.g., `T` for text symbols):

```bash
python nm_tool.py /path/to/binary -f T
```

### Search for a Specific Symbol

To search for a specific symbol by name:

```bash
python nm_tool.py /path/to/binary -s my_function
```

### Count Symbols by Type

To count the number of symbols by type:

```bash
python nm_tool.py /path/to/binary -c
```

### Export Symbols to CSV

To export symbols to a CSV file:

```bash
python nm_tool.py /path/to/binary -e symbols.csv
```

### Display Detailed Information

To display detailed information about the symbols:

```bash
python nm_tool.py /path/to/binary -d
```

### List Symbols with Sizes

To retrieve and display symbols along with their sizes:

```bash
python nm_tool.py /path/to/binary --size
```

---

## Error Handling and Logging

The script uses the `Loguru` library for logging. Logs are written to `nm_tool.log` and the console, providing detailed information about operations, warnings, and errors. This helps in tracking the actions performed by the script and diagnosing issues.

---

## Conclusion

The **NM Tool for Analyzing Symbols in Binary Files** is a powerful utility for developers and system administrators. It simplifies the process of analyzing binary files and retrieving symbol information while providing robust error handling and logging capabilities. By following this documentation, users can effectively utilize the tool for their binary analysis needs.
