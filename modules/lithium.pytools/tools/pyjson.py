import json
import argparse
import sys
from json.decoder import JSONDecodeError

try:
    import yaml
except ImportError:
    # If PyYAML is not installed, handle this case to maintain usability for other features.
    yaml = None

def load_json(file_path: str) -> dict:
    """
    Load JSON data from a file and return it as a dictionary.

    Args:
        file_path (str): Path to the JSON file.

    Returns:
        dict: The parsed JSON data as a Python dictionary.

    Raises:
        SystemExit: If the file is not found, cannot be read, or contains invalid JSON.
    """
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            return json.load(file)
    except JSONDecodeError as e:
        print(f"Invalid JSON: {e}")
        sys.exit(1)
    except FileNotFoundError:
        print(f"File not found: {file_path}")
        sys.exit(1)
    except IOError as e:
        print(f"Error reading file: {e}")
        sys.exit(1)

def print_json(obj: dict, minify: bool = False, indent: int = 4):
    """
    Print a JSON object as a formatted or minified string.

    Args:
        obj (dict): The JSON object to print.
        minify (bool, optional): Whether to print the JSON in minified form. Defaults to False.
        indent (int, optional): Indentation level for pretty-printing. Defaults to 4.

    Returns:
        None
    """
    if minify:
        print(json.dumps(obj, separators=(',', ':')))
    else:
        print(json.dumps(obj, indent=indent))

def save_json_to_yaml(json_obj: dict, output_file: str):
    """
    Save a JSON object to a YAML file.

    Args:
        json_obj (dict): The JSON object to convert to YAML.
        output_file (str): Path to the output YAML file.

    Returns:
        None

    Raises:
        ImportError: If PyYAML is not installed.
    """
    if yaml is None:
        print("YAML support is not available. Install PyYAML to enable this feature.")
        return
    with open(output_file, 'w', encoding='utf-8') as file:
        yaml.dump(json_obj, file, allow_unicode=True, default_flow_style=False)

def query_json(json_obj: dict, query_path: str):
    """
    Query a JSON object using a simple dot notation path.

    Args:
        json_obj (dict): The JSON object to query.
        query_path (str): Dot notation path to the desired data (e.g., 'a.b.0.c').

    Returns:
        None

    Raises:
        KeyError, IndexError, TypeError: If the query path is invalid or the data cannot be accessed.
    """
    try:
        parts = query_path.strip().split('.')
        result = json_obj
        for part in parts:
            if part.isdigit():
                result = result[int(part)]
            else:
                result = result[part]
        print_json(result)
    except (KeyError, IndexError, TypeError) as e:
        print(f"Failed to query JSON with path '{query_path}': {e}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Enhanced JSON processing tool.")
    parser.add_argument('file', type=str, help="JSON file to process")
    parser.add_argument('--minify', action='store_true', help="Minify JSON output")
    parser.add_argument('--format', action='store_true', help="Format JSON output with indentation")
    parser.add_argument('--yaml', type=str, metavar="OUTPUT_FILE", help="Convert JSON to YAML and save to a file")
    parser.add_argument('--query', type=str, metavar="QUERY_PATH", help="Query JSON data using simple dot notation (e.g., 'a.b.0.c')")

    args = parser.parse_args()

    # Load JSON data from the specified file.
    data = load_json(args.file)

    if args.yaml:
        save_json_to_yaml(data, args.yaml)
    elif args.query:
        query_json(data, args.query)
    else:
        print_json(data, minify=args.minify, indent=None if args.minify else 4)
