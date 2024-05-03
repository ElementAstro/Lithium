import os
import clang.cindex
from clang.cindex import CursorKind
import json

def parse_namespace(cursor):
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
    ast_info_list = []

    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(".hpp"):
                file_path = os.path.join(root, file)
                ast_info = parse_cpp_file(file_path)
                ast_info_list.append(ast_info)

    return ast_info_list

def generate_ast_json(directory, output_file):
    ast_info_list = parse_hpp_files(directory)
    json_output = json.dumps(ast_info_list, indent=2)

    with open(output_file, 'w') as file:
        file.write(json_output)

    print(f"AST信息已写入文件: {output_file}")

file_path = "example.cpp"
ast_info = parse_cpp_file(file_path)
json_output = json.dumps(ast_info, indent=2)
print(json_output)