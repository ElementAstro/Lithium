# compiler_parser.py
"""
This module contains classes and functions for parsing compiler output and converting it to JSON, CSV, or XML format.
"""

import argparse
import json
import csv
import sys
from pathlib import Path
import re
import xml.etree.ElementTree as ET
from concurrent.futures import ThreadPoolExecutor, as_completed
from typing import List, Dict, Any
from termcolor import colored
from loguru import logger


class CompilerOutputParser:
    """
    Base class for compiler output parsers.
    """

    def __init__(self, compiler: str):
        self.compiler = compiler.lower()
        self.logger = logger

    def parse(self, output: str) -> Dict[str, Any]:
        """
        Parses the compiler output.

        Args:
            output (str): The raw output from the compiler.

        Returns:
            dict: Parsed output containing the compiler version and categorized results.
        """
        raise NotImplementedError(
            "Parse method must be implemented by subclasses.")


class GCCClangParser(CompilerOutputParser):
    """
    Parser for GCC and Clang compiler outputs.
    """

    def __init__(self, compiler: str):
        super().__init__(compiler)
        self.version_pattern = re.compile(
            r'(gcc|clang) version (\d+\.\d+\.\d+)')
        self.error_pattern = re.compile(
            r'(?P<file>.*?):(?P<line>\d+):(?P<column>\d+):\s*(?P<type>\w+):\s*(?P<message>.+)'
        )

    def parse(self, output: str) -> Dict[str, Any]:
        self.logger.debug("Parsing GCC/Clang output.")
        version_match = self.version_pattern.search(output)
        matches = self.error_pattern.findall(output)

        results = {"errors": [], "warnings": [], "info": []}
        for match in matches:
            entry = {
                "file": match[0].strip(),
                "line": int(match[1]),
                "column": int(match[2]),
                "message": match[4].strip(),
                "severity": match[3].lower(),
            }
            if entry["severity"] == 'error':
                results["errors"].append(entry)
            elif entry["severity"] == 'warning':
                results["warnings"].append(entry)
            else:
                results["info"].append(entry)

        parsed = {
            "version": version_match.group() if version_match else "unknown",
            "results": results
        }
        self.logger.debug(f"Parsed data: {parsed}")
        return parsed


class MSVCParser(CompilerOutputParser):
    """
    Parser for MSVC compiler outputs.
    """

    def __init__(self, compiler: str):
        super().__init__(compiler)
        self.version_pattern = re.compile(
            r'Compiler Version (\d+\.\d+\.\d+\.\d+)')
        self.error_pattern = re.compile(
            r'(?P<file>.*)\((?P<line>\d+)\):\s*(?P<type>\w+)\s*(?P<code>\w+\d+):\s*(?P<message>.+)'
        )

    def parse(self, output: str) -> Dict[str, Any]:
        self.logger.debug("Parsing MSVC output.")
        version_match = self.version_pattern.search(output)
        matches = self.error_pattern.findall(output)

        results = {"errors": [], "warnings": [], "info": []}
        for match in matches:
            entry = {
                "file": match[0].strip(),
                "line": int(match[1]),
                "code": match[3].strip(),
                "message": match[4].strip(),
                "severity": match[2].lower(),
            }
            if entry["severity"] == 'error':
                results["errors"].append(entry)
            elif entry["severity"] == 'warning':
                results["warnings"].append(entry)
            else:
                results["info"].append(entry)

        parsed = {
            "version": version_match.group() if version_match else "unknown",
            "results": results
        }
        self.logger.debug(f"Parsed data: {parsed}")
        return parsed


class CMakeParser(CompilerOutputParser):
    """
    Parser for CMake compiler outputs.
    """

    def __init__(self, compiler: str):
        super().__init__(compiler)
        self.version_pattern = re.compile(r'cmake version (\d+\.\d+\.\d+)')
        self.error_pattern = re.compile(
            r'(?P<file>.*?):(?P<line>\d+):(?P<type>\w+):\s*(?P<message>.+)'
        )

    def parse(self, output: str) -> Dict[str, Any]:
        self.logger.debug("Parsing CMake output.")
        version_match = self.version_pattern.search(output)
        matches = self.error_pattern.findall(output)

        results = {"errors": [], "warnings": [], "info": []}
        for match in matches:
            entry = {
                "file": match[0].strip(),
                "line": int(match[1]),
                "message": match[3].strip(),
                "severity": match[2].lower(),
            }
            if entry["severity"] == 'error':
                results["errors"].append(entry)
            elif entry["severity"] == 'warning':
                results["warnings"].append(entry)
            else:
                results["info"].append(entry)

        parsed = {
            "version": version_match.group() if version_match else "unknown",
            "results": results
        }
        self.logger.debug(f"Parsed data: {parsed}")
        return parsed


class ParserFactory:
    """
    Factory class to get the appropriate parser based on compiler type.
    """

    @staticmethod
    def get_parser(compiler: str) -> CompilerOutputParser:
        comp = compiler.lower()
        if comp in ['gcc', 'clang']:
            return GCCClangParser(compiler)
        elif comp == 'msvc':
            return MSVCParser(compiler)
        elif comp == 'cmake':
            return CMakeParser(compiler)
        else:
            logger.error(f"Unsupported compiler: {compiler}")
            raise ValueError(f"Unsupported compiler: {compiler}")


def write_to_csv(data: List[Dict[str, Any]], output_path: Path) -> None:
    """
    Writes parsed data to a CSV file.

    Args:
        data (list): The parsed data to write.
        output_path (Path): The path to the output CSV file.
    """
    logger.debug(f"Writing data to CSV at {output_path}.")
    fieldnames = ['file', 'line', 'column',
                  'type', 'code', 'message', 'severity']
    try:
        with open(output_path, 'w', newline='', encoding="utf-8") as csvfile:
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()
            for entry in data:
                writer.writerow(entry)
        logger.info(f"CSV output saved to {output_path}.")
    except Exception as e:
        logger.error(f"Failed to write CSV: {e}")
        raise


def write_to_xml(data: List[Dict[str, Any]], output_path: Path) -> None:
    """
    Writes parsed data to an XML file.

    Args:
        data (list): The parsed data to write.
        output_path (Path): The path to the output XML file.
    """
    logger.debug(f"Writing data to XML at {output_path}.")
    root = ET.Element("CompilerOutput")
    for entry in data:
        item = ET.SubElement(root, "Item")
        for key, value in entry.items():
            child = ET.SubElement(item, key)
            child.text = str(value)
    try:
        tree = ET.ElementTree(root)
        tree.write(output_path, encoding="utf-8", xml_declaration=True)
        logger.info(f"XML output saved to {output_path}.")
    except Exception as e:
        logger.error(f"Failed to write XML: {e}")
        raise


def colorize_output(entries: List[Dict[str, Any]]) -> None:
    """
    Prints compiler results with colorized output in the console.

    Args:
        entries (list): A list of parsed compiler entries.
    """
    for entry in entries:
        message = f"{entry['type'].capitalize()} in {entry['file']}:{entry['line']}"
        full_message = f"{message} - {entry['message']}"
        if entry['severity'] == 'error':
            print(colored(full_message, 'red'))
        elif entry['severity'] == 'warning':
            print(colored(full_message, 'yellow'))
        else:
            print(colored(full_message, 'blue'))


class CompilerParser:
    """
    Main class to handle the parsing of compiler outputs and conversion to desired formats.
    """

    def __init__(self, args: argparse.Namespace):
        self.compiler = args.compiler
        self.file_paths = [Path(fp).resolve() for fp in args.file_paths]
        self.output_format = args.output_format.lower()
        self.output_file = Path(args.output_dir).resolve() / args.output_file
        self.filter = [f.lower() for f in args.filter] if args.filter else []
        self.stats = args.stats
        self.concurrency = args.concurrency
        self.logger = logger

    def process_file(self, file_path: Path) -> Dict[str, Any]:
        """
        Processes a single file to parse the compiler output.

        Args:
            file_path (Path): The path to the file containing the compiler output.

        Returns:
            dict: Parsed output containing the file path, compiler version, and categorized results.
        """
        self.logger.debug(f"Processing file: {file_path}")
        try:
            with open(file_path, 'r', encoding="utf-8") as file:
                output = file.read()
            parser = ParserFactory.get_parser(self.compiler)
            parsed_output = parser.parse(output)
            result = {
                "file": str(file_path),
                "version": parsed_output["version"],
                "results": parsed_output["results"]
            }
            self.logger.debug(f"Parsed result for {file_path}: {result}")
            return result
        except Exception as e:
            self.logger.error(f"Error processing {file_path}: {e}")
            raise

    def aggregate_results(self, results: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
        """
        Aggregates and flattens the parsed results.

        Args:
            results (list): List of parsed results from multiple files.

        Returns:
            list: Flattened list of all compiler entries.
        """
        self.logger.debug("Aggregating results.")
        flattened = []
        for result in results:
            for severity, entries in result['results'].items():
                for entry in entries:
                    entry_copy = entry.copy()
                    entry_copy['type'] = severity
                    entry_copy['file'] = result['file']
                    flattened.append(entry_copy)
        self.logger.debug(f"Total aggregated entries: {len(flattened)}")
        return flattened

    def apply_filters(self, data: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
        """
        Applies severity filters to the data.

        Args:
            data (list): The list of compiler entries.

        Returns:
            list: Filtered list based on severity.
        """
        if not self.filter:
            self.logger.debug("No filters applied.")
            return data
        filtered = [entry for entry in data if entry['severity'] in self.filter]
        self.logger.debug(f"Entries after filtering: {len(filtered)}")
        return filtered

    def calculate_statistics(self, data: List[Dict[str, Any]]) -> Dict[str, int]:
        """
        Calculates statistics based on the data.

        Args:
            data (list): The list of compiler entries.

        Returns:
            dict: Statistics including total, errors, warnings, and info counts.
        """
        stats = {
            "total": len(data),
            "errors": sum(1 for entry in data if entry['severity'] == 'error'),
            "warnings": sum(1 for entry in data if entry['severity'] == 'warning'),
            "info": sum(1 for entry in data if entry['severity'] == 'info'),
        }
        self.logger.debug(f"Statistics calculated: {stats}")
        return stats

    def output_results(self, data: List[Dict[str, Any]]) -> None:
        """
        Outputs the results in the desired format.

        Args:
            data (list): The list of compiler entries.
        """
        self.logger.debug(
            f"Outputting results in {self.output_format} format.")
        try:
            if self.output_format == 'json':
                with open(self.output_file, 'w', encoding="utf-8") as json_file:
                    json.dump(data, json_file, indent=4)
                self.logger.info(f"JSON output saved to {self.output_file}.")
            elif self.output_format == 'csv':
                write_to_csv(data, self.output_file)
            elif self.output_format == 'xml':
                write_to_xml(data, self.output_file)
            else:
                self.logger.error(
                    f"Unsupported output format: {self.output_format}")
                raise ValueError(
                    f"Unsupported output format: {self.output_format}")
        except Exception as e:
            self.logger.error(f"Failed to write output file: {e}")
            raise

    def run(self) -> None:
        """
        Executes the parsing and output generation process.
        """
        self.logger.info("Starting compiler output parsing.")
        results = []
        with ThreadPoolExecutor(max_workers=self.concurrency) as executor:
            future_to_file = {executor.submit(
                self.process_file, fp): fp for fp in self.file_paths}
            for future in as_completed(future_to_file):
                file_path = future_to_file[future]
                try:
                    result = future.result()
                    results.append(result)
                except Exception as e:
                    self.logger.error(f"Failed to process {file_path}: {e}")

        flattened = self.aggregate_results(results)
        filtered = self.apply_filters(flattened)

        if self.stats:
            stats = self.calculate_statistics(filtered)
            print(f"Statistics:\n{json.dumps(stats, indent=4)}")

        self.output_results(filtered)
        colorize_output(filtered)
        self.logger.info("Compiler output parsing completed.")


def setup_logging_config() -> None:
    """
    Configures Loguru for logging.
    """
    logger.remove()
    logger.add(
        "compiler_parser.log",
        rotation="10 MB",
        retention="7 days",
        compression="zip",
        enqueue=True,
        encoding="utf-8",
        format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level}</level> | <level>{message}</level>",
        level="DEBUG"
    )
    logger.add(
        sys.stdout,
        level="INFO",
        format="<level>{message}</level>",
    )
    logger.debug("Logging is set up.")


def main():
    """
    Main function to parse compiler output and convert to JSON, CSV, or XML format.
    """
    setup_logging_config()
    parser = argparse.ArgumentParser(
        description="Parse compiler output and convert to JSON, CSV, or XML format."
    )
    parser.add_argument('compiler', choices=['gcc', 'clang', 'msvc', 'cmake'],
                        help="The compiler used for the output.")
    parser.add_argument('file_paths', nargs='+',
                        help="Paths to the compiler output files.")
    parser.add_argument(
        '--output-format', choices=['json', 'csv', 'xml'], default='json', help="Output format.")
    parser.add_argument(
        '--output-file', default='output.json', help="Output file name.")
    parser.add_argument(
        '--output-dir', default='.', help="Output directory.")
    parser.add_argument(
        '--filter', nargs='*', choices=['error', 'warning', 'info'], help="Filter by message types.")
    parser.add_argument('--stats', action='store_true',
                        help="Include statistics in the output.")
    parser.add_argument(
        '--concurrency', type=int, default=4, help="Number of concurrent threads for processing files.")

    args = parser.parse_args()

    try:
        parser_instance = CompilerParser(args)
        parser_instance.run()
    except Exception as e:
        logger.exception(f"An error occurred during processing: {e}")
        sys.exit(1)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        logger.warning("Operation interrupted by user.")
        sys.exit(0)
