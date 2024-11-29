# Enhanced JSON Command-Line Utility Documentation

This document provides a comprehensive guide on how to use the **Enhanced JSON Command-Line Utility**, a Python script designed to manage and manipulate JSON files. The tool supports various functionalities such as loading, printing, converting, querying, validating, merging, and comparing JSON files, with enhanced logging and user-friendly terminal outputs.

---

## Key Features

- **Load JSON Files**: Load JSON data from specified files.
- **Print JSON**: Print JSON data in a formatted or minified style.
- **Convert to YAML**: Convert JSON files to YAML format.
- **Query JSON**: Query JSON data using dot notation.
- **Validate JSON**: Validate the format of JSON files.
- **Merge JSON Files**: Merge multiple JSON files into one.
- **Compare JSON Files**: Compare two JSON files and display differences.
- **Export to XML**: Export JSON data to XML format.
- **Display Statistics**: Show statistics about the JSON structure.
- **Flatten/Unflatten JSON**: Flatten nested JSON structures and unflatten them back.
- **Remove/Rename Keys**: Remove specific keys from JSON data or rename them.
- **Enhanced Logging**: Uses the `Loguru` library for detailed logging.
- **Rich Console Output**: Utilizes the `Rich` library for beautified terminal outputs.

---

## Requirements

- Python 3.7 or higher.
- Required libraries: `loguru`, `rich`, `PyYAML`, `dicttoxml`.

Install the required libraries using pip:

```bash
pip install loguru rich PyYAML dicttoxml
```

---

## Usage

The script can be executed from the command line with various options. The command syntax is as follows:

```bash
python pyjson.py <file> [options]
```

### Command-Line Options

- **`<file>`**: Required. Path to the JSON file to process.
- **`--minify`**: Minify JSON output.
- **`--format`**: Format JSON output with indentation.
- **`--yaml <OUTPUT_FILE>`**: Convert JSON to YAML and save to a file.
- **`--query <QUERY_PATH>`**: Query JSON data using simple dot notation (e.g., `a.b.0.c`).
- **`--validate`**: Validate JSON file format.
- **`--merge <FILES>`**: Merge multiple JSON files.
- **`--diff <FILE1> <FILE2>`**: Compare two JSON files.
- **`--export-xml <OUTPUT>`**: Export JSON to XML format.
- **`--stats`**: Display statistics about the JSON structure.
- **`--flatten`**: Flatten nested JSON structures.
- **`--unflatten`**: Unflatten JSON structures.
- **`--remove-key <KEY>`**: Remove a specific key from the JSON data.
- **`--rename-key <OLD> <NEW>`**: Rename a key in the JSON data.
- **`--help`**: Show help message and exit.

---

## Example Usage

### Load and Print JSON

To load a JSON file and print it:

```bash
python pyjson.py data.json --format
```

### Convert JSON to YAML

To convert a JSON file to YAML:

```bash
python pyjson.py data.json --yaml output.yaml
```

### Query JSON Data

To query specific data in the JSON:

```bash
python pyjson.py data.json --query "users.0.name"
```

### Validate JSON Format

To validate the format of a JSON file:

```bash
python pyjson.py data.json --validate
```

### Merge Multiple JSON Files

To merge multiple JSON files:

```bash
python pyjson.py data.json --merge file1.json file2.json
```

### Compare Two JSON Files

To compare two JSON files:

```bash
python pyjson.py data.json --diff file1.json file2.json
```

### Export JSON to XML

To export JSON data to XML format:

```bash
python pyjson.py data.json --export-xml output.xml
```

### Display Statistics

To display statistics about the JSON structure:

```bash
python pyjson.py data.json --stats
```

### Flatten Nested JSON

To flatten nested JSON structures:

```bash
python pyjson.py data.json --flatten
```

### Unflatten JSON

To unflatten a JSON structure:

```bash
python pyjson.py data.json --unflatten
```

### Remove a Key

To remove a specific key from the JSON data:

```bash
python pyjson.py data.json --remove-key "unwanted_key"
```

### Rename a Key

To rename a key in the JSON data:

```bash
python pyjson.py data.json --rename-key "old_key" "new_key"
```

---

## Error Handling and Logging

The script uses the `Loguru` library for logging. Logs are written to `pyjson.log` and the console, providing detailed information about operations, warnings, and errors. This helps in tracking the actions performed by the script and diagnosing issues.

---

## Conclusion

The **Enhanced JSON Command-Line Utility** is a powerful tool for managing and manipulating JSON files. It simplifies the process of handling JSON data while providing robust error handling and logging capabilities. By following this documentation, users can effectively utilize the tool for their JSON management needs.
