"""
This module contains functions for parsing compiler output and converting it to JSON, CSV, or XML format.
"""
import re
import json
import csv
import argparse
import xml.etree.ElementTree as ET
from concurrent.futures import ThreadPoolExecutor


def parse_gcc_clang_output(output):
    """
    Parses GCC/Clang compiler output.

    Args:
        output (str): The raw output from the GCC/Clang compiler.

    Returns:
        dict: A dictionary containing the compiler version and categorized results.
    """
    version_pattern = re.compile(r'(gcc|clang) version (\d+\.\d+\.\d+)')
    error_pattern = re.compile(
        r'(?P<file>.*):(?P<line>\d+):(?P<column>\d+):\s*(?P<type>\w+):\s*(?P<message>.+)')

    version_match = version_pattern.search(output)
    matches = error_pattern.findall(output)

    results = {"errors": [], "warnings": [], "info": []}
    for match in matches:
        entry = {
            "file": match[0],
            "line": int(match[1]),
            "column": int(match[2]),
            "message": match[4].strip()
        }
        if match[3].lower() == 'error':
            results["errors"].append(entry)
        elif match[3].lower() == 'warning':
            results["warnings"].append(entry)
        else:
            results["info"].append(entry)

    return {
        "version": version_match.group() if version_match else "unknown",
        "results": results
    }


def parse_msvc_output(output):
    """
    Parses MSVC compiler output.

    Args:
        output (str): The raw output from the MSVC compiler.

    Returns:
        dict: A dictionary containing the compiler version and categorized results.
    """
    version_pattern = re.compile(r'Compiler Version (\d+\.\d+\.\d+\.\d+)')
    error_pattern = re.compile(
        r'(?P<file>.*)\((?P<line>\d+)\):\s*(?P<type>\w+)\s*(?P<code>\w+\d+):\s*(?P<message>.+)')

    version_match = version_pattern.search(output)
    matches = error_pattern.findall(output)

    results = {"errors": [], "warnings": [], "info": []}
    for match in matches:
        entry = {
            "file": match[0],
            "line": int(match[1]),
            "code": match[3],
            "message": match[4].strip()
        }
        if match[2].lower() == 'error':
            results["errors"].append(entry)
        elif match[2].lower() == 'warning':
            results["warnings"].append(entry)
        else:
            results["info"].append(entry)

    return {
        "version": version_match.group() if version_match else "unknown",
        "results": results
    }


def parse_cmake_output(output):
    """
    Parses CMake compiler output.

    Args:
        output (str): The raw output from the CMake build system.

    Returns:
        dict: A dictionary containing the CMake version and categorized results.
    """
    version_pattern = re.compile(r'cmake version (\d+\.\d+\.\d+)')
    error_pattern = re.compile(
        r'(?P<file>.*):(?P<line>\d+):(?P<type>\w+):\s*(?P<message>.+)')

    version_match = version_pattern.search(output)
    matches = error_pattern.findall(output)

    results = {"errors": [], "warnings": [], "info": []}
    for match in matches:
        entry = {
            "file": match[0],
            "line": int(match[1]),
            "message": match[3].strip()
        }
        if match[2].lower() == 'error':
            results["errors"].append(entry)
        elif match[2].lower() == 'warning':
            results["warnings"].append(entry)
        else:
            results["info"].append(entry)

    return {
        "version": version_match.group() if version_match else "unknown",
        "results": results
    }


def parse_output(compiler, output):
    """
    Parses the compiler output based on the specified compiler type.

    Args:
        compiler (str): The compiler type (gcc, clang, msvc, cmake).
        output (str): The raw output from the compiler.

    Returns:
        dict: Parsed output containing the compiler version and categorized results.
    """
    if compiler.lower() in ['gcc', 'clang']:
        return parse_gcc_clang_output(output)
    elif compiler.lower() == 'msvc':
        return parse_msvc_output(output)
    elif compiler.lower() == 'cmake':
        return parse_cmake_output(output)
    else:
        raise ValueError("Unsupported compiler")


def write_to_csv(data, output_path):
    """
    Writes parsed data to a CSV file.

    Args:
        data (list): The parsed data to write.
        output_path (str): The path to the output CSV file.
    """
    with open(output_path, 'w', newline='', encoding="utf-8") as csvfile:
        fieldnames = ['file', 'line', 'column', 'type', 'code', 'message']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        for entry in data:
            writer.writerow(entry)


def write_to_xml(data, output_path):
    """
    Writes parsed data to an XML file.

    Args:
        data (list): The parsed data to write.
        output_path (str): The path to the output XML file.
    """
    root = ET.Element("CompilerOutput")
    for entry in data:
        item = ET.SubElement(root, "Item")
        for key, value in entry.items():
            child = ET.SubElement(item, key)
            child.text = str(value)
    tree = ET.ElementTree(root)
    tree.write(output_path, encoding="utf-8", xml_declaration=True)


def process_file(compiler, file_path):
    """
    Processes a file to parse the compiler output.

    Args:
        compiler (str): The compiler type (gcc, clang, msvc, cmake).
        file_path (str): The path to the file containing the compiler output.

    Returns:
        dict: Parsed output containing the file path, compiler version, and categorized results.
    """
    with open(file_path, 'r', encoding="utf-8") as file:
        output = file.read()
    parsed_output = parse_output(compiler, output)
    return {
        "file": file_path,
        "version": parsed_output["version"],
        "results": parsed_output["results"]
    }


def main():
    """
    Main function to parse compiler output and convert to JSON, CSV, or XML format.
    """
    parser = argparse.ArgumentParser(
        description="Parse compiler output and convert to JSON, CSV, or XML format.")
    parser.add_argument('compiler', choices=[
                        'gcc', 'clang', 'msvc', 'cmake'], help="The compiler used for the output.")
    parser.add_argument('file_paths', nargs='+',
                        help="Paths to the compiler output files.")
    parser.add_argument(
        '--output-format', choices=['json', 'csv', 'xml'], default='json', help="Output format.")
    parser.add_argument(
        '--output-file', default='output.json', help="Output file name.")
    parser.add_argument(
        '--filter', choices=['error', 'warning', 'info'], help="Filter by message type.")
    parser.add_argument(
        '--stats', action='store_true', help="Include statistics in the output.")

    args = parser.parse_args()

    all_results = []
    with ThreadPoolExecutor() as executor:
        futures = [executor.submit(process_file, args.compiler, file_path)
                   for file_path in args.file_paths]
        for future in futures:
            all_results.append(future.result())

    flattened_results = []
    for result in all_results:
        for key, entries in result['results'].items():
            for entry in entries:
                entry['type'] = key
                entry['file'] = result['file']
                flattened_results.append(entry)

    if args.filter:
        flattened_results = [
            entry for entry in flattened_results if entry['type'] == args.filter]

    if args.stats:
        stats = {
            "errors": sum(1 for entry in flattened_results if entry['type'] == 'errors'),
            "warnings": sum(1 for entry in flattened_results if entry['type'] == 'warnings'),
            "info": sum(1 for entry in flattened_results if entry['type'] == 'info'),
        }
        print(f"Statistics: {json.dumps(stats, indent=4)}")

    if args.output_format == 'json':
        json_output = json.dumps(flattened_results, indent=4)
        with open(args.output_file, 'w', encoding="utf-8") as json_file:
            json_file.write(json_output)
        print(f"JSON output saved to {args.output_file}")
    elif args.output_format == 'csv':
        write_to_csv(flattened_results, args.output_file)
        print(f"CSV output saved to {args.output_file}")
    elif args.output_format == 'xml':
        write_to_xml(flattened_results, args.output_file)
        print(f"XML output saved to {args.output_file}")


if __name__ == "__main__":
    main()
