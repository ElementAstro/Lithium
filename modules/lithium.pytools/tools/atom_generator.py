"""
ATOM_MODULE Generator for C++ Headers

This script generates ATOM_MODULE code from C++ header files. It parses header files to extract
public methods and global functions, then generates code that registers these components. It
supports configuring logging levels, blacklisting, and whitelisting of functions or methods.

Dependencies:
- clang.cindex: For parsing C++ headers using libclang.
- loguru: For detailed logging.
- argparse: For command-line argument parsing.

Usage:
- Provide paths to C++ header files via the `filepaths` argument.
- Optionally specify output file, log level, whitelist, blacklist, module name, and instance prefix.

"""

import clang.cindex
from loguru import logger
import argparse
import os
import threading
import yaml
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from typing import List, Optional, Dict, Tuple
from dataclasses import dataclass, field

from .libclang_finder import get_libclang_path

# Set the path to libclang
try:
    libclang_path = get_libclang_path()
    clang.cindex.Config.set_library_file(libclang_path)
    logger.info(f"Successfully set libclang path to: {libclang_path}")
except Exception as e:
    logger.exception("Failed to set libclang path")

# Default blacklist and whitelist
DEFAULT_BLACKLIST = []
DEFAULT_WHITELIST = []


@dataclass
class GeneratorConfig:
    """
    Configuration parameters for the ATOM_MODULE generator.
    """
    filepaths: List[Path]
    output: Optional[Path] = None
    log_level: str = "INFO"
    blacklist: List[str] = field(default_factory=lambda: DEFAULT_BLACKLIST)
    whitelist: List[str] = field(default_factory=lambda: DEFAULT_WHITELIST)
    module_name: str = "all_components"
    instance_prefix: str = ""
    config_file: Optional[Path] = None


def parse_args() -> GeneratorConfig:
    """
    Parses command-line arguments for the script.

    This function uses argparse to define and parse command-line arguments. It includes options
    for specifying file paths, output file, log level, blacklist, whitelist, module name, and
    instance prefix.

    Returns:
        GeneratorConfig: The parsed command-line arguments.
    """
    parser = argparse.ArgumentParser(
        description="Generate ATOM_MODULE from C++ headers.")
    parser.add_argument("filepaths", nargs='+', type=Path,
                        help="Paths to the C++ header files.")
    parser.add_argument("--output", type=Path, default=None,
                        help="Output file for generated module code.")
    parser.add_argument("--log-level", default="INFO",
                        help="Set the log level (DEBUG, INFO, WARNING, ERROR).")
    parser.add_argument("--blacklist", nargs="*", default=DEFAULT_BLACKLIST,
                        help="List of blacklisted functions or methods.")
    parser.add_argument("--whitelist", nargs="*", default=DEFAULT_WHITELIST,
                        help="List of whitelisted functions or methods.")
    parser.add_argument("--module-name", default="all_components",
                        help="Name of the generated ATOM_MODULE.")
    parser.add_argument("--instance-prefix", default="",
                        help="Prefix for instance names in the module.")
    parser.add_argument("--config-file", type=Path, default=None,
                        help="Path to a YAML configuration file.")
    args = parser.parse_args()
    return GeneratorConfig(
        filepaths=args.filepaths,
        output=args.output,
        log_level=args.log_level,
        blacklist=args.blacklist,
        whitelist=args.whitelist,
        module_name=args.module_name,
        instance_prefix=args.instance_prefix,
        config_file=args.config_file
    )


def load_config(config_file: Optional[Path]) -> Dict:
    """
    Loads configuration from a YAML file.

    Args:
        config_file (Optional[Path]): Path to the configuration file.

    Returns:
        dict: The loaded configuration.
    """
    if config_file and config_file.is_file():
        with open(config_file, 'r') as f:
            config = yaml.safe_load(f)
        logger.info(f"Loaded configuration from {config_file}")
        return config
    return {}


def is_in_list(name: str, whitelist: List[str], blacklist: List[str]) -> bool:
    """
    Checks if a name is in the whitelist and not in the blacklist.

    Args:
        name (str): The name to check.
        whitelist (List[str]): The list of whitelisted names.
        blacklist (List[str]): The list of blacklisted names.

    Returns:
        bool: True if the name is in the whitelist or if the whitelist is empty, and not in the blacklist.
    """
    if whitelist and name not in whitelist:
        return False
    if name in blacklist:
        return False
    return True


def find_classes_methods_and_functions(node: clang.cindex.Cursor, namespace: str = "", whitelist: Optional[List[str]] = None, blacklist: Optional[List[str]] = None) -> Tuple[Dict[str, List[str]], List[str]]:
    """
    Recursively finds classes, methods, and functions within the AST.

    Args:
        node (clang.cindex.Cursor): The current AST node.
        namespace (str, optional): The current namespace.
        whitelist (List[str], optional): List of whitelisted functions or methods.
        blacklist (List[str], optional): List of blacklisted functions or methods.

    Returns:
        tuple: A dictionary of classes and their methods, and a list of functions.
    """
    classes = {}
    functions = []
    for child in node.get_children():
        # Handle namespaces
        if child.kind == clang.cindex.CursorKind.NAMESPACE:
            nested_namespace = f"{namespace}::{child.spelling}" if namespace else child.spelling
            nested_classes, nested_functions = find_classes_methods_and_functions(
                child, nested_namespace, whitelist, blacklist)
            classes.update(nested_classes)
            functions.extend(nested_functions)
        # Handle class declarations
        elif child.kind == clang.cindex.CursorKind.CLASS_DECL:
            class_name = f"{namespace}::{child.spelling}" if namespace else child.spelling
            methods = []
            for sub_child in child.get_children():
                if (sub_child.kind == clang.cindex.CursorKind.CXX_METHOD and
                        sub_child.access_specifier == clang.cindex.AccessSpecifier.PUBLIC):
                    method_name = sub_child.spelling
                    if is_in_list(method_name, whitelist, blacklist):
                        methods.append(method_name)
                        logger.debug(
                            f"Found public method: {class_name}::{method_name}")
            if methods:
                classes[class_name] = methods
                logger.debug(f"Registered class: {class_name}")
        # Handle global function declarations
        elif child.kind == clang.cindex.CursorKind.FUNCTION_DECL:
            function_name = f"{namespace}::{child.spelling}" if namespace else child.spelling
            if is_in_list(function_name, whitelist, blacklist):
                functions.append(function_name)
                logger.debug(f"Found global function: {function_name}")
    return classes, functions


def parse_header_file(filepath: Path, whitelist: Optional[List[str]] = None, blacklist: Optional[List[str]] = None) -> Tuple[Dict[str, List[str]], List[str]]:
    """
    Parses a C++ header file to extract classes, methods, and functions.

    Args:
        filepath (Path): Path to the C++ header file.
        whitelist (List[str], optional): List of whitelisted functions or methods.
        blacklist (List[str], optional): List of blacklisted functions or methods.

    Returns:
        tuple: A dictionary of classes and their methods, and a list of functions.
    """
    index = clang.cindex.Index.create()
    translation_unit = index.parse(str(filepath))
    logger.info(f"Parsing the file: {filepath}\n")
    return find_classes_methods_and_functions(translation_unit.cursor, whitelist=whitelist, blacklist=blacklist)


def generate_atom_module(filepaths: List[Path], output_file: Optional[Path] = None, log_level: str = "INFO", whitelist: Optional[List[str]] = None, blacklist: Optional[List[str]] = None, module_name: str = "all_components", instance_prefix: str = ""):
    """
    Generates ATOM_MODULE code from specified C++ header files.

    Args:
        filepaths (List[Path]): List of paths to C++ header files.
        output_file (Optional[Path], optional): Path to the output file for generated code. If None, prints to console.
        log_level (str, optional): The log level for the output (DEBUG, INFO, WARNING, ERROR).
        whitelist (List[str], optional): List of whitelisted functions or methods.
        blacklist (List[str], optional): List of blacklisted functions or methods.
        module_name (str, optional): Name of the generated ATOM_MODULE.
        instance_prefix (str, optional): Prefix for instance names in the module.
    """
    # Set the log level
    logger.remove()
    logger.add(lambda msg: print(msg, end=""), level=log_level.upper())

    # Initialize containers for classes and functions
    all_classes = {}
    all_functions = []

    with ThreadPoolExecutor() as executor:
        futures = [executor.submit(
            parse_header_file, filepath, whitelist, blacklist) for filepath in filepaths]
        for future in futures:
            classes, functions = future.result()
            all_classes.update(classes)
            all_functions.extend(functions)

    logger.info("Generating ATOM_MODULE...\n")

    # Start building the module code
    code_lines = []
    code_lines.append(
        f'ATOM_MODULE({module_name}, [](Component &component) {{')
    code_lines.append(f'    LOG_F(INFO, "Registering all components...");\n')

    # Process classes and their methods
    for class_name, methods in all_classes.items():
        for method in methods:
            code_lines.append(
                f'    component.def("{method}", &{class_name}::{method}, "main", "{method} of {class_name}");')
            logger.info(f"Registered method: {class_name}::{method}")

        # Use the last part of the class name as the instance name
        instance_name = f"{instance_prefix}{class_name.split('::')[-1].lower()}"
        code_lines.append(
            f'    component.addVariable("{instance_name}.instance", "{class_name} instance");')
        code_lines.append(
            f'    component.defType<{class_name}>("{instance_name}");')
        logger.info(f"Registered class type: {class_name}")

    # Process global functions
    if all_functions:
        for function in all_functions:
            function_name = function.split("::")[-1]
            code_lines.append(
                f'    component.def("{function_name}", &{function}, "main", "Global function {function}");')
            logger.info(f"Registered global function: {function}")

    code_lines.append(f'    LOG_F(INFO, "All components registered.");')
    code_lines.append('});\n')

    logger.info("ATOM_MODULE generation completed.\n")

    generated_code = "\n".join(code_lines)

    # Output to file or print to console
    if output_file:
        with open(output_file, 'w') as f:
            f.write(generated_code)
        logger.info(f"Generated code written to: {output_file}\n")
    else:
        print(generated_code)


def main():
    """
    Entry point for the script execution.

    Parses command-line arguments and generates ATOM_MODULE code based on the provided header files.
    Configures logging and handles exceptions during module generation.
    """
    args = parse_args()
    config = load_config(args.config_file)
    generate_atom_module(
        args.filepaths,
        args.output or config.get('output'),
        args.log_level or config.get('log_level', 'INFO'),
        args.whitelist or config.get('whitelist', DEFAULT_WHITELIST),
        args.blacklist or config.get('blacklist', DEFAULT_BLACKLIST),
        args.module_name or config.get('module_name', 'all_components'),
        args.instance_prefix or config.get('instance_prefix', '')
    )


if __name__ == "__main__":
    main()
