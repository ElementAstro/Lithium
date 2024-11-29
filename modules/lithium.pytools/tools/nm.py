# python
"""
NM Tool for analyzing symbols in binary files.
Provides functionalities to retrieve, filter, search, count, and export symbols from a binary.
"""

import argparse
import subprocess
import json
import csv
import sys
import os
import re
from typing import List, Tuple, Optional, Dict

from loguru import logger
from rich.console import Console
from rich.table import Table
from rich.progress import track

console = Console()

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
logger.add(
    sys.stderr,
    level="INFO",
    format="<level>{message}</level>",
)


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

    def _run_nm(self, demangle: bool = False, extern_only: bool = False) -> str:
        """
        Executes the 'nm' command on the binary and captures its output.

        Args:
            demangle (bool): Whether to demangle C++ symbol names.
            extern_only (bool): Whether to display only external symbols.

        Returns:
            str: The standard output from the 'nm' command.

        Raises:
            NMError: If the 'nm' command fails or is not found.
        """
        logger.debug(f"Running nm on binary: {self.binary_path}")
        cmd = ['nm', self.binary_path]
        if demangle:
            cmd.append('-C')
        if extern_only:
            cmd.append('-g')
        logger.debug(f"Command: {' '.join(cmd)}")
        try:
            result = subprocess.run(
                cmd,
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

    def get_symbols(self, demangle: bool = False, extern_only: bool = False) -> List[Tuple[str, str, str]]:
        """
        Retrieves all symbols from the binary along with their addresses and types.

        Args:
            demangle (bool): Whether to demangle C++ symbol names.
            extern_only (bool): Whether to retrieve only external symbols.

        Returns:
            List[Tuple[str, str, str]]: A list of tuples containing address, symbol type, and symbol name.
        """
        output = self._run_nm(demangle=demangle, extern_only=extern_only)
        symbols = []

        # Parse nm output
        for line in output.splitlines():
            if line.strip():  # Ignore empty lines
                parts = line.strip().split(maxsplit=2)
                if len(parts) == 3:
                    address, symbol_type, symbol_name = parts
                elif len(parts) == 2:
                    address, symbol_type = parts
                    symbol_name = ''
                else:
                    logger.warning(f"Unparsed line: {line}")
                    continue
                symbols.append((address, symbol_type, symbol_name))

        logger.info(f"Total symbols retrieved: {len(symbols)}")
        return symbols

    def filter_symbols(self, symbol_type: Optional[str] = None, regex: Optional[str] = None) -> List[Tuple[str, str, str]]:
        """
        Filters the retrieved symbols by their type or name pattern.

        Args:
            symbol_type (Optional[str]): The type of symbol to filter by (e.g., 'T', 'D', 'B').
            regex (Optional[str]): A regular expression to filter symbol names.

        Returns:
            List[Tuple[str, str, str]]: A list of filtered symbols.
        """
        symbols = self.get_symbols()
        if symbol_type:
            symbols = [s for s in symbols if s[1] == symbol_type]
            logger.info(
                f"Symbols filtered by type '{symbol_type}': {len(symbols)} found")
        if regex:
            pattern = re.compile(regex)
            symbols = [s for s in symbols if pattern.search(s[2])]
            logger.info(
                f"Symbols filtered by pattern '{regex}': {len(symbols)} found")
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
        Formats and displays the list of symbols using Rich for enhanced terminal output.

        Args:
            symbols (List[Tuple[str, str, str]]): The list of symbols to display.
            detailed (bool): Whether to display detailed information.
        """
        if not symbols:
            logger.info("No symbols to display.")
            console.print("[bold yellow]No symbols to display.[/bold yellow]")
            return

        table = Table(title="Symbol Table")

        table.add_column("Address", style="cyan")
        table.add_column("Type", style="magenta")
        table.add_column("Name", style="green")

        for addr, sym_type, name in symbols:
            table.add_row(addr, sym_type, name)

        console.print(table)
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
                        f.write(f"{addr} {sym_type} {name}\n")
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

    def get_symbol_sizes(self) -> List[Tuple[str, str, str, str]]:
        """
        Retrieves symbols along with their sizes.

        Returns:
            List[Tuple[str, str, str, str]]: A list of tuples containing address, size, symbol type, and symbol name.
        """
        logger.debug("Retrieving symbols with sizes.")
        cmd = ['nm', '-S', self.binary_path]
        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                check=True
            )
            symbols = []
            for line in result.stdout.splitlines():
                if line.strip():
                    parts = line.strip().split(maxsplit=3)
                    if len(parts) == 4:
                        address, size, symbol_type, symbol_name = parts
                    else:
                        logger.warning(f"Unparsed line: {line}")
                        continue
                    symbols.append((address, size, symbol_type, symbol_name))
            logger.info(f"Total symbols with sizes retrieved: {len(symbols)}")
            return symbols
        except subprocess.CalledProcessError as e:
            logger.error(f"Error running nm with -S: {e}")
            raise NMError(f"Error running nm with -S: {e}") from e

    def display_symbol_sizes(self, symbols: List[Tuple[str, str, str, str]]) -> None:
        """
        Displays symbols along with their sizes using Rich.

        Args:
            symbols (List[Tuple[str, str, str, str]]): The list of symbols with sizes.
        """
        if not symbols:
            logger.info("No symbols with sizes to display.")
            console.print(
                "[bold yellow]No symbols with sizes to display.[/bold yellow]")
            return

        table = Table(title="Symbol Sizes")

        table.add_column("Address", style="cyan")
        table.add_column("Size", style="blue")
        table.add_column("Type", style="magenta")
        table.add_column("Name", style="green")

        for addr, size, sym_type, name in symbols:
            table.add_row(addr, size, sym_type, name)

        console.print(table)
        logger.info(f"Displayed {len(symbols)} symbols with sizes.")

    def search_symbols(self, pattern: str) -> List[Tuple[str, str, str]]:
        """
        Searches for symbols matching a regular expression pattern.

        Args:
            pattern (str): The regular expression pattern to search for.

        Returns:
            List[Tuple[str, str, str]]: A list of matching symbols.
        """
        logger.debug(f"Searching symbols with pattern: {pattern}")
        symbols = self.get_symbols()
        regex = re.compile(pattern)
        matching_symbols = [s for s in symbols if regex.search(s[2])]
        logger.info(f"Found {len(matching_symbols)} symbols matching pattern.")
        return matching_symbols


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
    parser.add_argument('--demangle', action='store_true',
                        help='Demangle symbol names.')
    parser.add_argument('--extern', action='store_true',
                        help='Display only external symbols.')
    parser.add_argument('--size', action='store_true',
                        help='Display symbols with sizes.')
    parser.add_argument('--pattern', type=str,
                        help='Search symbols matching a regex pattern.')

    args = parser.parse_args()

    # Adjust loguru logging level based on verbosity
    if args.verbose:
        logger.remove()
        logger.add(sys.stderr, level="DEBUG",
                   format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level}</level> | <level>{message}</level>")

    try:
        nm_tool = NM(args.binary)
    except NMError as e:
        logger.error(e)
        sys.exit(1)

    try:
        actions_performed = False

        if args.size:
            actions_performed = True
            logger.info("Displaying symbols with sizes.")
            symbols_with_sizes = nm_tool.get_symbol_sizes()
            nm_tool.display_symbol_sizes(symbols_with_sizes)

        if args.filter or args.pattern:
            actions_performed = True
            logger.info(
                f"Filtering symbols by type: {args.filter} and pattern: {args.pattern}")
            filtered_symbols = nm_tool.filter_symbols(
                symbol_type=args.filter, regex=args.pattern)
            nm_tool.display_symbols(filtered_symbols, args.detailed)

        if args.search:
            actions_performed = True
            logger.info(f"Searching for symbol: {args.search}")
            found_symbol = nm_tool.find_symbol(args.search)
            if found_symbol:
                nm_tool.display_symbols([found_symbol], args.detailed)
            else:
                console.print(
                    f"[bold red]Symbol '{args.search}' not found.[/bold red]")

        if args.address:
            actions_performed = True
            logger.info(f"Searching for symbol at address: {args.address}")
            found_symbol = nm_tool.find_symbol_by_address(args.address)
            if found_symbol:
                nm_tool.display_symbols([found_symbol], args.detailed)
            else:
                console.print(
                    f"[bold red]No symbol found at address '{args.address}'.[/bold red]")

        if args.count:
            actions_performed = True
            logger.info("Counting symbols by type.")
            counts = nm_tool.count_symbols_by_type()
            console.print(
                "\n[bold underline]Symbol counts by type:[/bold underline]")
            for sym_type, count in counts.items():
                console.print(f"[green]{sym_type}[/green]: {count}")

        if args.export:
            actions_performed = True
            export_format = args.export.split('.')[-1].lower()
            logger.info(
                f"Exporting symbols to {args.export} in {export_format.upper()} format.")
            nm_tool.export_symbols(args.export, export_format)
            console.print(
                f"[bold green]Symbols exported to {args.export} in {export_format.upper()} format.[/bold green]")

        # Default behavior: display all symbols if no specific action is taken
        if not actions_performed:
            logger.info("Displaying all symbols.")
            console.print(
                f"\n[bold underline]All symbols in {args.binary}:[/bold underline]")
            nm_tool.display_symbols(
                nm_tool.get_symbols(demangle=args.demangle,
                                    extern_only=args.extern),
                args.detailed
            )

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
