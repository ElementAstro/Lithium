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

Author: [Your Name]
Date: [Date]
"""

import clang.cindex
from loguru import logger
import argparse
import os

from .finder import get_libclang_path

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


def parse_args():
    """
    Parses command-line arguments for the script.

    This function uses argparse to define and parse command-line arguments. It includes options
    for specifying file paths, output file, log level, blacklist, whitelist, module name, and
    instance prefix.

    Returns:
        argparse.Namespace: The parsed command-line arguments.
    """
    parser = argparse.ArgumentParser(
        description="Generate ATOM_MODULE from C++ headers.")
    parser.add_argument("filepaths", nargs='+',
                        help="Paths to the C++ header files.")
    parser.add_argument("--output", default=None,
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
    return parser.parse_args()


def is_in_list(name, whitelist, blacklist):
    """
    Checks if a name is in the whitelist and not in the blacklist.

    Args:
        name (str): The name to check.
        whitelist (list): The list of whitelisted names.
        blacklist (list): The list of blacklisted names.

    Returns:
        bool: True if the name is in the whitelist or if the whitelist is empty, and not in the blacklist.
    """
    if whitelist and name not in whitelist:
        return False
    if name in blacklist:
        return False
    return True


def generate_atom_module(filepaths, output_file=None, log_level="INFO", whitelist=None, blacklist=None, module_name="all_components", instance_prefix=""):
    """
    Generates ATOM_MODULE code from specified C++ header files.

    Args:
        filepaths (list): List of paths to C++ header files.
        output_file (str, optional): Path to the output file for generated code. If None, prints to console.
        log_level (str, optional): The log level for the output (DEBUG, INFO, WARNING, ERROR).
        whitelist (list, optional): List of whitelisted functions or methods.
        blacklist (list, optional): List of blacklisted functions or methods.
        module_name (str, optional): Name of the generated ATOM_MODULE.
        instance_prefix (str, optional): Prefix for instance names in the module.
    """
    # Set the log level
    logger.remove()
    logger.add(lambda msg: print(msg, end=""), level=log_level.upper())

    # Initialize containers for classes and functions
    all_classes = {}
    all_functions = []

    for filepath in filepaths:
        index = clang.cindex.Index.create()
        translation_unit = index.parse(filepath)
        logger.info(f"Parsing the file: {filepath}\n")

        def find_classes_methods_and_functions(node, namespace=""):
            """
            Recursively finds classes, methods, and functions within the AST.

            Args:
                node (clang.cindex.Cursor): The current AST node.
                namespace (str, optional): The current namespace.

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
                        child, nested_namespace)
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

        classes, functions = find_classes_methods_and_functions(
            translation_unit.cursor)
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


# Run the script
if __name__ == "__main__":
    """
    Entry point for the script execution.

    Parses command-line arguments and generates ATOM_MODULE code based on the provided header files.
    Configures logging and handles exceptions during module generation.
    """
    args = parse_args()
    generate_atom_module(args.filepaths, args.output, args.log_level,
                         args.whitelist, args.blacklist, args.module_name, args.instance_prefix)
