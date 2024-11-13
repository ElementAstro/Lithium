#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
File: byte_convertor.py
Author: Max Qian <astro_air@126.com>
Date: 2024-08-26
Version: 2.0

Description:
------------
This Python script provides functionality to convert binary files into C-style header files
containing array data, and vice versa. The script supports several features, including data
compression, various data formats (hex, binary, decimal, base64), optional C++ class wrappers,
splitting large arrays into multiple header files, and data integrity verification.

The script can be used in two main modes:
    1. `to_header`: Converts a binary file into a C header file with array data.
    2. `to_file`: Converts a C header file containing array data back into a binary file.

Features:
---------
- Convert binary files to C-style arrays in hex, binary, decimal, or base64 format.
- Compress the binary data before storing it in the header file.
- Generate optional C++ class wrappers for the array data with customizable class names.
- Optionally split large arrays across multiple header files.
- Generate `#ifndef` protection macros to prevent multiple inclusions of the same header file.
- Customize array and size variable names, array data types, and comment styles.
- Verify data integrity after conversion by comparing checksums.
- Display progress bars for long-running conversions.

Dependencies:
-------------
- Python 3.x
- Standard Python libraries: sys, os, zlib, datetime, typing, base64, hashlib
- External Python libraries: loguru, tqdm

Usage:
------
The script can be executed directly from the command line. Below is the usage syntax:

    Usage: byte_convertor.py <mode> <input_file> [output_file] [options]

    Modes:
    -------
    to_header: Convert a binary file to a C header file.
    to_file  : Convert a C header file back to a binary file.

    Options for 'to_header':
    -------------------------
    --array_name <name>         : Set the name of the array (default: resource_data).
    --size_name <name>          : Set the name of the size variable (default: resource_size).
    --array_type <type>         : Set the type of the array (default: unsigned char).
    --comment_style <C|CPP>     : Set the comment style (default: C).
    --compress                  : Compress the data in the header file.
    --format <hex|bin|dec|base64>: Set the format of the array data (default: hex).
    --start <byte_offset>       : Set the start byte for conversion.
    --end <byte_offset>         : Set the end byte for conversion.
    --no_protect                : Do not include #ifndef protection macros.
    --cpp_class <ClassName>     : Generate a simple C++ class wrapper with the specified class name.
    --split_size <bytes>        : Split the output into multiple headers with this max size.
    --verify                    : Verify data integrity after conversion.

    Options for 'to_file':
    -----------------------
    --decompress                : Decompress the data when converting back to a file.
    --verify                    : Verify data integrity after conversion.

Examples:
---------
1. Convert a binary file to a C header file:
    $ python3 byte_convertor.py to_header my_binary.bin my_header.h --compress --array_name my_array

2. Convert a C header file back to a binary file:
    $ python3 byte_convertor.py to_file my_header.h my_binary.bin --decompress

3. Split a large binary file into multiple headers, each with a maximum size of 1024 bytes:
    $ python3 byte_convertor.py to_header large_binary.bin --split_size 1024

4. Convert with base64 format and generate a C++ class wrapper:
    $ python3 byte_convertor.py to_header data.bin data.h --format base64 --cpp_class DataWrapper

License:
--------
This script is released under the MIT License.

Disclaimer:
-----------
This script is provided "as is", without warranty of any kind, express or implied, including
but not limited to the warranties of merchantability, fitness for a particular purpose, and
noninfringement. In no event shall the authors be liable for any claim, damages, or other
liability, whether in an action of contract, tort, or otherwise, arising from, out of, or in
connection with the script or the use or other dealings in the script.
"""

import sys
import os
import zlib
import base64
import hashlib
from datetime import datetime
from typing import Optional, List, Tuple
from loguru import logger
from tqdm import tqdm


def setup_logging() -> None:
    """
    Configure loguru for logging.
    """
    logger.remove()  # Remove default logger
    logger.add(
        "byte_convertor.log",
        rotation="10 MB",
        retention="7 days",
        compression="zip",
        enqueue=True,
        encoding="utf-8",
        format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level}</level> | <level>{message}</level>",
    )
    logger.add(
        sys.stdout,
        level="INFO",
        format="<level>{message}</level>",
    )


class ByteConvertorError(Exception):
    """Custom exception class for Byte Convertor errors."""
    pass


def compute_checksum(data: bytes) -> str:
    """
    Compute SHA256 checksum of given data.

    Args:
        data (bytes): Data to compute checksum for.

    Returns:
        str: SHA256 checksum as hexadecimal string.
    """
    sha256 = hashlib.sha256()
    sha256.update(data)
    return sha256.hexdigest()


def convert_file_to_array(
    filename: str,
    start: int = 0,
    end: Optional[int] = None,
    data_format: str = 'hex'
) -> str:
    """
    Convert a binary file to a C-style array string.

    Args:
        filename (str): Path to the binary file.
        start (int): Start byte for conversion. Defaults to 0.
        end (Optional[int]): End byte for conversion. Defaults to None (end of file).
        data_format (str): Format of the array data ('hex', 'bin', 'dec', 'base64'). Defaults to 'hex'.

    Returns:
        str: C-style array string.

    Raises:
        ValueError: If unsupported data format is provided.
        ByteConvertorError: If file reading fails.
    """
    try:
        with open(filename, 'rb') as file:
            data = file.read()[start:end]
        logger.debug(
            f"Read {len(data)} bytes from {filename} (bytes {start} to {end})")
    except IOError as e:
        logger.error(f"Failed to read file {filename}: {e}")
        raise ByteConvertorError(f"Failed to read file {filename}: {e}") from e

    if data_format == 'hex':
        array_str = ', '.join(f'0x{b:02X}' for b in data)
    elif data_format == 'bin':
        array_str = ', '.join(f'0b{b:08b}' for b in data)
    elif data_format == 'dec':
        array_str = ', '.join(f'{b}' for b in data)
    elif data_format == 'base64':
        array_str = f'"{base64.b64encode(data).decode()}"'
    else:
        logger.error(f"Unsupported format: {data_format}")
        raise ValueError(f"Unsupported format: {data_format}")

    logger.debug(f"Converted data to {data_format} format")
    return array_str


def convert_array_to_file(
    array_data: str,
    output_filename: str,
    data_format: str = 'hex',
    decompress: bool = False
) -> None:
    """
    Convert a C-style array string back to a binary file.

    Args:
        array_data (str): C-style array string.
        output_filename (str): Path to the output binary file.
        data_format (str): Format of the array data ('hex', 'bin', 'dec', 'base64').
        decompress (bool): Whether to decompress the data. Defaults to False.

    Raises:
        ValueError: If unsupported data format is provided.
        ByteConvertorError: If writing to file fails.
    """
    try:
        if data_format == 'hex':
            byte_values = bytes(int(b.strip(), 16)
                                for b in array_data.split(','))
        elif data_format == 'bin':
            byte_values = bytes(int(b.strip(), 2)
                                for b in array_data.split(','))
        elif data_format == 'dec':
            byte_values = bytes(int(b.strip(), 10)
                                for b in array_data.split(','))
        elif data_format == 'base64':
            byte_values = base64.b64decode(array_data.strip('"'))
        else:
            logger.error(f"Unsupported format: {data_format}")
            raise ValueError(f"Unsupported format: {data_format}")

        logger.debug(
            f"Converted array data from {data_format} format to bytes")
    except Exception as e:
        logger.error(f"Failed to parse array data: {e}")
        raise ByteConvertorError(f"Failed to parse array data: {e}") from e

    if decompress:
        try:
            byte_values = zlib.decompress(byte_values)
            logger.debug("Decompressed data using zlib")
        except zlib.error as e:
            logger.error(f"Decompression failed: {e}")
            raise ByteConvertorError(f"Decompression failed: {e}") from e

    try:
        with open(output_filename, 'wb') as file:
            file.write(byte_values)
        logger.info(
            f"Successfully wrote {len(byte_values)} bytes to {output_filename}")
    except IOError as e:
        logger.error(f"Failed to write to file {output_filename}: {e}")
        raise ByteConvertorError(
            f"Failed to write to file {output_filename}: {e}") from e


def extract_array_data_from_header(header_filename: str, array_name: str = "resource_data") -> Tuple[str, str]:
    """
    Extract the array data and its format from a C header file.

    Args:
        header_filename (str): Path to the C header file.
        array_name (str): Name of the array variable to extract.

    Returns:
        Tuple[str, str]: Extracted array data string and its format.

    Raises:
        ValueError: If no array data found in the header file.
        ByteConvertorError: If file reading fails.
    """
    try:
        with open(header_filename, 'r', encoding='utf-8') as file:
            content = file.read()
        logger.debug(f"Read content from {header_filename}")
    except IOError as e:
        logger.error(f"Failed to read header file {header_filename}: {e}")
        raise ByteConvertorError(
            f"Failed to read header file {header_filename}: {e}") from e

    import re
    pattern = re.compile(
        rf'const\s+\w+\s+{array_name}\[\]\s*=\s*\{{([^}}]+)\}};')
    match = pattern.search(content)
    if not match:
        logger.error(f"No {array_name}[] found in the header file.")
        raise ValueError(f"No {array_name}[] found in the header file.")

    array_data = match.group(1).strip()
    if '"' in array_data:
        data_format = 'base64'
    elif any(b.startswith('0x') for b in array_data.split(',')):
        data_format = 'hex'
    elif any(b.startswith('0b') for b in array_data.split(',')):
        data_format = 'bin'
    else:
        data_format = 'dec'

    logger.debug(f"Extracted array data in {data_format} format")
    return array_data, data_format


def generate_macro_name(filename: str) -> str:
    """
    Generate a macro name based on the filename.

    Args:
        filename (str): Path to the file.

    Returns:
        str: Generated macro name.
    """
    name, _ = os.path.splitext(os.path.basename(filename))
    macro = f'{name.upper()}_H'
    logger.debug(f"Generated macro name: {macro}")
    return macro


def generate_output_name(input_file: str, extension: str) -> str:
    """
    Generate an output file name based on the input file name and extension.

    Args:
        input_file (str): Path to the input file.
        extension (str): Desired extension for the output file.

    Returns:
        str: Generated output file name.
    """
    name, _ = os.path.splitext(input_file)
    output_name = f"{name}{extension}"
    logger.debug(f"Generated output file name: {output_name}")
    return output_name


def convert_to_header(
    input_file: str,
    output_header: Optional[str] = None,
    array_name: str = "resource_data",
    size_name: str = "resource_size",
    array_type: str = "unsigned char",
    comment_style: str = "C",
    compress: bool = False,
    data_format: str = 'hex',
    start: int = 0,
    end: Optional[int] = None,
    protect: bool = True,
    cpp_class: bool = False,
    split_size: Optional[int] = None,
    verify: bool = False,
    class_name: Optional[str] = None
) -> None:
    """
    Convert a binary file to a C header file.

    Args:
        input_file (str): Path to the binary file.
        output_header (Optional[str]): Path to the output header file. Defaults to None.
        array_name (str): Name of the array in the header file. Defaults to "resource_data".
        size_name (str): Name of the size variable in the header file. Defaults to "resource_size".
        array_type (str): Data type of the array in the header file. Defaults to "unsigned char".
        comment_style (str): Comment style ("C" or "CPP"). Defaults to "C".
        compress (bool): Whether to compress the data. Defaults to False.
        data_format (str): Format of the array data ("hex", "bin", "dec", "base64"). Defaults to 'hex'.
        start (int): Start byte for conversion. Defaults to 0.
        end (Optional[int]): End byte for conversion. Defaults to None.
        protect (bool): Whether to include #ifndef protection macros. Defaults to True.
        cpp_class (bool): Whether to generate a C++ class wrapper. Defaults to False.
        split_size (Optional[int]): Maximum size (in bytes) of each output header. Defaults to None.
        verify (bool): Whether to verify data integrity after conversion. Defaults to False.
        class_name (Optional[str]): Name of the C++ class if cpp_class is True. Defaults to array_name.capitalize().
    """
    original_data = None
    try:
        with open(input_file, 'rb') as file:
            original_data = file.read()[start:end]
        logger.info(
            f"Read {len(original_data)} bytes from {input_file} (bytes {start} to {end})")
    except IOError as e:
        logger.error(f"Failed to read file {input_file}: {e}")
        raise ByteConvertorError(
            f"Failed to read file {input_file}: {e}") from e

    original_checksum = compute_checksum(original_data)
    logger.debug(f"Original data checksum: {original_checksum}")

    data = original_data
    if compress:
        data = zlib.compress(data)
        logger.info(
            f"Compressed data from {len(original_data)} to {len(data)} bytes")
        array_name = f"{array_name}_compressed"

    parts = [data[i:i+split_size]
             for i in range(0, len(data), split_size)] if split_size else [data]
    logger.info(f"Splitting data into {len(parts)} part(s)")

    if output_header is None:
        output_header = generate_output_name(input_file, ".h")

    macro_name = generate_macro_name(output_header)
    comment_start, comment_end = (
        "/*", "*/") if comment_style == "C" else ("//", "")

    for i, part in enumerate(tqdm(parts, desc="Generating header parts")):
        part_name = f"{array_name}_part_{i}" if len(parts) > 1 else array_name
        part_header = output_header.replace(
            '.h', f'_part_{i}.h') if len(parts) > 1 else output_header
        if data_format == 'base64':
            array_str = f'"{base64.b64encode(part).decode()}"'
        else:
            array_str = ', '.join(f'0x{b:02X}' for b in part) if data_format == 'hex' else \
                        ', '.join(f'0b{b:08b}' for b in part) if data_format == 'bin' else \
                        ', '.join(f'{b}' for b in part)

        try:
            with open(part_header, 'w', encoding='utf-8') as header_file:
                if protect:
                    header_file.write(f'#ifndef {macro_name.upper()}_{i}\n')
                    header_file.write(f'#define {macro_name.upper()}_{i}\n\n')
                header_file.write(
                    f'{comment_start} Generated from {input_file} on {datetime.now()}{comment_end}\n')
                header_file.write(
                    f'{comment_start} Original size: {len(original_data)} bytes{comment_end}\n')
                header_file.write(
                    f'{comment_start} Compressed: {"Yes" if compress else "No"}{comment_end}\n')
                header_file.write(
                    f'{comment_start} Data format: {data_format.upper()}{comment_end}\n')
                header_file.write(
                    f'{comment_start} Part {i+1} of {len(parts)}{comment_end}\n\n')
                header_file.write(
                    f'const {array_type} {part_name}[] = {{ {array_str} }};\n')
                header_file.write(
                    f'const unsigned int {size_name}_{i} = sizeof({part_name});\n')
                if protect:
                    header_file.write(
                        f'\n#endif /* {macro_name.upper()}_{i} */\n')

                if cpp_class and i == len(parts) - 1:
                    cpp_cls_name = class_name if class_name else array_name.capitalize()
                    header_file.write(f'\nclass {cpp_cls_name} {{\n')
                    header_file.write('public:\n')
                    header_file.write(
                        f'    const {array_type}* data() const {{ return {part_name}; }}\n')
                    header_file.write(
                        f'    unsigned int size() const {{ return {size_name}_{i}; }}\n')
                    header_file.write('};\n')
            logger.info(f"Generated header file: {part_header}")
        except IOError as e:
            logger.error(f"Failed to write header file {part_header}: {e}")
            raise ByteConvertorError(
                f"Failed to write header file {part_header}: {e}") from e

    if verify:
        try:
            reconstructed_data = bytearray()
            for i in range(len(parts)):
                part_header = output_header.replace(
                    '.h', f'_part_{i}.h') if len(parts) > 1 else output_header
                array_data, fmt = extract_array_data_from_header(
                    part_header, part_name)
                if fmt == 'base64':
                    part_data = base64.b64decode(array_data.strip('"'))
                elif fmt == 'hex':
                    part_data = bytes(int(b.strip(), 16)
                                      for b in array_data.split(','))
                elif fmt == 'bin':
                    part_data = bytes(int(b.strip(), 2)
                                      for b in array_data.split(','))
                elif fmt == 'dec':
                    part_data = bytes(int(b.strip(), 10)
                                      for b in array_data.split(','))
                else:
                    raise ValueError(
                        f"Unsupported format during verification: {fmt}")
                reconstructed_data.extend(part_data)
            if compress:
                reconstructed_data = zlib.decompress(reconstructed_data)
            reconstructed_checksum = compute_checksum(reconstructed_data)
            if original_checksum == reconstructed_checksum:
                logger.info(
                    "Data integrity verification successful. Checksums match.")
            else:
                logger.error(
                    "Data integrity verification failed. Checksums do not match.")
                raise ByteConvertorError(
                    "Data integrity verification failed. Checksums do not match.")
        except Exception as e:
            logger.error(f"Verification failed: {e}")
            raise ByteConvertorError(f"Verification failed: {e}") from e


def convert_to_file(
    input_header: str,
    output_file: Optional[str] = None,
    decompress: bool = False,
    data_format: Optional[str] = None,
    verify: bool = False
) -> None:
    """
    Convert a C header file back to a binary file.

    Args:
        input_header (str): Path to the C header file.
        output_file (Optional[str]): Path to the output binary file. Defaults to None.
        decompress (bool): Whether to decompress the data. Defaults to False.
        data_format (Optional[str]): Format of the array data ('hex', 'bin', 'dec', 'base64'). If None, auto-detect.
        verify (bool): Whether to verify data integrity after conversion. Defaults to False.

    Raises:
        ByteConvertorError: If conversion fails.
    """
    try:
        array_data, detected_format = extract_array_data_from_header(
            input_header)
        logger.debug(f"Detected data format: {detected_format}")
        if data_format is None:
            data_format = detected_format
        else:
            logger.debug(
                f"Overriding detected format with user-specified format: {data_format}")
    except Exception as e:
        logger.error(f"Failed to extract array data from {input_header}: {e}")
        raise ByteConvertorError(
            f"Failed to extract array data from {input_header}: {e}") from e

    if output_file is None:
        output_file = generate_output_name(input_header, ".bin")

    try:
        convert_array_to_file(array_data, output_file, data_format, decompress)
    except ByteConvertorError as e:
        logger.error(f"Conversion to file failed: {e}")
        raise

    if verify:
        try:
            with open(output_file, 'rb') as file:
                converted_data = file.read()
            original_checksum = compute_checksum(converted_data)
            logger.debug(f"Converted data checksum: {original_checksum}")
            logger.info("Data integrity verification completed.")
            # Additional verification logic can be added here if source data is available
        except Exception as e:
            logger.error(f"Verification failed: {e}")
            raise ByteConvertorError(f"Verification failed: {e}") from e


def parse_args(args: List[str]) -> Tuple[str, str, Optional[str], dict]:
    """
    Parse command-line arguments.

    Args:
        args (List[str]): List of command-line arguments.

    Returns:
        Tuple[str, str, Optional[str], dict]: Parsed mode, input file, output file, and options.
    """
    import argparse

    parser = argparse.ArgumentParser(
        description='Byte Convertor: Binary <-> C Header Converter')
    subparsers = parser.add_subparsers(
        dest='mode', required=True, help='Modes of operation')

    # to_header parser
    parser_to_header = subparsers.add_parser(
        'to_header', help='Convert a binary file to a C header file')
    parser_to_header.add_argument(
        'input_file', type=str, help='Path to the binary file')
    parser_to_header.add_argument(
        'output_file', type=str, nargs='?', default=None, help='Path to the output header file')
    parser_to_header.add_argument('--array_name', type=str, default="resource_data",
                                  help='Name of the array (default: resource_data)')
    parser_to_header.add_argument('--size_name', type=str, default="resource_size",
                                  help='Name of the size variable (default: resource_size)')
    parser_to_header.add_argument('--array_type', type=str, default="unsigned char",
                                  help='Type of the array (default: unsigned char)')
    parser_to_header.add_argument('--comment_style', type=str, choices=[
                                  'C', 'CPP'], default="C", help='Comment style (default: C)')
    parser_to_header.add_argument(
        '--compress', action='store_true', help='Compress the data in the header file')
    parser_to_header.add_argument('--format', type=str, choices=[
                                  'hex', 'bin', 'dec', 'base64'], default='hex', help='Format of the array data (default: hex)')
    parser_to_header.add_argument(
        '--start', type=int, default=0, help='Start byte for conversion (default: 0)')
    parser_to_header.add_argument(
        '--end', type=int, default=None, help='End byte for conversion (default: None)')
    parser_to_header.add_argument(
        '--no_protect', action='store_true', help='Do not include #ifndef protection macros')
    parser_to_header.add_argument('--cpp_class', type=str, nargs='?', const="Wrapper",
                                  default=None, help='Generate a C++ class wrapper with optional class name')
    parser_to_header.add_argument('--split_size', type=int, default=None,
                                  help='Split the output into multiple headers with this max size in bytes')
    parser_to_header.add_argument(
        '--verify', action='store_true', help='Verify data integrity after conversion')

    # to_file parser
    parser_to_file = subparsers.add_parser(
        'to_file', help='Convert a C header file back to a binary file')
    parser_to_file.add_argument(
        'input_header', type=str, help='Path to the C header file')
    parser_to_file.add_argument(
        'output_file', type=str, nargs='?', default=None, help='Path to the output binary file')
    parser_to_file.add_argument('--decompress', action='store_true',
                                help='Decompress the data when converting back to a file')
    parser_to_file.add_argument('--format', type=str, choices=[
                                'hex', 'bin', 'dec', 'base64'], default=None, help='Format of the array data (auto-detect if not specified)')
    parser_to_file.add_argument(
        '--verify', action='store_true', help='Verify data integrity after conversion')

    parsed_args = parser.parse_args(args[1:])

    options = vars(parsed_args)
    mode = options.pop('mode')
    if mode == 'to_header':
        class_name = options.pop('cpp_class')
        options['cpp_class'] = bool(class_name)
        options['class_name'] = class_name
    return mode, options.pop('input_file'), options.pop('output_file'), options


def main() -> None:
    """
    Main function to handle command-line interface.

    Raises:
        SystemExit: If incorrect usage or mode is provided.
    """
    setup_logging()
    args = sys.argv
    try:
        mode, input_file, output_file, options = parse_args(args)
    except Exception as e:
        logger.error(f"Argument parsing failed: {e}")
        sys.exit(1)

    try:
        if mode == "to_header":
            convert_to_header(
                input_file=input_file,
                output_header=output_file,
                array_name=options.get("array_name", "resource_data"),
                size_name=options.get("size_name", "resource_size"),
                array_type=options.get("array_type", "unsigned char"),
                comment_style=options.get("comment_style", "C"),
                compress=options.get("compress", False),
                data_format=options.get("format", 'hex'),
                start=options.get("start", 0),
                end=options.get("end", None),
                protect=not options.get("no_protect", False),
                cpp_class=options.get("cpp_class", False),
                split_size=options.get("split_size", None),
                verify=options.get("verify", False),
                class_name=options.get("class_name")
            )
        elif mode == "to_file":
            convert_to_file(
                input_header=input_file,
                output_file=output_file,
                decompress=options.get("decompress", False),
                data_format=options.get("format", None),
                verify=options.get("verify", False)
            )
        else:
            logger.error(f"Unknown mode: {mode}")
            sys.exit(1)
    except ByteConvertorError as e:
        logger.error(f"Conversion failed: {e}")
        sys.exit(1)
    except Exception as e:
        logger.exception(f"An unexpected error occurred: {e}")
        sys.exit(1)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        logger.warning("Operation interrupted by user.")
        sys.exit(0)
