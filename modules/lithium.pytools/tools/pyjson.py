#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
@file         pyjson.py
@brief        An enhanced command-line utility to manage and manipulate JSON files.

@details      This script provides functionality to load, print, convert, query, validate,
              merge, and compare JSON files. It enhances internal operations with robust
              exception handling and detailed logging using Loguru.

              Usage:
                python pyjson.py <file> [options]

              Options:
                --minify                Minify JSON output.
                --format                Format JSON output with indentation.
                --yaml <OUTPUT_FILE>    Convert JSON to YAML and save to a file.
                --query <QUERY_PATH>    Query JSON data using simple dot notation (e.g., 'a.b.0.c').
                --validate              Validate JSON file format.
                --merge <FILES>         Merge multiple JSON files.
                --diff <FILE1> <FILE2>  Compare two JSON files.
                --export-xml <OUTPUT>   Export JSON to XML format.
                --stats                 Display statistics about the JSON structure.
                --flatten               Flatten nested JSON structures.
                --unflatten             Unflatten JSON structures.
                --remove-key <KEY>      Remove a specific key from the JSON data.
                --rename-key <OLD> <NEW> Rename a key in the JSON data.
                --help                  Show help message and exit.

@requires     - Python 3.7+
              - `loguru` Python library
              - `PyYAML` Python library (optional for YAML conversion)
              - `dicttoxml` Python library (optional for XML export)

@version      3.1
@date         2024-05-01
"""

import json
import argparse
import sys
from json.decoder import JSONDecodeError
from loguru import logger
from typing import Optional, List, Dict, Any

try:
    import yaml
except ImportError:
    yaml = None

try:
    from dicttoxml import dicttoxml
except ImportError:
    dicttoxml = None

from rich.console import Console
from rich.table import Table
from rich.traceback import install

# Initialize Rich console and install rich traceback
console = Console()
install(show_locals=True)

# Configure Loguru
logger.remove()
logger.add(
    "pyjson.log",
    rotation="5 MB",
    retention="7 days",
    compression="zip",
    enqueue=True,
    encoding="utf-8",
    format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level}</level> | {message}",
    level="DEBUG",
)
logger.add(
    sys.stderr,
    level="INFO",
    format="<level>{message}</level>",
)
logger.debug("Logging is configured.")


def load_json(file_path: str) -> Dict[str, Any]:
    """
    Load JSON data from a file and return it as a dictionary.

    Args:
        file_path (str): Path to the JSON file.

    Returns:
        dict: The parsed JSON data as a Python dictionary.

    Raises:
        SystemExit: If the file is not found, cannot be read, or contains invalid JSON.
    """
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            logger.debug(f"Loading JSON from file: {file_path}")
            data = json.load(file)
            logger.info(f"Successfully loaded JSON from '{file_path}'.")
            return data
    except JSONDecodeError as e:
        logger.error(f"Invalid JSON in file '{file_path}': {e}")
        console.print(
            f"[bold red]Invalid JSON in file '{file_path}': {e}[/bold red]")
        sys.exit(1)
    except FileNotFoundError:
        logger.error(f"File not found: {file_path}")
        console.print(f"[bold red]File not found: {file_path}[/bold red]")
        sys.exit(1)
    except IOError as e:
        logger.error(f"Error reading file '{file_path}': {e}")
        console.print(
            f"[bold red]Error reading file '{file_path}': {e}[/bold red]")
        sys.exit(1)


def print_json(obj: Dict[str, Any], minify: bool = False, indent: Optional[int] = 4):
    """
    Print a JSON object as a formatted or minified string using Rich.

    Args:
        obj (dict): The JSON object to print.
        minify (bool, optional): Whether to print the JSON in minified form. Defaults to False.
        indent (int, optional): Indentation level for pretty-printing. Defaults to 4.

    Returns:
        None
    """
    try:
        if minify:
            json_str = json.dumps(obj, separators=(',', ':'))
            logger.debug("Printing minified JSON.")
            console.print(f"[bold green]{json_str}[/bold green]")
        else:
            json_str = json.dumps(obj, indent=indent)
            logger.debug(f"Printing formatted JSON with indent={indent}.")
            console.print_json(json_str)
    except (TypeError, OverflowError) as e:
        logger.error(f"Error printing JSON: {e}")
        console.print(f"[bold red]Error printing JSON: {e}[/bold red]")
        sys.exit(1)


def save_json_to_yaml(json_obj: Dict[str, Any], output_file: str):
    """
    Save a JSON object to a YAML file.

    Args:
        json_obj (dict): The JSON object to convert to YAML.
        output_file (str): Path to the output YAML file.

    Returns:
        None

    Raises:
        ImportError: If PyYAML is not installed.
    """
    if yaml is None:
        logger.error(
            "YAML support is not available. Install PyYAML to enable this feature.")
        console.print(
            "[bold red]YAML support is not available. Install PyYAML to enable this feature.[/bold red]")
        return
    try:
        with open(output_file, 'w', encoding='utf-8') as file:
            yaml.dump(json_obj, file, allow_unicode=True,
                      default_flow_style=False)
        logger.info(
            f"Successfully converted JSON to YAML and saved to '{output_file}'.")
        console.print(
            f"[bold green]YAML saved to '{output_file}'.[/bold green]")
    except IOError as e:
        logger.error(f"Error writing YAML file '{output_file}': {e}")
        console.print(
            f"[bold red]Error writing YAML file '{output_file}': {e}[/bold red]")
        sys.exit(1)


def export_json_to_xml(json_obj: Dict[str, Any], output_file: str):
    """
    Export a JSON object to an XML file.

    Args:
        json_obj (dict): The JSON object to convert to XML.
        output_file (str): Path to the output XML file.

    Returns:
        None

    Raises:
        ImportError: If dicttoxml is not installed.
    """
    if dicttoxml is None:
        logger.error(
            "XML support is not available. Install dicttoxml to enable this feature.")
        console.print(
            "[bold red]XML support is not available. Install dicttoxml to enable this feature.[/bold red]")
        return
    try:
        xml_bytes = dicttoxml(json_obj, custom_root='root', attr_type=False)
        xml_str = xml_bytes.decode('utf-8')
        with open(output_file, 'w', encoding='utf-8') as file:
            file.write(xml_str)
        logger.info(
            f"Successfully exported JSON to XML and saved to '{output_file}'.")
        console.print(
            f"[bold green]XML exported to '{output_file}'.[/bold green]")
    except Exception as e:
        logger.error(f"Error exporting JSON to XML: {e}")
        console.print(f"[bold red]Error exporting JSON to XML: {e}[/bold red]")
        sys.exit(1)


def query_json(json_obj: Dict[str, Any], query_path: str):
    """
    Query a JSON object using a simple dot notation path.

    Args:
        json_obj (dict): The JSON object to query.
        query_path (str): Dot notation path to the desired data (e.g., 'a.b.0.c').

    Returns:
        None

    Raises:
        KeyError, IndexError, TypeError: If the query path is invalid or the data cannot be accessed.
    """
    try:
        logger.debug(f"Querying JSON with path: {query_path}")
        parts = query_path.strip().split('.')
        result = json_obj
        for part in parts:
            if part.isdigit():
                index = int(part)
                result = result[index]
            else:
                result = result[part]
        logger.info(f"Query successful for path '{query_path}'.")
        print_json(result)
    except (KeyError, IndexError, TypeError) as e:
        logger.error(f"Failed to query JSON with path '{query_path}': {e}")
        console.print(
            f"[bold red]Failed to query JSON with path '{query_path}': {e}[/bold red]")
        sys.exit(1)


def validate_json(file_path: str):
    """
    Validate the format of a JSON file.

    Args:
        file_path (str): Path to the JSON file.

    Returns:
        None
    """
    try:
        load_json(file_path)
        logger.info(f"JSON file '{file_path}' is valid.")
        console.print(
            f"[bold green]JSON file '{file_path}' is valid.[/bold green]")
    except SystemExit:
        logger.warning(f"JSON file '{file_path}' is invalid.")
        console.print(
            f"[bold red]JSON file '{file_path}' is invalid.[/bold red]")
        sys.exit(1)


def merge_json(files: List[str]) -> Dict[str, Any]:
    """
    Merge multiple JSON files into one.

    Args:
        files (list): List of file paths to JSON files.

    Returns:
        dict: The merged JSON data.
    """
    merged_data = {}
    for file in files:
        logger.debug(f"Merging file: {file}")
        data = load_json(file)
        merged_data.update(data)
    logger.info(f"Successfully merged {len(files)} files.")
    console.print(
        f"[bold green]Successfully merged {len(files)} files.[/bold green]")
    return merged_data


def diff_json(file1: str, file2: str):
    """
    Compare two JSON files and display the differences in a table.

    Args:
        file1 (str): Path to the first JSON file.
        file2 (str): Path to the second JSON file.

    Returns:
        None
    """
    logger.debug(f"Comparing files '{file1}' and '{file2}'.")
    data1 = load_json(file1)
    data2 = load_json(file2)

    diff = {key: data2[key]
            for key in data2 if key not in data1 or data1[key] != data2[key]}
    logger.info(f"Differences found between '{file1}' and '{file2}'.")

    if diff:
        table = Table(title=f"Differences between '{file1}' and '{file2}'")
        table.add_column("Key", style="cyan", no_wrap=True)
        table.add_column("Value in File2", style="magenta")
        for key, value in diff.items():
            table.add_row(key, str(value))
        console.print(table)
    else:
        console.print("[bold green]No differences found.[/bold green]")


def display_stats(json_obj: Dict[str, Any]):
    """
    Display statistics about the JSON structure.

    Args:
        json_obj (dict): The JSON object to analyze.

    Returns:
        None
    """
    try:
        total_keys = count_keys(json_obj)
        total_elements = count_elements(json_obj)
        depth = get_depth(json_obj)
        logger.info("Displaying JSON statistics.")

        table = Table(title="JSON Statistics")
        table.add_column("Statistic", style="cyan", no_wrap=True)
        table.add_column("Value", style="magenta")
        table.add_row("Total Keys", str(total_keys))
        table.add_row("Total Elements", str(total_elements))
        table.add_row("Depth", str(depth))
        console.print(table)
    except Exception as e:
        logger.error(f"Error displaying JSON statistics: {e}")
        console.print(
            f"[bold red]Error displaying JSON statistics: {e}[/bold red]")
        sys.exit(1)


def count_keys(obj: Any) -> int:
    """Recursively count the number of keys in the JSON object."""
    if isinstance(obj, dict):
        return sum(count_keys(v) for v in obj.values()) + len(obj)
    elif isinstance(obj, list):
        return sum(count_keys(item) for item in obj)
    else:
        return 0


def count_elements(obj: Any) -> int:
    """Recursively count the number of elements in the JSON object."""
    if isinstance(obj, dict):
        return sum(count_elements(v) for v in obj.values()) + 1
    elif isinstance(obj, list):
        return sum(count_elements(item) for item in obj) + len(obj)
    else:
        return 1


def get_depth(obj: Any, current_depth: int = 1) -> int:
    """Recursively determine the depth of the JSON object."""
    if isinstance(obj, dict):
        return max([get_depth(v, current_depth + 1) for v in obj.values()], default=current_depth)
    elif isinstance(obj, list):
        return max([get_depth(item, current_depth + 1) for item in obj], default=current_depth)
    else:
        return current_depth


def flatten_json(y: Dict[str, Any]) -> Dict[str, Any]:
    """
    Flatten a nested JSON object.

    Args:
        y (dict): The JSON object to flatten.

    Returns:
        dict: The flattened JSON object.
    """
    out = {}

    def flatten(x: Any, name: str = ''):
        if isinstance(x, dict):
            for a in x:
                flatten(x[a], f"{name}{a}.")
        elif isinstance(x, list):
            for i, a in enumerate(x):
                flatten(a, f"{name}{i}.")
        else:
            out[name[:-1]] = x

    flatten(y)
    logger.info("Successfully flattened JSON.")
    return out


def unflatten_json(x: Dict[str, Any]) -> Dict[str, Any]:
    """
    Unflatten a JSON object.

    Args:
        x (dict): The flattened JSON object.

    Returns:
        dict: The unflattened JSON object.
    """
    result_dict = {}
    for key, value in x.items():
        parts = key.split('.')
        d = result_dict
        for part in parts[:-1]:
            if part.isdigit():
                part = int(part)
                if not isinstance(d, list):
                    d_type = type(d)
                    d = []
                while len(d) <= part:
                    d.append({})
            else:
                d = d.setdefault(part, {})
        last_part = parts[-1]
        if last_part.isdigit():
            last_part = int(last_part)
            if not isinstance(d, list):
                d = []
            while len(d) <= last_part:
                d.append(None)
            d[last_part] = value
        else:
            d[last_part] = value
    logger.info("Successfully unflattened JSON.")
    return result_dict


def flatten_data(json_obj: Dict[str, Any]):
    """
    Flatten the JSON data and print it.

    Args:
        json_obj (dict): The JSON object to flatten.

    Returns:
        None
    """
    flattened = flatten_json(json_obj)
    console.print("[bold green]Flattened JSON:[/bold green]")
    print_json(flattened, minify=True)


def unflatten_data(json_obj: Dict[str, Any]):
    """
    Unflatten the JSON data and print it.

    Args:
        json_obj (dict): The flattened JSON object.

    Returns:
        None
    """
    unflattened = unflatten_json(json_obj)
    console.print("[bold green]Unflattened JSON:[/bold green]")
    print_json(unflattened)


def remove_key(json_obj: Dict[str, Any], key: str):
    """
    Remove a specific key from the JSON object.

    Args:
        json_obj (dict): The JSON object.
        key (str): The key to remove.

    Returns:
        None
    """
    try:
        del json_obj[key]
        logger.info(f"Removed key '{key}' from JSON.")
        console.print(
            f"[bold green]Removed key '{key}' from JSON.[/bold green]")
        print_json(json_obj)
    except KeyError:
        logger.error(f"Key '{key}' not found in JSON.")
        console.print(f"[bold red]Key '{key}' not found in JSON.[/bold red]")
        sys.exit(1)


def rename_key(json_obj: Dict[str, Any], old_key: str, new_key: str):
    """
    Rename a key in the JSON object.

    Args:
        json_obj (dict): The JSON object.
        old_key (str): The current key name.
        new_key (str): The new key name.

    Returns:
        None
    """
    try:
        json_obj[new_key] = json_obj.pop(old_key)
        logger.info(f"Renamed key '{old_key}' to '{new_key}'.")
        console.print(
            f"[bold green]Renamed key '{old_key}' to '{new_key}'.[/bold green]")
        print_json(json_obj)
    except KeyError:
        logger.error(f"Key '{old_key}' not found in JSON.")
        console.print(
            f"[bold red]Key '{old_key}' not found in JSON.[/bold red]")
        sys.exit(1)


def main():
    parser = argparse.ArgumentParser(
        description="Enhanced JSON processing tool."
    )
    parser.add_argument('file', type=str, help="JSON file to process")
    parser.add_argument('--minify', action='store_true',
                        help="Minify JSON output")
    parser.add_argument('--format', action='store_true',
                        help="Format JSON output with indentation")
    parser.add_argument('--yaml', type=str, metavar="OUTPUT_FILE",
                        help="Convert JSON to YAML and save to a file")
    parser.add_argument('--query', type=str, metavar="QUERY_PATH",
                        help="Query JSON data using simple dot notation (e.g., 'a.b.0.c')")
    parser.add_argument('--validate', action='store_true',
                        help="Validate JSON file format")
    parser.add_argument('--merge', type=str, nargs='+',
                        metavar="FILES", help="Merge multiple JSON files")
    parser.add_argument('--diff', type=str, nargs=2,
                        metavar=("FILE1", "FILE2"), help="Compare two JSON files")
    parser.add_argument('--export-xml', type=str, metavar="OUTPUT_FILE",
                        help="Export JSON to XML format")
    parser.add_argument('--stats', action='store_true',
                        help="Display statistics about the JSON structure")
    parser.add_argument('--flatten', action='store_true',
                        help="Flatten nested JSON structures")
    parser.add_argument('--unflatten', action='store_true',
                        help="Unflatten JSON structures")
    parser.add_argument('--remove-key', type=str, metavar="KEY",
                        help="Remove a specific key from the JSON data")
    parser.add_argument('--rename-key', type=str, nargs=2, metavar=("OLD_KEY", "NEW_KEY"),
                        help="Rename a key in the JSON data")
    parser.add_argument('--help', action='help',
                        help="Show help message and exit")

    args = parser.parse_args()

    # Load JSON data from the specified file.
    data = load_json(args.file)

    # Handle command-line arguments and execute corresponding functions.
    if args.yaml:
        save_json_to_yaml(data, args.yaml)
    elif args.export_xml:
        export_json_to_xml(data, args.export_xml)
    elif args.query:
        query_json(data, args.query)
    elif args.validate:
        validate_json(args.file)
    elif args.merge:
        merged_data = merge_json(args.merge)
        console.print("[bold green]Merged JSON Data:[/bold green]")
        print_json(merged_data)
    elif args.diff:
        diff_json(args.diff[0], args.diff[1])
    elif args.stats:
        display_stats(data)
    elif args.flatten:
        flatten_data(data)
    elif args.unflatten:
        unflatten_data(data)
    elif args.remove_key:
        remove_key(data, args.remove_key)
    elif args.rename_key:
        old_key, new_key = args.rename_key
        rename_key(data, old_key, new_key)
    else:
        if args.minify:
            console.print("[bold green]JSON Output (Minified):[/bold green]")
        else:
            console.print("[bold green]JSON Output (Formatted):[/bold green]")
        print_json(data, minify=args.minify, indent=None if args.minify else 4)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        logger.warning("Operation interrupted by user.")
        console.print(
            "\n[bold yellow]Operation interrupted by user.[/bold yellow]")
        sys.exit(0)
    except Exception as e:
        logger.exception(f"An unexpected error occurred: {e}")
        console.print(
            f"[bold red]An unexpected error occurred: {e}[/bold red]")
        sys.exit(1)
