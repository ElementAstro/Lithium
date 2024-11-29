# 7z Command-Line Tool Wrapper Documentation

This document provides a comprehensive guide on how to use the **7z Command-Line Tool Wrapper**, a Python script designed to simplify the usage of the `7z` command-line utility for file compression and extraction. This tool provides functionalities such as compressing files, extracting archives, listing contents, and testing archive integrity, all while enhancing logging and user feedback through the console.

---

## Key Features

- **Compression**: Compress files into various formats using specified compression levels and optional passwords.
- **Extraction**: Extract files from archives with options for overwriting existing files.
- **Listing Contents**: List the contents of an archive.
- **Integrity Testing**: Test the integrity of archives to ensure they are not corrupted.
- **File Management**: Delete archives and update them by adding or removing files.
- **Logging**: Detailed logging of operations and errors using the `Loguru` library.
- **Rich Console Output**: Provides progress feedback and formatted output using the `Rich` library.

---

## Requirements

- Python 3.7 or higher.
- Required libraries: `paramiko`, `loguru`, `rich`.

Install the required libraries using pip:

```bash
pip install paramiko loguru rich
```

Make sure the `7z` executable is installed and accessible in your system's PATH. You can download it from [7-Zip](https://www.7-zip.org/download.html).

---

## Usage

The script can be executed from the command line with various options. The command syntax is as follows:

```bash
python 7z_wrapper.py <action> [options]
```

### Available Actions

1. **Compress Files**

   ```bash
   python 7z_wrapper.py compress -f <file1> <file2> ... -a <archive> [--level <level>] [--password <password>] [--format <format>] [--exclude <pattern1> <pattern2> ...]
   ```

   - **`-f`**: Required. List of files to compress.
   - **`-a`**: Required. Path to the archive.
   - **`--level`**: Compression level (0-9, default is 5).
   - **`--password`**: Optional. Password for the archive.
   - **`--format`**: Archive format (default is '7z').
   - **`--exclude`**: Patterns to exclude from compression.

2. **Extract Files**

   ```bash
   python 7z_wrapper.py extract -a <archive> -d <destination> [--password <password>] [--force] [--overwrite]
   ```

   - **`-a`**: Required. Path to the archive.
   - **`-d`**: Required. Path to the extraction destination.
   - **`--password`**: Optional. Password for the archive.
   - **`--force`**: Optional. Force overwrite if the destination directory exists.
   - **`--overwrite`**: Optional. Overwrite existing files.

3. **List Archive Contents**

   ```bash
   python 7z_wrapper.py list -a <archive> [--password <password>]
   ```

   - **`-a`**: Required. Path to the archive.
   - **`--password`**: Optional. Password for the archive.

4. **Test Archive Integrity**

   ```bash
   python 7z_wrapper.py test -a <archive> [--password <password>]
   ```

   - **`-a`**: Required. Path to the archive.
   - **`--password`**: Optional. Password for the archive.

5. **Delete Archive**

   ```bash
   python 7z_wrapper.py delete -a <archive>
   ```

   - **`-a`**: Required. Path to the archive.

6. **Update Archive**

   ```bash
   python 7z_wrapper.py update -a <archive> -f <file1> <file2> ... [--add] [--delete]
   ```

   - **`-a`**: Required. Path to the archive.
   - **`-f`**: Required. List of files to add or delete.
   - **`--add`**: Optional. Add files to the archive.
   - **`--delete`**: Optional. Delete files from the archive.

7. **Get 7z Version**

   ```bash
   python 7z_wrapper.py version
   ```

8. **List Supported Formats**

   ```bash
   python 7z_wrapper.py formats
   ```

---

## Example Usage

### Compress Files

To compress files into an archive:

```bash
python 7z_wrapper.py compress -f file1.txt file2.txt -a archive.7z --level 5 --password mysecret
```

### Extract Files

To extract an archive:

```bash
python 7z_wrapper.py extract -a archive.7z -d ./extracted --password mysecret --force
```

### List Archive Contents

To list the contents of an archive:

```bash
python 7z_wrapper.py list -a archive.7z
```

### Test Archive Integrity

To test the integrity of an archive:

```bash
python 7z_wrapper.py test -a archive.7z
```

### Delete an Archive

To delete an archive:

```bash
python 7z_wrapper.py delete -a archive.7z
```

### Update an Archive

To update an archive by adding files:

```bash
python 7z_wrapper.py update -a archive.7z -f file3.txt --add
```

### Get 7z Version

To get the version of the 7z utility:

```bash
python 7z_wrapper.py version
```

### List Supported Formats

To list the supported archive formats:

```bash
python 7z_wrapper.py formats
```

---

## Error Handling and Logging

The script uses the `Loguru` library for logging. Logs are written to `sftp_client.log` and the console, providing detailed information about operations, warnings, and errors. This helps in tracking the actions performed by the script and diagnosing issues.

---

## Conclusion

The **7z Command-Line Tool Wrapper** is a versatile utility for managing file compression and extraction. It simplifies the process of working with archives while providing robust error handling and logging capabilities. By following this documentation, users can effectively utilize the tool for their file management needs.
