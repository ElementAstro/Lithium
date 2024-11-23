import argparse
import json
import yaml
import toml
import xml.etree.ElementTree as ET
import csv
import configparser
from loguru import logger
from rich.console import Console
from rich.syntax import Syntax
from pathlib import Path
from typing import Optional, List, Union
import sys

# Initialize Rich Console
console = Console()

# Configure Loguru Logger
logger.add("ra.log", format="{time} {level} {message}",
           level="INFO", rotation="10 MB")


def pretty_print_json(data: str, indent: Optional[int] = 4) -> None:
    """
    Format and print JSON data.

    Example:
        >>> pretty_print_json('{"key": "value"}')
    """
    try:
        parsed_data = json.loads(data)
        pretty_json = json.dumps(
            parsed_data, indent=indent, ensure_ascii=False)
        syntax = Syntax(pretty_json, "json",
                        theme="monokai", line_numbers=True)
        console.print(syntax)
    except json.JSONDecodeError as e:
        console.print(f"[red]JSON Decode Error: {e}[/red]")
        logger.error(f"JSON Decode Error: {e}")


def pretty_print_yaml(data: str) -> None:
    """
    Format and print YAML data.

    Example:
        >>> pretty_print_yaml('key: value')
    """
    try:
        parsed_data = yaml.safe_load(data)
        pretty_yaml = yaml.dump(
            parsed_data, sort_keys=False, allow_unicode=True)
        syntax = Syntax(pretty_yaml, "yaml",
                        theme="monokai", line_numbers=True)
        console.print(syntax)
    except yaml.YAMLError as e:
        console.print(f"[red]YAML Parse Error: {e}[/red]")
        logger.error(f"YAML Parse Error: {e}")


def pretty_print_toml(data: str) -> None:
    """
    Format and print TOML data.

    Example:
        >>> pretty_print_toml('key = "value"')
    """
    try:
        parsed_data = toml.loads(data)
        pretty_toml = toml.dumps(parsed_data)
        syntax = Syntax(pretty_toml, "toml",
                        theme="monokai", line_numbers=True)
        console.print(syntax)
    except toml.TomlDecodeError as e:
        console.print(f"[red]TOML Decode Error: {e}[/red]")
        logger.error(f"TOML Decode Error: {e}")


def pretty_print_xml(data: str) -> None:
    """
    Format and print XML data.

    Example:
        >>> pretty_print_xml('<root><key>value</key></root>')
    """
    try:
        parsed_data = ET.fromstring(data)
        pretty_xml = ET.tostring(parsed_data, encoding='unicode', method='xml')
        syntax = Syntax(pretty_xml, "xml", theme="monokai", line_numbers=True)
        console.print(syntax)
    except ET.ParseError as e:
        console.print(f"[red]XML Parse Error: {e}[/red]")
        logger.error(f"XML Parse Error: {e}")


def pretty_print_csv(data: str) -> None:
    """
    Format and print CSV data.

    Example:
        >>> pretty_print_csv('key,value\\nfoo,bar')
    """
    try:
        reader = csv.reader(data.splitlines())
        rows = list(reader)
        if rows:
            max_lengths = [max(len(str(item)) for item in column)
                           for column in zip(*rows)]
            table = ""
            for row in rows:
                line = " | ".join(
                    f"{item:<{max_lengths[i]}}" for i, item in enumerate(row))
                table += line + "\n"
            console.print(table)
        else:
            console.print("[yellow]CSV file is empty.[/yellow]")
    except Exception as e:
        console.print(f"[red]CSV Parse Error: {e}[/red]")
        logger.error(f"CSV Parse Error: {e}")


def pretty_print_ini(data: str) -> None:
    """
    Format and print INI data.

    Example:
        >>> pretty_print_ini('[section]\\nkey=value')
    """
    try:
        config = configparser.ConfigParser()
        config.read_string(data)
        pretty_ini = ""
        for section in config.sections():
            pretty_ini += f"[{section}]\n"
            for key, value in config.items(section):
                pretty_ini += f"{key} = {value}\n"
            pretty_ini += "\n"
        syntax = Syntax(pretty_ini, "ini", theme="monokai", line_numbers=True)
        console.print(syntax)
    except configparser.Error as e:
        console.print(f"[red]INI Parse Error: {e}[/red]")
        logger.error(f"INI Parse Error: {e}")


def validate_file(file_path: Union[Path, str], file_format: str) -> bool:
    """
    Validate the syntax of the file without printing.

    Example:
        >>> validate_file(Path('example.json'), 'json')
    """
    try:
        content = file_path.read_text(
            encoding='utf-8') if isinstance(file_path, Path) else file_path
        if file_format == "json":
            json.loads(content)
        elif file_format == "yaml":
            yaml.safe_load(content)
        elif file_format == "toml":
            toml.loads(content)
        elif file_format == "xml":
            ET.fromstring(content)
        elif file_format == "csv":
            csv.reader(content.splitlines())
        elif file_format == "ini":
            config = configparser.ConfigParser()
            config.read_string(content)
        else:
            console.print(
                f"[red]Unsupported file format for validation: {file_format}[/red]")
            logger.error(
                f"Unsupported file format for validation: {file_format}")
            return False
        console.print(f"[green]{file_format.upper()} file is valid.[/green]")
        logger.info(f"{file_format.upper()} file is valid.")
        return True
    except Exception as e:
        console.print(
            f"[red]{file_format.upper()} Validation Error: {e}[/red]")
        logger.error(f"{file_format.upper()} Validation Error: {e}")
        return False


def display_file(file_path: Path, file_format: str, output: Optional[Path] = None, indent: Optional[int] = 4, validate: bool = False) -> None:
    """
    Display or validate file content based on format.

    Example:
        >>> display_file(Path('example.json'), 'json')
    """
    try:
        if isinstance(file_path, Path):
            content = file_path.read_text(encoding='utf-8')
        else:
            content = file_path  # For stdin input

        if validate:
            is_valid = validate_file(file_path, file_format)
            if not is_valid:
                return
            else:
                return

        if file_format == "json":
            result = json.dumps(json.loads(content),
                                indent=indent, ensure_ascii=False)
        elif file_format == "yaml":
            result = yaml.dump(yaml.safe_load(content),
                               sort_keys=False, allow_unicode=True)
        elif file_format == "toml":
            result = toml.dumps(toml.loads(content))
        elif file_format == "xml":
            parsed = ET.ElementTree(ET.fromstring(content))
            result = ET.tostring(
                parsed.getroot(), encoding='unicode', method='xml')
        elif file_format == "csv":
            reader = csv.reader(content.splitlines())
            rows = list(reader)
            if rows:
                max_lengths = [max(len(str(item)) for item in column)
                               for column in zip(*rows)]
                formatted_rows = [
                    " | ".join(
                        f"{item:<{max_lengths[i]}}" for i, item in enumerate(row))
                    for row in rows
                ]
                result = "\n".join(formatted_rows)
            else:
                result = "CSV file is empty."
        elif file_format == "ini":
            config = configparser.ConfigParser()
            config.read_string(content)
            result = ""
            for section in config.sections():
                result += f"[{section}]\n"
                for key, value in config.items(section):
                    result += f"{key} = {value}\n"
                result += "\n"
        else:
            console.print("[red]Unsupported file format![/red]")
            logger.error("Unsupported file format!")
            return

        if output:
            if output.exists():
                console.print(
                    f"[red]Output file {output} already exists. Use --overwrite to overwrite.[/red]")
                logger.error(f"Output file {output} already exists.")
                return
            output.write_text(result, encoding='utf-8')
            console.print(
                f"[green]Content has been written to {output}[/green]")
            logger.info(f"Content has been written to {output}")
        else:
            syntax = Syntax(result, file_format,
                            theme="monokai", line_numbers=True)
            console.print(syntax)
    except FileNotFoundError:
        console.print(f"[red]File not found: {file_path}[/red]")
        logger.error(f"File not found: {file_path}")
    except Exception as e:
        console.print(f"[red]An error occurred: {e}[/red]")
        logger.error(f"An error occurred: {e}")


def read_stdin() -> str:
    """
    Read data from standard input.

    Example:
        >>> echo '{"key": "value"}' | ra.py -
    """
    return sys.stdin.read()


def main() -> None:
    """
    Main function to parse arguments and display or validate file content.
    """
    parser = argparse.ArgumentParser(
        description="A CLI tool to beautifully display or validate JSON, YAML, TOML, XML, CSV, and INI file contents",
        formatter_class=argparse.RawTextHelpFormatter,
        epilog="""
Examples:
    Pretty print a JSON file:
        ra.py example.json

    Validate a YAML file:
        ra.py example.yaml --validate

    Pretty print multiple files with overwrite:
        ra.py example1.json example2.yaml --overwrite

    Read from standard input:
        cat example.toml | ra.py - --format toml
"""
    )
    parser.add_argument(
        "files",
        nargs="+",
        type=Union[Path, str],
        help="Path to the file(s) or '-' to read from standard input. "
             "e.g., example.json, example.yaml, example.toml, example.xml, example.csv, example.ini",
    )
    parser.add_argument(
        "--format",
        choices=["json", "yaml", "toml", "xml", "csv", "ini"],
        help="File format. Specify if the file extension does not match the format.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        help="Write the output content to the specified file(s). "
             "If multiple files are provided, use a pattern like 'output{}.txt' where {} will be replaced by the file index.",
    )
    parser.add_argument(
        "--overwrite",
        action="store_true",
        help="Overwrite the output file(s) if they exist.",
    )
    parser.add_argument(
        "--theme",
        type=str,
        default="monokai",
        help="Code highlighting theme.",
    )
    parser.add_argument(
        "--indent",
        type=int,
        default=4,
        help="Indentation level for pretty-printing.",
    )
    parser.add_argument(
        "--validate",
        action="store_true",
        help="Validate the file's syntax without printing.",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Enable verbose logging.",
    )

    args = parser.parse_args()

    # Set logging level based on verbosity
    if args.verbose:
        logger.remove()
        logger.add(
            "ra.log", format="{time} {level} {message}", level="DEBUG", rotation="10 MB")
        logger.debug("Verbose mode enabled.")

    for index, file in enumerate(args.files, start=1):
        if file == '-':
            logger.debug("Reading from standard input.")
            data = read_stdin()
            file_path = '-'  # Indicate stdin
            file_format = args.format or 'json'  # Default to JSON if not specified
        else:
            file_path = Path(file)
            suffix_map = {
                ".json": "json",
                ".yaml": "yaml",
                ".yml": "yaml",
                ".toml": "toml",
                ".xml": "xml",
                ".csv": "csv",
                ".ini": "ini",
            }
            file_format = args.format or suffix_map.get(
                file_path.suffix.lower())

        if not file_format:
            console.print(
                f"[red]Unable to auto-detect file format for {file}. Please use --format to specify the format.[/red]")
            logger.error(f"Unable to auto-detect file format for {file}.")
            continue

        output = None
        if args.output:
            if len(args.files) > 1:
                output = Path(args.output.name.format(
                    index)) if '{}' in args.output.name else args.output.parent / f"{args.output.stem}{index}{args.output.suffix}"
            else:
                output = args.output

        display_file(
            file_path=file_path,
            file_format=file_format,
            output=output,
            indent=args.indent,
            validate=args.validate
        )


if __name__ == "__main__":
    main()
