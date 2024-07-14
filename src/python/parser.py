import ast
import json
import argparse

class PythonToScriptConverter(ast.NodeVisitor):
    def __init__(self):
        self.script = []

    def visit_Module(self, node):
        """Visit the module body and process each statement."""
        for stmt in node.body:
            self.visit(stmt)

    def visit_FunctionDef(self, node):
        """Handle function definitions."""
        steps = self.get_steps(node.body)
        self.script.append({"type": "function", "name": node.name, "steps": steps})

    def visit_Expr(self, node):
        """Handle expressions, especially function calls."""
        if isinstance(node.value, ast.Call):
            self.handle_function_call(node.value)
        else:
            self.generic_visit(node)

    def visit_Assign(self, node):
        """Handle assignment statements."""
        target = node.targets[0].id
        value = self.get_value(node.value)
        self.script.append({"type": "assign", "variable": target, "value": value})

    def visit_AugAssign(self, node):
        """Handle augmented assignment statements."""
        target = node.target.id
        value = self.get_value(node.value)
        op = self.get_operator(node.op)
        self.script.append({"type": "assign", "variable": target, "value": {"operation": op, "left": target, "right": value}})

    def visit_If(self, node):
        """Handle if statements."""
        cond = self.get_condition(node.test)
        true_steps = self.get_steps(node.body)
        false_steps = self.get_steps(node.orelse)
        self.script.append({"type": "condition", "condition": cond, "true": true_steps, "false": false_steps})

    def visit_While(self, node):
        """Handle while loops."""
        cond = self.get_condition(node.test)
        steps = self.get_steps(node.body)
        self.script.append({"type": "loop", "condition": cond, "steps": steps})

    def visit_For(self, node):
        """Handle for loops."""
        target = node.target.id
        iter = self.get_value(node.iter)
        steps = self.get_steps(node.body)
        self.script.append({"type": "loop", "variable": target, "iterations": iter, "steps": steps})

    def visit_Break(self, node):
        """Handle break statements."""
        self.script.append({"type": "break"})

    def visit_Continue(self, node):
        """Handle continue statements."""
        self.script.append({"type": "continue"})

    def visit_Import(self, node):
        """Handle import statements."""
        for alias in node.names:
            self.script.append({"type": "import", "module": alias.name})

    def visit_ImportFrom(self, node):
        """Handle from ... import statements."""
        module = node.module
        for alias in node.names:
            self.script.append({"type": "import", "module": module, "name": alias.name})

    def handle_function_call(self, node):
        """Handle function calls."""
        function_name = node.func.id
        params = {arg.arg: self.get_value(arg.value) for arg in node.keywords}
        self.script.append({"type": "call", "function": function_name, "params": params})

    def get_steps(self, nodes):
        """Process a list of statements and return their steps."""
        converter = PythonToScriptConverter()
        for node in nodes:
            converter.visit(node)
        return converter.script

    def get_value(self, node):
        """Retrieve the value from a node."""
        if isinstance(node, ast.Constant):
            return node.value
        elif isinstance(node, ast.Name):
            return node.id
        elif isinstance(node, ast.BinOp):
            return {"operation": self.get_operator(node.op), "left": self.get_value(node.left), "right": self.get_value(node.right)}
        elif isinstance(node, ast.List):
            return [self.get_value(elem) for elem in node.elts]
        elif isinstance(node, ast.Dict):
            return {self.get_value(k): self.get_value(v) for k, v in zip(node.keys, node.values)}
        elif isinstance(node, ast.Call):
            return self.handle_function_call(node)
        return None

    def get_condition(self, node):
        """Retrieve the condition from a node."""
        if isinstance(node, ast.Compare):
            left = self.get_value(node.left)
            right = self.get_value(node.comparators[0])
            operation = self.get_operator(node.ops[0])
            return {"operation": operation, "left": left, "right": right}
        return None

    def get_operator(self, node):
        """Retrieve the operator from a node."""
        if isinstance(node, ast.Add):
            return "+"
        elif isinstance(node, ast.Sub):
            return "-"
        elif isinstance(node, ast.Mult):
            return "*"
        elif isinstance(node, ast.Div):
            return "/"
        elif isinstance(node, ast.Eq):
            return "=="
        elif isinstance(node, ast.NotEq):
            return "!="
        elif isinstance(node, ast.Lt):
            return "<"
        elif isinstance(node, ast.LtE):
            return "<="
        elif isinstance(node, ast.Gt):
            return ">"
        elif isinstance(node, ast.GtE):
            return ">="
        return None

def convert_python_to_script(python_code):
    """Convert Python code to a JSON script."""
    tree = ast.parse(python_code)
    converter = PythonToScriptConverter()
    converter.visit(tree)
    return json.dumps(converter.script, indent=4)

def main():
    parser = argparse.ArgumentParser(description='Convert Python code to a JSON script.')
    parser.add_argument('input', help='Input Python file')
    parser.add_argument('output', help='Output JSON file')
    args = parser.parse_args()

    with open(args.input, 'r', encoding="utf-8") as f:
        python_code = f.read()

    script_json = convert_python_to_script(python_code)

    with open(args.output, 'w', encoding="utf-8") as f:
        f.write(script_json)

if __name__ == '__main__':
    main()
