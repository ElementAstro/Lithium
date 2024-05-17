#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
C++ AST Generator and pybind11 Binding Generator

This script generates the Abstract Syntax Tree (AST) for C++ header files (.hpp)
in a specified directory and its subdirectories. The AST information is exported
to a JSON file. Additionally, pybind11 bindings are automatically generated.

Usage:
    python generator.py <directory> <output_file> <bindings_file>

Dependencies:
    - clang
    - libclang
    - pybind11
"""

import os
import sys
import clang.cindex
from clang.cindex import CursorKind
import json

def camel_to_snake(name):
    if not isinstance(name, str) or not name:
        return ""
    snake_case = []
    for i, char in enumerate(name):
        if char.isupper():
            if i == 0:
                snake_case.append(char.lower())
            else:
                if not name[i - 1].isupper():
                    snake_case.append('_')
                snake_case.append(char.lower())
        else:
            snake_case.append(char)

    return ''.join(snake_case)

def parse_namespace(cursor) -> dict:
    '''
    Parse a CursorKind.NAMESPACE cursor and return its information as a dictionary.
    Args:
        cursor (Cursor): The CursorKind.NAMESPACE cursor to parse.
    Returns:
        dict: A dictionary containing the namespace information.
    '''
    namespace_info = {
        "type": "namespace",
        "name": cursor.spelling,
        "children": []
    }
    for child in cursor.get_children():
        child_info = parse_cursor(child)
        if child_info:
            namespace_info["children"].append(child_info)
    return namespace_info

def parse_class(cursor):
    """
    Parse a CursorKind.CLASS_DECL cursor and return its information as a dictionary.
    Args:
        cursor (Cursor): The CursorKind.CLASS_DECL cursor to parse.
    Returns:
        dict: A dictionary containing the class information.
    """
    class_info = {
        "type": "class",
        "name": cursor.spelling,
        "base_classes": [c.spelling for c in cursor.get_children() if c.kind == CursorKind.CXX_BASE_SPECIFIER],
        "constructors": [],
        "destructor": None,
        "member_variables": [],
        "children": []
    }

    for child in cursor.get_children():
        if child.kind == CursorKind.CONSTRUCTOR:
            class_info["constructors"].append(parse_function(child))
        elif child.kind == CursorKind.DESTRUCTOR:
            class_info["destructor"] = parse_function(child)
        elif child.kind == CursorKind.FIELD_DECL:
            class_info["member_variables"].append(parse_variable(child))
        else:
            child_info = parse_cursor(child)
            if child_info:
                class_info["children"].append(child_info)
    return class_info

def parse_struct(cursor):
    struct_info = {
        "type": "struct",
        "name": cursor.spelling,
        "member_variables": [],
        "children": []
    }
    for child in cursor.get_children():
        if child.kind == CursorKind.FIELD_DECL:
            struct_info["member_variables"].append(parse_variable(child))
        else:
            child_info = parse_cursor(child)
            if child_info:
                struct_info["children"].append(child_info)
    return struct_info

def parse_function(cursor):
    function_info = {
        "type": "function",
        "name": cursor.spelling,
        "return_type": cursor.result_type.spelling,
        "parameters": []
    }
    for child in cursor.get_children():
        if child.kind == CursorKind.PARM_DECL:
            function_info["parameters"].append({
                "name": child.spelling,
                "type": child.type.spelling
            })
    return function_info

def parse_enum(cursor):
    enum_info = {
        "type": "enum",
        "name": cursor.spelling,
        "constants": []
    }
    for child in cursor.get_children():
        if child.kind == CursorKind.ENUM_CONSTANT_DECL:
            enum_info["constants"].append({
                "name": child.spelling,
                "value": child.enum_value
            })
    return enum_info

def parse_macro(cursor):
    macro_info = {
        "type": "macro",
        "name": cursor.spelling,
        "definition": cursor.displayname
    }
    return macro_info

def parse_variable(cursor):
    variable_info = {
        "type": "variable",
        "name": cursor.spelling,
        "data_type": cursor.type.spelling
    }
    return variable_info

def parse_typedef(cursor):
    typedef_info = {
        "type": "typedef",
        "name": cursor.spelling,
        "underlying_type": cursor.underlying_typedef_type.spelling
    }
    return typedef_info

def parse_union(cursor):
    """
    Parse a CursorKind.UNION_DECL cursor and return its information as a dictionary.
    Args:
        cursor (Cursor): The CursorKind.UNION_DECL cursor to parse.
    Returns:
        dict: A dictionary containing the union information.
    """
    union_info = {
        "type": "union",
        "name": cursor.spelling,
        "children": []
    }
    for child in cursor.get_children():
        child_info = parse_cursor(child)
        if child_info:
            union_info["children"].append(child_info)
    return union_info

def parse_cursor(cursor):
    '''
    Parse a Cursor and return its information as a dictionary.
    Args:
        cursor (Cursor): The Cursor to parse.
    Returns:
        dict: A dictionary containing the cursor information.
    '''
    if cursor.kind == CursorKind.NAMESPACE:
        return parse_namespace(cursor)
    elif cursor.kind == CursorKind.CLASS_DECL or cursor.kind == CursorKind.STRUCT_DECL:
        if cursor.kind == CursorKind.CLASS_DECL:
            return parse_class(cursor)
        else:
            return parse_struct(cursor)
    elif cursor.kind == CursorKind.CXX_METHOD or cursor.kind == CursorKind.FUNCTION_DECL:
        return parse_function(cursor)
    elif cursor.kind == CursorKind.ENUM_DECL:
        return parse_enum(cursor)
    elif cursor.kind == CursorKind.MACRO_DEFINITION:
        return parse_macro(cursor)
    elif cursor.kind == CursorKind.VAR_DECL:
        return parse_variable(cursor)
    elif cursor.kind == CursorKind.TYPEDEF_DECL:
        return parse_typedef(cursor)
    elif cursor.kind == CursorKind.UNION_DECL:
        return parse_union(cursor)
    return None

def parse_cpp_file(file_path):
    """
    Parse a C++ header file (.hpp) and return its AST information as a dictionary.
    Args:
        file_path (str): The path to the C++ header file.
    Returns:
        dict: A dictionary containing the AST information.
    """
    index = clang.cindex.Index.create()
    translation_unit = index.parse(file_path, args=['-x', 'c++', '-std=c++17'])
    cursor = translation_unit.cursor

    ast_info = {
        "file": file_path,
        "children": []
    }

    for child in cursor.get_children():
        child_info = parse_cursor(child)
        if child_info:
            ast_info["children"].append(child_info)

    return ast_info

def parse_hpp_files(directory):
    """
    Parse all C++ header files (.hpp) in a directory and return their AST information as a list of dictionaries.
    Args:
        directory (str): The directory containing the C++ header files.
    Returns:
        list: A list of dictionaries containing the AST information.
    """
    if not os.path.exists(directory):
        raise ValueError("Folder does not exist")

    ast_info_list = []

    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(".hpp"):
                file_path = os.path.join(root, file)
                ast_info = parse_cpp_file(file_path)
                ast_info_list.append(ast_info)

    return ast_info_list

def generate_ast_json(directory, output_file):
    """
    Generate an AST JSON file from a directory of C++ header files.
    Args:
        directory (str): The directory containing the C++ header files.
        output_file (str): The path to the output JSON file.
    """
    ast_info_list = parse_hpp_files(directory)
    json_output = json.dumps(ast_info_list, indent=2)

    with open(output_file, 'w') as file:
        file.write(json_output)

    print(f"AST information written to file: {output_file}")

def generate_pybind11_bindings(ast_info_list, bindings_file):
    """
    Generate pybind11 bindings from the AST information.
    Args:
        ast_info_list (list): A list of dictionaries containing the AST information.
        bindings_file (str): The path to the output pybind11 bindings file.
    """
    bindings = ['#include <pybind11/pybind11.h>', '#include <pybind11/stl.h>', 'namespace py = pybind11;', 'PYBIND11_MODULE(module_name, m) {']

    def generate_function_binding(function_info, namespace=None):
        if namespace:
            function_name = f"{namespace}::{function_info['name']}"
        else:
            function_name = function_info['name']
        bindings.append(f'    m.def("{camel_to_snake(function_info["name"])}", &{function_name});')

    def generate_class_binding(class_info, namespace=None):
        if namespace:
            class_name = f"{namespace}::{class_info['name']}"
        else:
            class_name = class_info['name']
        bindings.append(f'    py::class_<{class_name}>(m, "{class_info["name"]}")')

        # Add constructors
        for constructor in class_info["constructors"]:
            parameter_types = ", ".join([param["type"] for param in constructor["parameters"]])
            if parameter_types:
                bindings.append(f'        .def(py::init<{parameter_types}>())')
            else:
                bindings.append('        .def(py::init<>())')

        # Add destructor if available
        if class_info["destructor"]:
            bindings.append('        .def("__del__", &{}::{})'.format(class_name, class_info["destructor"]["name"]))

        # Add member variables
        for member_variable in class_info["member_variables"]:
            bindings.append(f'        .def_readwrite("{member_variable["name"]}", &{class_name}::{member_variable["name"]})')

        # Add methods
        for child in class_info['children']:
            if child['type'] == 'function':
                if namespace:
                    method_name = f"{namespace}::{class_info['name']}::{child['name']}"
                else:
                    method_name = f"{class_info['name']}::{child['name']}"
                if len(child['parameters']) > 0:
                    bindings.append(f'        .def("{child["name"]}", py::overload_cast<{", ".join([param["type"] for param in child["parameters"]])}>(&{method_name}))')
                else:
                    bindings.append(f'        .def("{child["name"]}", &{method_name})')

        # Add enumerations
        for child in class_info['children']:
            if child['type'] == 'enum':
                generate_enum_binding(child, class_name)

        bindings[-1] += ';'

    def generate_enum_binding(enum_info, enclosing_class=None):
        if enclosing_class:
            enum_name = f"{enclosing_class}::{enum_info['name']}"
        else:
            enum_name = enum_info['name']
        bindings.append(f'    py::enum_<{enum_name}>(m, "{enum_info["name"]}")')
        for constant in enum_info['constants']:
            bindings.append(f'        .value("{constant["name"]}", {enum_name}::{constant["name"]})')
        bindings.append('        .export_values();')

    def generate_namespace_binding(namespace_info):
        bindings.append(f'    py::module_ {namespace_info["name"]} = m.def_submodule("{namespace_info["name"]}");')
        for child in namespace_info['children']:
            if child['type'] == 'function':
                generate_function_binding(child, namespace_info['name'])
            elif child['type'] == 'class':
                generate_class_binding(child, namespace_info['name'])
            elif child['type'] == 'enum':
                generate_enum_binding(child, namespace_info['name'])
            elif child['type'] == 'namespace':
                generate_namespace_binding(child)

    for ast_info in ast_info_list:
        for child in ast_info['children']:
            if child['type'] == 'function':
                generate_function_binding(child)
            elif child['type'] == 'class':
                generate_class_binding(child)
            elif child['type'] == 'enum':
                generate_enum_binding(child)
            elif child['type'] == 'namespace':
                generate_namespace_binding(child)

    bindings.append('}')
    with open(bindings_file, 'w') as file:
        file.write('\n'.join(bindings))
    print(f"pybind11 bindings written to file: {bindings_file}")

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python generator.py <directory> <output_file> <bindings_file>")
        sys.exit(1)

    directory = sys.argv[1]
    output_file = sys.argv[2]
    bindings_file = sys.argv[3]

    ast_info_list = parse_hpp_files(directory)
    generate_ast_json(directory, output_file)
    generate_pybind11_bindings(ast_info_list, bindings_file)
