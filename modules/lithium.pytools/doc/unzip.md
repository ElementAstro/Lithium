# Unzip Command-Line Tool Wrapper Documentation

This document provides a detailed guide on how to use the **Unzip Command-Line Tool Wrapper**. This Python script provides a convenient interface for extracting, listing, testing, and deleting zip archives using the `unzip` command-line utility. It includes error handling, logging, and validation features to ensure smooth operation.

---

## Features

- **Extraction**: Extract zip archives to a specified destination.
- **Listing Contents**: List the contents of zip archives.
- **Integrity Testing**: Test the integrity of zip archives to ensure they are not corrupted.
- **Deletion**: Delete zip archives.
- **Logging**: Detailed logging of operations and errors using `loguru`.
- **Custom Exception Handling**: Custom exceptions for better error handling.

---

## Requirements

- Python 3.6 or higher.
- Required libraries: `loguru`.

Install the required libraries using pip:

```bash
pip install loguru
```

You also need the `unzip` command-line utility installed on your system. This is typically available on Unix-like systems (Linux, macOS) and can be installed on Windows via utilities like Cygwin or Windows Subsystem for Linux (WSL).

---

## Usage

The script can be executed from the command line with various subcommands for different actions. The command syntax is as follows:

```bash
python unzip_wrapper.py <action> [options]
```

### Available Actions

1. **extract**: Extract files from a zip archive.
2. **list**: List the contents of a zip archive.
3. **test**: Test the integrity of a zip archive.
4. **delete**: Delete a zip archive.

### Command-Line Arguments

#### Extract Action

```bash
python unzip_wrapper.py extract -a <archive> -d <destination> [-p <password>] [--force]
```

- **`-a`, `--archive`**: Required. Path to the zip archive.
- **`-d`, `--destination`**: Required. Path to the extraction destination.
- **`-p`, `--password`**: Optional. Password for the archive.
- **`--force`**: Optional. Force overwrite if the destination directory exists.

#### List Action

```bash
python unzip_wrapper.py list -a <archive> [-p <password>]
```

- **`-a`, `--archive`**: Required. Path to the zip archive.
- **`-p`, `--password`**: Optional. Password for the archive.

#### Test Action

```bash
python unzip_wrapper.py test -a <archive> [-p <password>]
```

- **`-a`, `--archive`**: Required. Path to the zip archive.
- **`-p`, `--password`**: Optional. Password for the archive.

#### Delete Action

```bash
python unzip_wrapper.py delete -a <archive>
```

- **`-a`, `--archive`**: Required. Path to the zip archive.

---

## How It Works

1. **Initialization**: The `UnzipWrapper` class is initialized with the path to the `unzip` executable. It checks if the executable exists and is valid.
2. **Command Execution**: The script executes the specified action by calling the appropriate method in the `UnzipWrapper` class:
   - **Extract**: Validates the archive, checks the destination, and runs the unzip command to extract files.
   - **List**: Validates the archive and runs the unzip command to list its contents.
   - **Test**: Validates the archive and runs the unzip command to test its integrity.
   - **Delete**: Validates the archive and deletes it.
3. **Error Handling**: Custom exceptions are raised for different types of errors, such as validation failures, extraction issues, and command execution failures. These are logged for troubleshooting.

---

## Key Methods in the UnzipWrapper Class

- **`__init__(self, executable: str = "unzip")`**: Initializes the unzip command-line tool wrapper.
- **`_run_command(self, args: List[str])`**: Runs the unzip command and captures the output.
- **`_validate_archive_exists(self, archive: Union[str, Path])`**: Validates that the specified archive exists.
- **`extract(self, archive: Union[str, Path], destination: Union[str, Path], password: Optional[str] = None, force: bool = False)`**: Extracts the specified archive to the destination folder.
- **`list_contents(self, archive: Union[str, Path], password: Optional[str] = None) -> str`**: Lists the contents of the archive.
- **`test_integrity(self, archive: Union[str, Path], password: Optional[str] = None) -> bool`**: Tests the integrity of the archive.
- **`delete_archive(self, archive: Union[str, Path])`**: Deletes the specified archive.
- **`execute(self, action: str, ...)`**: Executes the corresponding unzip command based on the action type.

---

## Example Usage

### Extracting an Archive

```bash
python unzip_wrapper.py extract -a /path/to/archive.zip -d /path/to/destination --force
```

### Listing Contents of an Archive

```bash
python unzip_wrapper.py list -a /path/to/archive.zip
```

### Testing Integrity of an Archive

```bash
python unzip_wrapper.py test -a /path/to/archive.zip
```

### Deleting an Archive

```bash
python unzip_wrapper.py delete -a /path/to/archive.zip
```

---

## Logging

The script uses the `loguru` library for logging. Logs are generated for various operations, including command execution, success messages, and error handling. This helps in tracking the operations performed by the script and diagnosing issues.

---

## Conclusion

The **Unzip Command-Line Tool Wrapper** is a powerful utility for managing zip archives directly from the command line. It simplifies the process of extracting, listing, testing, and deleting zip files while providing robust error handling and logging capabilities. By following this documentation, users can effectively utilize the wrapper for their archive management tasks.
