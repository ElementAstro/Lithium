#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
C++ AST Generator

This script generates the Abstract Syntax Tree (AST) for C++ header files (.hpp)
in a specified directory and its subdirectories. The AST information is exported
to a JSON file.

Usage:
    python generator.py <directory> <output_file>

Dependencies:
    - clang
    - libclang
"""

import os
import sys
import clang.cindex
from clang.cindex import CursorKind
import json

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
        "children": []
    }
    for child in cursor.get_children():
        child_info = parse_cursor(child)
        if child_info:
            class_info["children"].append(child_info)
    return class_info

def parse_struct(cursor):
    struct_info = {
        "type": "struct",
        "name": cursor.spelling,
        "children": []
    }
    for child in cursor.get_children():
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

def parse_comment(cursor : clang.cindex.Cursor):
    comment_info = None
    if not cursor.brief_comment:
        comment_info = {
            "type": "comment",
            "text": cursor.brief_comment
        }
        return comment_info
    if cursor.raw_comment:
        comment_info = {
            "type": "comment",
            "text": cursor.raw_comment
        }
        return comment_info
    for child in cursor.get_children():
        parse_comment(child)

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
    elif cursor.kind == CursorKind.CLASS_DECL:
        return parse_class(cursor)
    elif cursor.kind == CursorKind.STRUCT_DECL:
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
    translation_unit = index.parse(file_path, args=['-x', 'c++'])
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

    print(f"AST信息已写入文件: {output_file}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python generator.py <directory> <output_file>")
        sys.exit(1)

    directory = sys.argv[1]
    output_file = sys.argv[2]
    generate_ast_json(directory, output_file)