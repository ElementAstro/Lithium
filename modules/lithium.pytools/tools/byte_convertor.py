#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
File: convert_to_header.py
Author: Max Qian <astro_air@126.com>
Date: 2024-08-26
Version: 1.0

Description:
------------
This Python script provides functionality to convert binary files into C-style header files
containing array data, and vice versa. The script supports several features, including data
compression, various data formats (hex, binary, decimal), optional C++ class wrappers, and
splitting large arrays into multiple header files.

The script can be used in two main modes:
    1. `to_header`: Converts a binary file into a C header file with array data.
    2. `to_file`: Converts a C header file containing array data back into a binary file.

Features:
---------
- Convert binary files to C-style arrays in hex, binary, or decimal format.
- Compress the binary data before storing it in the header file.
- Generate optional C++ class wrappers for the array data.
- Optionally split large arrays across multiple header files.
- Generate `#ifndef` protection macros to prevent multiple inclusions of the same header file.
- Customize array and size variable names, array data types, and comment styles.

Dependencies:
-------------
- Python 3.x
- Standard Python libraries: sys, os, zlib, datetime, typing

Usage:
------
The script can be executed directly from the command line. Below is the usage syntax:

    Usage: convert_to_header.py <mode> <input_file> [output_file] [options]

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
    --format <hex|bin|dec>      : Set the format of the array data (default: hex).
    --start <byte_offset>       : Set the start byte for conversion.
    --end <byte_offset>         : Set the end byte for conversion.
    --no_protect                : Do not include #ifndef protection macros.
    --cpp_class                 : Generate a simple C++ class wrapper.
    --split_size <bytes>        : Split the output into multiple headers with this max size.

    Options for 'to_file':
    -----------------------
    --decompress                : Decompress the data when converting back to a file.

Examples:
---------
1. Convert a binary file to a C header file:
    $ python3 convert_to_header.py to_header my_binary.bin my_header.h --compress --array_name my_array

2. Convert a C header file back to a binary file:
    $ python3 convert_to_header.py to_file my_header.h my_binary.bin --decompress

3. Split a large binary file into multiple headers, each with a maximum size of 1024 bytes:
    $ python3 convert_to_header.py to_header large_binary.bin --split_size 1024

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
from datetime import datetime
from typing import Optional, List, Tuple


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
        data_format (str): Format of the array data ('hex', 'bin', 'dec'). Defaults to 'hex'.

    Returns:
        str: C-style array string.
    """
    with open(filename, 'rb') as file:
        data = file.read()[start:end]

    if data_format == 'hex':
        return ', '.join(f'0x{b:02X}' for b in data)
    elif data_format == 'bin':
        return ', '.join(f'0b{b:08b}' for b in data)
    elif data_format == 'dec':
        return ', '.join(f'{b}' for b in data)
    else:
        raise ValueError(f"Unsupported format: {data_format}")


def convert_array_to_file(
    array_data: str,
    output_filename: str
) -> None:
    """
    Convert a C-style array string back to a binary file.

    Args:
        array_data (str): C-style array string.
        output_filename (str): Path to the output binary file.
    """
    byte_values = [
        int(b.strip(), 16 if '0x' in b else 2 if '0b' in b else 10)
        for b in array_data.split(',')
    ]

    with open(output_filename, 'wb') as file:
        file.write(bytes(byte_values))


def extract_array_data_from_header(header_filename: str) -> str:
    """
    Extract the array data from a C header file.

    Args:
        header_filename (str): Path to the C header file.

    Returns:
        str: Extracted array data string.

    Raises:
        ValueError: If no array data found in the header file.
    """
    with open(header_filename, 'r', encoding='utf-8') as file:
        lines = file.readlines()

    for line in lines:
        if 'resource_data[]' in line:
            array_data = line.split('{')[1].split('}')[0].strip()
            return array_data

    raise ValueError("No resource_data[] found in the header file.")


def generate_macro_name(filename: str) -> str:
    """
    Generate a macro name based on the filename.

    Args:
        filename (str): Path to the file.

    Returns:
        str: Generated macro name.
    """
    name, _ = os.path.splitext(os.path.basename(filename))
    return f'RESOURCE_{name.upper()}_H'


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
    return f"{name}{extension}"


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
    split_size: Optional[int] = None
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
        data_format (str): Format of the array data ("hex", "bin", "dec"). Defaults to 'hex'.
        start (int): Start byte for conversion. Defaults to 0.
        end (Optional[int]): End byte for conversion. Defaults to None.
        protect (bool): Whether to include #ifndef protection macros. Defaults to True.
        cpp_class (bool): Whether to generate a C++ class wrapper. Defaults to False.
        split_size (Optional[int]): Maximum size (in bytes) of each output header. Defaults to None.
    """
    with open(input_file, 'rb') as file:
        data = file.read()[start:end]

    original_size = len(data)

    if compress:
        data = zlib.compress(data)
        array_name = f"{array_name}_compressed"

    parts = [data[i:i+split_size]
             for i in range(0, len(data), split_size)] if split_size else [data]

    if output_header is None:
        output_header = generate_output_name(input_file, ".h")

    macro_name = generate_macro_name(input_file)
    comment_start, comment_end = (
        "/*", "*/") if comment_style == "C" else ("//", "")

    for i, part in enumerate(parts):
        part_name = f"{array_name}_part_{i}" if len(parts) > 1 else array_name
        part_header = output_header.replace(
            '.h', f'_part_{i}.h') if len(parts) > 1 else output_header
        array_data = ', '.join(f'0x{b:02X}' for b in part)

        with open(part_header, 'w', encoding='utf-8') as header_file:
            if protect:
                header_file.write(f'#ifndef {macro_name}\n')
                header_file.write(f'#define {macro_name}\n\n')
            header_file.write(f'{comment_start} Generated from {input_file}\n')
            header_file.write(
                f'{comment_start} Original size: {original_size} bytes\n')
            header_file.write(
                f'{comment_start} Compressed: {"Yes" if compress else "No"}\n')
            header_file.write(
                f'{comment_start} Generated on: {datetime.now()}\n{comment_end}\n\n')
            header_file.write(
                f'const {array_type} {part_name}[] = {{ {array_data} }};\n')
            header_file.write(
                f'const unsigned int {size_name}_{i} = sizeof({part_name});\n')
            if protect:
                header_file.write('#endif\n')

            if cpp_class and i == len(parts) - 1:
                header_file.write('\n')
                header_file.write('class {array_name.capitalize()}Wrapper {\n')
                header_file.write('public:\n')
                header_file.write(
                    f'    const {array_type}* data() const {{ return {array_name}; }}\n')
                header_file.write(
                    f'    unsigned int size() const {{ return {size_name}; }}\n')
                header_file.write('};\n')


def convert_to_file(
    input_header: str,
    output_file: Optional[str] = None,
    decompress: bool = False
) -> None:
    """
    Convert a C header file back to a binary file.

    Args:
        input_header (str): Path to the C header file.
        output_file (Optional[str]): Path to the output binary file. Defaults to None.
        decompress (bool): Whether to decompress the data. Defaults to False.
    """
    array_data = extract_array_data_from_header(input_header)

    if output_file is None:
        output_file = generate_output_name(input_header, ".bin")

    raw_data = bytes(int(b.strip(), 16) for b in array_data.split(','))

    if decompress:
        raw_data = zlib.decompress(raw_data)

    with open(output_file, 'wb') as file:
        file.write(raw_data)


def parse_args(args: List[str]) -> Tuple[str, str, Optional[str], dict]:
    """
    Parse command-line arguments.

    Args:
        args (List[str]): List of command-line arguments.

    Returns:
        Tuple[str, str, Optional[str], dict]: Parsed mode, input file, output file, and options.
    """
    mode = args[1]
    input_file = args[2]
    output_file = args[3] if len(
        args) > 3 and not args[3].startswith("--") else None

    options = {
        "array_name": "resource_data",
        "size_name": "resource_size",
        "array_type": "unsigned char",
        "comment_style": "C",
        "compress": False,
        "data_format": 'hex',
        "start": 0,
        "end": None,
        "protect": True,
        "cpp_class": False,
        "split_size": None,
        "decompress": False
    }

    for i in range(3, len(args)):
        if args[i] == "--array_name" and i + 1 < len(args):
            options["array_name"] = args[i + 1]
        elif args[i] == "--size_name" and i + 1 < len(args):
            options["size_name"] = args[i + 1]
        elif args[i] == "--array_type" and i + 1 < len(args):
            options["array_type"] = args[i + 1]
        elif args[i] == "--comment_style" and i + 1 < len(args):
            options["comment_style"] = args[i + 1].upper()
        elif args[i] == "--compress":
            options["compress"] = True
        elif args[i] == "--format" and i + 1 < len(args):
            options["data_format"] = args[i + 1]
        elif args[i] == "--start" and i + 1 < len(args):
            options["start"] = int(args[i + 1])
        elif args[i] == "--end" and i + 1 < len(args):
            options["end"] = int(args[i + 1])
        elif args[i] == "--no_protect":
            options["protect"] = False
        elif args[i] == "--cpp_class":
            options["cpp_class"] = True
        elif args[i] == "--split_size" and i + 1 < len(args):
            options["split_size"] = int(args[i + 1])
        elif args[i] == "--decompress":
            options["decompress"] = True

    return mode, input_file, output_file, options


def main() -> None:
    """
    Main function to handle command-line interface.

    Raises:
        SystemExit: If incorrect usage or mode is provided.
    """
    if len(sys.argv) < 3:
        print(
            "Usage: convert_to_header.py <mode> <input_file> [output_file] [options]"
        )
        print("Modes:")
        print("  to_header - Convert a binary file to a C header file")
        print("  to_file   - Convert a C header file back to a binary file")
        print("Options for 'to_header':")
        print("  --array_name <name>         Set the name of the array (default: resource_data)")
        print("  --size_name <name>          Set the name of the size variable (default: resource_size)")
        print("  --array_type <type>         Set the type of the array (default: unsigned char)")
        print("  --comment_style <C|CPP>     Set the comment style (default: C)")
        print("  --compress                  Compress the data in the header file")
        print(
            "  --format <hex|bin|dec>      Set the format of the array data (default: hex)"
        )
        print("  --start <byte_offset>       Set the start byte for conversion")
        print("  --end <byte_offset>         Set the end byte for conversion")
        print("  --no_protect                Do not include #ifndef protection macros")
        print("  --cpp_class                 Generate a simple C++ class wrapper")
        print("  --split_size <bytes>        Split the output into multiple headers with this max size")
        print("Options for 'to_file':")
        print("  --decompress                Decompress the data when converting back to a file")
        sys.exit(1)

    mode, input_file, output_file, options = parse_args(sys.argv)

    if mode == "to_header":
        convert_to_header(
            input_file=input_file,
            output_header=output_file,
            array_name=options["array_name"],
            size_name=options["size_name"],
            array_type=options["array_type"],
            comment_style=options["comment_style"],
            compress=options["compress"],
            data_format=options["data_format"],
            start=options["start"],
            end=options["end"],
            protect=options["protect"],
            cpp_class=options["cpp_class"],
            split_size=options["split_size"],
        )
    elif mode == "to_file":
        convert_to_file(
            input_header=input_file,
            output_file=output_file,
            decompress=options["decompress"],
        )
    else:
        print(f"Unknown mode: {mode}")
        sys.exit(1)


if __name__ == "__main__":
    main()
