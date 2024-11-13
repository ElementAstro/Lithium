"""
NM Tool for analyzing symbols in binary files.
Provides functionalities to retrieve, filter, search, count, and export symbols from a binary.
"""

import argparse
import subprocess
import json
import csv
from typing import List, Tuple, Optional, Dict
import sys
import os
from loguru import logger

# Configure loguru for logging
logger.remove()  # Remove the default logger to customize logging settings
logger.add(
    "nm_tool.log",
    rotation="10 MB",
    retention="10 days",
    compression="zip",
    enqueue=True,
    encoding="utf-8",
    format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level}</level> | <level>{message}</level>",
)
logger.add(sys.stdout, level="INFO", format="<level>{message}</level>")


class NMError(Exception):
    """Custom exception class for NM tool errors."""


class NM:
    """
    NM class encapsulates the functionality to interact with the 'nm' tool for binary analysis.

    Attributes:
        binary_path (str): The file path to the binary to be analyzed.
    """

    def __init__(self, binary_path: str):
        """
        Initializes the NM tool with the specified binary path and validates the binary.

        Args:
            binary_path (str): Path to the binary file.

        Raises:
            NMError: If the binary does not exist or is not executable.
        """
        self.binary_path = binary_path
        self._validate_binary()

    def _validate_binary(self) -> None:
        """
        Validates that the binary file exists and is executable.

        Raises:
            NMError: If the binary file does not exist or is not executable.
        """
        logger.debug(f"Validating binary path: {self.binary_path}")
        if not os.path.isfile(self.binary_path):
            logger.error(f"Binary file does not exist: {self.binary_path}")
            raise NMError(f"Binary file does not exist: {self.binary_path}")
        if not os.access(self.binary_path, os.X_OK):
            logger.warning(
                f"Binary file is not executable: {self.binary_path}")

    def _run_nm(self) -> str:
        """
        Executes the 'nm' command on the binary and captures its output.

        Returns:
            str: The standard output from the 'nm' command.

        Raises:
            NMError: If the 'nm' command fails or is not found.
        """
        logger.debug(f"Running nm on binary: {self.binary_path}")
        try:
            result = subprocess.run(
                ['nm', self.binary_path],
                capture_output=True,
                text=True,
                check=True
            )
            logger.debug("nm command executed successfully.")
            return result.stdout
        except subprocess.CalledProcessError as e:
            logger.error(f"Error running nm: {e}")
            raise NMError(f"Error running nm: {e}") from e
        except FileNotFoundError as exc:
            logger.error(
                "nm command not found. Please ensure it is installed and in PATH.")
            raise NMError(
                "nm command not found. Please ensure it is installed and in PATH.") from exc

    def get_symbols(self) -> List[Tuple[str, str, str]]:
        """
        Retrieves all symbols from the binary along with their addresses and types.

        Returns:
            List[Tuple[str, str, str]]: A list of tuples containing address, symbol type, and symbol name.
        """
        output = self._run_nm()
        symbols = []

        # Parse nm output
        for line in output.splitlines():
            if line.strip():  # Ignore empty lines
                parts = line.split()
                if len(parts) >= 3:
                    address, symbol_type, symbol_name = parts[0], parts[1], ' '.join(
                        parts[2:])
                    symbols.append((address, symbol_type, symbol_name))
                else:
                    logger.warning(f"Unparsed line: {line}")

        logger.info(f"Total symbols retrieved: {len(symbols)}")
        return symbols

    def filter_symbols(self, symbol_type: Optional[str] = None) -> List[Tuple[str, str, str]]:
        """
        Filters the retrieved symbols by their type.

        Args:
            symbol_type (Optional[str]): The type of symbol to filter by (e.g., T, D, B).

        Returns:
            List[Tuple[str, str, str]]: A list of filtered symbols.
        """
        symbols = self.get_symbols()
        if symbol_type:
            filtered = [s for s in symbols if s[1] == symbol_type]
            logger.info(
                f"Symbols filtered by type '{symbol_type}': {len(filtered)} found")
            return filtered
        logger.info("No symbol type filter applied.")
        return symbols

    def find_symbol(self, symbol_name: str) -> Optional[Tuple[str, str, str]]:
        """
        Searches for a specific symbol by its name.

        Args:
            symbol_name (str): The name of the symbol to search for.

        Returns:
            Optional[Tuple[str, str, str]]: The symbol tuple if found, else None.
        """
        logger.debug(f"Searching for symbol by name: {symbol_name}")
        symbols = self.get_symbols()
        for symbol in symbols:
            if symbol_name == symbol[2]:
                logger.info(f"Symbol '{symbol_name}' found: {symbol}")
                return symbol
        logger.info(f"Symbol '{symbol_name}' not found.")
        return None

    def find_symbol_by_address(self, address: str) -> Optional[Tuple[str, str, str]]:
        """
        Searches for a symbol by its address.

        Args:
            address (str): The address of the symbol to search for.

        Returns:
            Optional[Tuple[str, str, str]]: The symbol tuple if found, else None.
        """
        logger.debug(f"Searching for symbol by address: {address}")
        symbols = self.get_symbols()
        for addr, sym_type, name in symbols:
            if addr.lower() == address.lower():
                logger.info(
                    f"Symbol at address '{address}' found: {(addr, sym_type, name)}")
                return (addr, sym_type, name)
        logger.info(f"No symbol found at address '{address}'.")
        return None

    def display_symbols(self, symbols: List[Tuple[str, str, str]], detailed: bool = False) -> None:
        """
        Formats and displays the list of symbols.

        Args:
            symbols (List[Tuple[str, str, str]]): The list of symbols to display.
            detailed (bool): Whether to display detailed information.
        """
        if not symbols:
            logger.info("No symbols to display.")
            print("No symbols to display.")
            return

        for addr, sym_type, name in symbols:
            if detailed:
                print(f"Address: {addr}, Type: {sym_type}, Name: {name}")
            else:
                print(f"{addr}: {sym_type} {name}")
        logger.info(f"Displayed {len(symbols)} symbols.")

    def count_symbols_by_type(self) -> Dict[str, int]:
        """
        Counts the number of symbols by their type.

        Returns:
            Dict[str, int]: A dictionary with symbol types as keys and their counts as values.
        """
        symbols = self.get_symbols()
        type_count: Dict[str, int] = {}
        for _, sym_type, _ in symbols:
            type_count[sym_type] = type_count.get(sym_type, 0) + 1
        logger.info("Symbol counts by type calculated.")
        return type_count

    def export_symbols(self, filename: str, export_format: str = 'txt') -> None:
        """
        Exports the symbols to a file in the specified format.

        Args:
            filename (str): The name of the file to export symbols to.
            export_format (str): The format to export symbols in ('txt', 'csv', 'json', 'xml').

        Raises:
            NMError: If the export format is unsupported or if file operations fail.
        """
        symbols = self.get_symbols()
        logger.debug(
            f"Exporting symbols to {filename} in {export_format.upper()} format.")
        try:
            if export_format == 'txt':
                with open(filename, 'w', encoding='utf-8') as f:
                    for addr, sym_type, name in symbols:
                        f.write(f"{addr}: {sym_type} {name}\n")
                logger.info(f"Symbols exported to {filename} in TXT format.")
            elif export_format == 'csv':
                with open(filename, 'w', newline='', encoding='utf-8') as f:
                    writer = csv.writer(f)
                    writer.writerow(["Address", "Type", "Name"])
                    writer.writerows(symbols)
                logger.info(f"Symbols exported to {filename} in CSV format.")
            elif export_format == 'json':
                with open(filename, 'w', encoding='utf-8') as f:
                    json.dump(
                        [{"address": addr, "type": sym_type, "name": name}
                            for addr, sym_type, name in symbols],
                        f,
                        indent=4
                    )
                logger.info(f"Symbols exported to {filename} in JSON format.")
            elif export_format == 'xml':
                import xml.etree.ElementTree as ET

                root = ET.Element("Symbols")
                for addr, sym_type, name in symbols:
                    symbol = ET.SubElement(root, "Symbol")
                    ET.SubElement(symbol, "Address").text = addr
                    ET.SubElement(symbol, "Type").text = sym_type
                    ET.SubElement(symbol, "Name").text = name
                tree = ET.ElementTree(root)
                tree.write(filename, encoding='utf-8', xml_declaration=True)
                logger.info(f"Symbols exported to {filename} in XML format.")
            else:
                logger.error(f"Unsupported export format: {export_format}")
                raise NMError(f"Unsupported format: {export_format}")
        except IOError as e:
            logger.error(f"Failed to export symbols: {e}")
            raise NMError(f"Failed to export symbols: {e}") from e
        except ImportError as exc:
            logger.error(
                "XML export requires the xml module, which is not available.")
            raise NMError(
                "XML export requires the xml module, which is not available.") from exc


def main():
    """
    Main function to parse command-line arguments and execute NM tool actions.
    Supports filtering, searching, counting, and exporting symbols.
    """
    parser = argparse.ArgumentParser(
        description='NM tool wrapper for analyzing symbols in binaries.'
    )
    parser.add_argument('binary', type=str, help='Path to the binary file.')
    parser.add_argument('-f', '--filter', type=str,
                        help='Filter symbols by type (e.g., T, D, B).')
    parser.add_argument('-s', '--search', type=str,
                        help='Search for a specific symbol by name.')
    parser.add_argument('-a', '--address', type=str,
                        help='Find symbol by address.')
    parser.add_argument('-d', '--detailed', action='store_true',
                        help='Display detailed output for symbols.')
    parser.add_argument('-c', '--count', action='store_true',
                        help='Count symbols by type.')
    parser.add_argument('-e', '--export', type=str,
                        help='Export symbols to a file (supports txt, csv, json, xml).')
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='Increase output verbosity.')

    args = parser.parse_args()

    # Adjust loguru logging level based on verbosity
    if args.verbose:
        logger.remove()  # Remove the default stdout logger
        logger.add(sys.stdout, level="DEBUG",
                   format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level}</level> | <level>{message}</level>")

    try:
        nm_tool = NM(args.binary)
    except NMError as e:
        logger.error(e)
        sys.exit(1)

    try:
        actions_performed = False

        if args.filter:
            actions_performed = True
            logger.info(f"Filtering symbols by type: {args.filter}")
            filtered_symbols = nm_tool.filter_symbols(args.filter)
            nm_tool.display_symbols(filtered_symbols, args.detailed)

        if args.search:
            actions_performed = True
            logger.info(f"Searching for symbol: {args.search}")
            found_symbol = nm_tool.find_symbol(args.search)
            if found_symbol:
                nm_tool.display_symbols([found_symbol], args.detailed)
            else:
                print(f"Symbol '{args.search}' not found.")

        if args.address:
            actions_performed = True
            logger.info(f"Searching for symbol at address: {args.address}")
            found_symbol = nm_tool.find_symbol_by_address(args.address)
            if found_symbol:
                nm_tool.display_symbols([found_symbol], args.detailed)
            else:
                print(f"No symbol found at address '{args.address}'.")

        if args.count:
            actions_performed = True
            logger.info("Counting symbols by type.")
            counts = nm_tool.count_symbols_by_type()
            print("\nSymbol counts by type:")
            for sym_type, count in counts.items():
                print(f"{sym_type}: {count}")

        if args.export:
            actions_performed = True
            export_format = args.export.split('.')[-1].lower()
            logger.info(
                f"Exporting symbols to {args.export} in {export_format.upper()} format.")
            nm_tool.export_symbols(args.export, export_format)
            print(
                f"Symbols exported to {args.export} in {export_format.upper()} format.")

        # Default behavior: display all symbols if no specific action is taken
        if not actions_performed:
            logger.info("Displaying all symbols.")
            print(f"\nAll symbols in {args.binary}:")
            nm_tool.display_symbols(nm_tool.get_symbols(), args.detailed)

    except NMError as e:
        logger.error(e)
        sys.exit(1)
    except subprocess.CalledProcessError as e:
        logger.exception(f"A subprocess error occurred: {e}")
        sys.exit(1)
    except Exception as e:
        logger.exception(f"An unexpected error occurred: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
