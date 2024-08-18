#!/usr/bin/env python3
"""
Python to Custom Script Converter

This tool converts Python code into a custom script format defined by the user.
It utilizes Python's Abstract Syntax Tree (AST) to parse the code and translate it
into the custom format. The tool supports command-line usage and batch processing of files.

Features:
- Conversion of Python constructs like functions, loops, conditionals, and assignments.
- Error reporting with line and column numbers.
- Symbol table management for variable and function scoping.
- Support for command-line arguments for file processing.
- Batch processing for multiple files.
- Performance optimizations to handle large codebases efficiently.

Usage:
    python converter.py <input_file.py> [-o output.json] [--batch dir]

    - `input_file.py`: The Python file to be converted.
    - `-o output.json`: The output file to save the converted script (optional).
    - `--batch dir`: Directory containing Python files to convert in batch mode (optional).
"""

import ast
import json
import os
import argparse
from typing import Any, Dict, List, Optional, Union


class CompilerError(Exception):
    """Custom exception for handling compilation errors."""
    pass


class PythonToCustomScriptConverter(ast.NodeVisitor):
    """
    AST Node Visitor that converts Python code into a custom script format.

    Attributes:
        script (List[Dict[str, Any]]): The resulting custom script as a list of instructions.
        errors (List[str]): List of error messages encountered during conversion.
        symbol_table (Dict[str, Union[str, ast.AST]]): Stores variable and function definitions.
        current_scope (List[Dict[str, Union[str, ast.AST]]]): Stack of scopes for managing variable visibility.
        max_recursion_depth (int): Maximum allowed recursion depth to prevent stack overflow.
    """

    def __init__(self) -> None:
        self.script: List[Dict[str, Any]] = []
        self.errors: List[str] = []
        self.symbol_table: Dict[str, Union[str, ast.AST]] = {}
        self.current_scope: List[Dict[str, Union[str, ast.AST]]] = [{}]
        self.max_recursion_depth: int = 1000  # Maximum recursion depth limit

    def report_error(self, message: str, node: Optional[ast.AST] = None) -> None:
        """
        Report an error with an optional node to indicate where the error occurred.

        Args:
            message (str): The error message.
            node (Optional[ast.AST]): The AST node where the error occurred.
        """
        if node and hasattr(node, 'lineno'):
            message = f"{message} at line {node.lineno}, column {node.col_offset}"
        self.errors.append(message)

    def enter_scope(self) -> None:
        """Enter a new scope by pushing a new dictionary onto the scope stack."""
        self.current_scope.append({})

    def exit_scope(self) -> None:
        """Exit the current scope by popping the top dictionary off the scope stack."""
        self.current_scope.pop()

    def add_to_symbol_table(self, name: str, type: Union[str, ast.AST], node: ast.AST) -> None:
        """
        Add a new symbol to the current scope in the symbol table.

        Args:
            name (str): The name of the symbol (variable or function).
            type (Union[str, ast.AST]): The type or node associated with the symbol.
            node (ast.AST): The AST node representing the symbol's declaration.
        """
        if name in self.current_scope[-1]:
            self.report_error(f"Redefinition of {name}", node)
        self.current_scope[-1][name] = type

    def lookup_symbol_table(self, name: str) -> Optional[Union[str, ast.AST]]:
        """
        Lookup a symbol in the symbol table, searching through all scopes.

        Args:
            name (str): The name of the symbol to lookup.

        Returns:
            Optional[Union[str, ast.AST]]: The type or node associated with the symbol, or None if not found.
        """
        for scope in reversed(self.current_scope):
            if name in scope:
                return scope[name]
        return None

    def visit_Module(self, node: ast.Module) -> List[Dict[str, Any]]:
        """
        Visit a module node and process all its statements.

        Args:
            node (ast.Module): The module node to visit.

        Returns:
            List[Dict[str, Any]]: The script generated from the module.
        """
        for stmt in node.body:
            self.visit(stmt)
        return self.script

    def visit_Assign(self, node: ast.Assign) -> None:
        """
        Visit an assignment node and convert it to a custom script assignment.

        Args:
            node (ast.Assign): The assignment node to visit.
        """
        try:
            target = node.targets[0]
            if isinstance(target, ast.Name):
                value = self.visit(node.value)
                assignment = {
                    "type": "assign",
                    "variable": target.id,
                    "value": value
                }
                self.script.append(assignment)
                self.add_to_symbol_table(target.id, type(value), node)
        except Exception as e:
            self.report_error(f"Error processing assignment: {e}", node)

    def visit_AugAssign(self, node: ast.AugAssign) -> None:
        """
        Visit an augmented assignment node (e.g., x += 1) and convert it to a custom script assignment.

        Args:
            node (ast.AugAssign): The augmented assignment node to visit.
        """
        try:
            target = node.target
            if isinstance(target, ast.Name):
                op = self.visit(node.op)
                value = self.visit(node.value)
                assignment = {
                    "type": "assign",
                    "variable": target.id,
                    "value": {
                        f"${op}": ["$", target.id, value]
                    }
                }
                self.script.append(assignment)
        except Exception as e:
            self.report_error(
                f"Error processing augmented assignment: {e}", node)

    def visit_BinOp(self, node: ast.BinOp) -> Optional[Dict[str, Any]]:
        """
        Visit a binary operation node (e.g., a + b) and convert it to a custom script operation.

        Args:
            node (ast.BinOp): The binary operation node to visit.

        Returns:
            Optional[Dict[str, Any]]: The custom script representation of the binary operation.
        """
        try:
            op = self.visit(node.op)
            left = self.visit(node.left)
            right = self.visit(node.right)
            return {
                f"${op}": [left, right]
            }
        except Exception as e:
            self.report_error(f"Error processing binary operation: {e}", node)
            return None

    def visit_Name(self, node: ast.Name) -> str:
        """
        Visit a name node (variable) and return its custom script representation.

        Args:
            node (ast.Name): The name node to visit.

        Returns:
            str: The custom script representation of the name.
        """
        symbol_type = self.lookup_symbol_table(node.id)
        if symbol_type is None:
            self.report_error(f"Use of undeclared variable {node.id}", node)
        return f"${node.id}"

    def visit_Constant(self, node: ast.Constant) -> Any:
        """
        Visit a constant node and return its value.

        Args:
            node (ast.Constant): The constant node to visit.

        Returns:
            Any: The value of the constant.
        """
        return node.value

    def visit_FunctionDef(self, node: ast.FunctionDef) -> None:
        """
        Visit a function definition node and convert it to a custom script function.

        Args:
            node (ast.FunctionDef): The function definition node to visit.
        """
        try:
            self.add_to_symbol_table(node.name, "function", node)
            func = {
                "type": "function_def",
                "name": node.name,
                "params": [arg.arg for arg in node.args.args],
                "steps": []
            }
            self.enter_scope()
            for param in node.args.args:
                self.add_to_symbol_table(param.arg, "parameter", node)
            for stmt in node.body:
                func["steps"].append(self.visit(stmt))
            self.exit_scope()
            self.script.append(func)
        except Exception as e:
            self.report_error(
                f"Error processing function definition '{node.name}': {e}", node)

    def visit_Return(self, node: ast.Return) -> Optional[Dict[str, Any]]:
        """
        Visit a return statement node and convert it to a custom script return.

        Args:
            node (ast.Return): The return statement node to visit.

        Returns:
            Optional[Dict[str, Any]]: The custom script representation of the return statement.
        """
        try:
            return {
                "type": "return",
                "value": self.visit(node.value)
            }
        except Exception as e:
            self.report_error(f"Error processing return statement: {e}", node)
            return None

    def visit_If(self, node: ast.If) -> None:
        """
        Visit an if statement node and convert it to a custom script conditional.

        Args:
            node (ast.If): The if statement node to visit.
        """
        try:
            condition = self.visit(node.test)
            true_branch: List[Dict[str, Any]] = []
            false_branch: List[Dict[str, Any]] = []

            for stmt in node.body:
                true_branch.append(self.visit(stmt))
            for stmt in node.orelse:
                false_branch.append(self.visit(stmt))

            if_statement = {
                "type": "condition",
                "condition": condition,
                "true": true_branch[0] if len(true_branch) == 1 else {"steps": true_branch},
                "false": false_branch[0] if len(false_branch) == 1 else {"steps": false_branch} if false_branch else None
            }
            self.script.append(if_statement)
        except Exception as e:
            self.report_error(f"Error processing if statement: {e}", node)

    def visit_Compare(self, node: ast.Compare) -> Optional[Dict[str, Any]]:
        """
        Visit a comparison node and convert it to a custom script comparison.

        Args:
            node (ast.Compare): The comparison node to visit.

        Returns:
            Optional[Dict[str, Any]]: The custom script representation of the comparison.
        """
        try:
            left = self.visit(node.left)
            right = self.visit(node.comparators[0])
            op = self.visit(node.ops[0])
            return {
                f"${op}": [left, right]
            }
        except Exception as e:
            self.report_error(f"Error processing comparison: {e}", node)
            return None

    def visit_While(self, node: ast.While) -> None:
        """
        Visit a while loop node and convert it to a custom script loop.

        Args:
            node (ast.While): The while loop node to visit.
        """
        try:
            condition = self.visit(node.test)
            steps: List[Dict[str, Any]] = []
            self.enter_scope()
            for stmt in node.body:
                steps.append(self.visit(stmt))
            self.exit_scope()
            while_statement = {
                "type": "while",
                "condition": condition,
                "steps": steps
            }
            self.script.append(while_statement)
        except Exception as e:
            self.report_error(f"Error processing while loop: {e}", node)

    def visit_For(self, node: ast.For) -> None:
        """
        Visit a for loop node and convert it to a custom script loop.

        Args:
            node (ast.For): The for loop node to visit.
        """
        try:
            target = self.visit(node.target)
            iter_ = self.visit(node.iter)
            steps: List[Dict[str, Any]] = []
            self.enter_scope()
            for stmt in node.body:
                steps.append(self.visit(stmt))
            self.exit_scope()
            for_statement = {
                "type": "for",
                "variable": target,
                "iterable": iter_,
                "steps": steps
            }
            self.script.append(for_statement)
        except Exception as e:
            self.report_error(f"Error processing for loop: {e}", node)

    def visit_Call(self, node: ast.Call) -> Optional[Dict[str, Any]]:
        """
        Visit a function call node and convert it to a custom script function call.

        Args:
            node (ast.Call): The function call node to visit.

        Returns:
            Optional[Dict[str, Any]]: The custom script representation of the function call.
        """
        try:
            func_name = self.visit(node.func)
            args = [self.visit(arg) for arg in node.args]
            if len(self.current_scope) > self.max_recursion_depth:
                self.report_error(
                    f"Max recursion depth exceeded in function call to {func_name}", node)
                return None
            return {
                "type": "call",
                "function": func_name[1:],  # Remove the $ prefix
                "params": args
            }
        except Exception as e:
            self.report_error(f"Error processing function call: {e}", node)
            return None

    def visit_List(self, node: ast.List) -> Optional[List[Any]]:
        """
        Visit a list node and convert it to a list of custom script elements.

        Args:
            node (ast.List): The list node to visit.

        Returns:
            Optional[List[Any]]: The custom script representation of the list.
        """
        try:
            return [self.visit(elem) for elem in node.elts]
        except Exception as e:
            self.report_error(f"Error processing list: {e}", node)
            return None

    def visit_Dict(self, node: ast.Dict) -> Optional[Dict[Any, Any]]:
        """
        Visit a dictionary node and convert it to a custom script dictionary.

        Args:
            node (ast.Dict): The dictionary node to visit.

        Returns:
            Optional[Dict[Any, Any]]: The custom script representation of the dictionary.
        """
        try:
            keys = [self.visit(key) for key in node.keys]
            values = [self.visit(value) for value in node.values]
            return dict(zip(keys, values))
        except Exception as e:
            self.report_error(f"Error processing dictionary: {e}", node)
            return None

    # Basic operation visitors
    def visit_Add(self, node: ast.Add) -> str:
        return "add"

    def visit_Sub(self, node: ast.Sub) -> str:
        return "sub"

    def visit_Mult(self, node: ast.Mult) -> str:
        return "mul"

    def visit_Div(self, node: ast.Div) -> str:
        return "div"

    def visit_Eq(self, node: ast.Eq) -> str:
        return "eq"

    def visit_NotEq(self, node: ast.NotEq) -> str:
        return "neq"

    def visit_Lt(self, node: ast.Lt) -> str:
        return "lt"

    def visit_LtE(self, node: ast.LtE) -> str:
        return "lte"

    def visit_Gt(self, node: ast.Gt) -> str:
        return "gt"

    def visit_GtE(self, node: ast.GtE) -> str:
        return "gte"

    def visit(self, node: ast.AST) -> Any:
        """
        Visit a node in the AST and return its custom script representation.

        Args:
            node (ast.AST): The AST node to visit.

        Returns:
            Any: The custom script representation of the node.
        """
        try:
            return super().visit(node)
        except AttributeError:
            self.report_error(f"Unsupported syntax: {ast.dump(node)}", node)
            return None

    def convert(self, code: str) -> str:
        """
        Convert the given Python code into a custom script.

        Args:
            code (str): The Python code to convert.

        Returns:
            str: The resulting custom script as a JSON string.
        """
        try:
            tree = ast.parse(code)
            self.visit(tree)
            if self.errors:
                print("Errors encountered during conversion:")
                for error in self.errors:
                    print(f"  - {error}")
            else:
                print("Conversion successful!")
            return json.dumps(self.script, indent=4)
        except Exception as e:
            raise CompilerError(f"Failed to compile script: {e}")


def process_file(file_path: str, output_path: Optional[str] = None) -> None:
    """
    Process a single Python file and convert it to a custom script.

    Args:
        file_path (str): Path to the input Python file.
        output_path (Optional[str]): Path to the output JSON file. If None, prints to stdout.
    """
    with open(file_path, 'r') as file:
        code = file.read()
    converter = PythonToCustomScriptConverter()
    custom_script = converter.convert(code)

    if output_path:
        with open(output_path, 'w') as out_file:
            out_file.write(custom_script)
        print(f"Converted script saved to {output_path}")
    else:
        print(custom_script)


def batch_process_directory(directory: str) -> None:
    """
    Batch process all Python files in a directory.

    Args:
        directory (str): Path to the directory containing Python files.
    """
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith('.py'):
                file_path = os.path.join(root, file)
                output_path = file_path.replace('.py', '.json')
                process_file(file_path, output_path)


def main() -> None:
    """
    Main entry point for the command-line tool.
    """
    parser = argparse.ArgumentParser(
        description="Convert Python files to custom script format.")
    parser.add_argument(
        'input', help="The input Python file or directory for batch processing.")
    parser.add_argument(
        '-o', '--output', help="The output file path for single file conversion.")
    parser.add_argument('--batch', action='store_true',
                        help="Batch process all Python files in a directory.")

    args = parser.parse_args()

    if args.batch:
        batch_process_directory(args.input)
    else:
        process_file(args.input, args.output)


if __name__ == "__main__":
    main()
