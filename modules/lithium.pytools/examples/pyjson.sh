# Example 1: Minify JSON output
# This example minifies the JSON output from the specified file.
$ python pyjson.py example.json --minify

# Example 2: Format JSON output with indentation
# This example formats the JSON output from the specified file with indentation.
$ python pyjson.py example.json --format

# Example 3: Convert JSON to YAML and save to a file
# This example converts the JSON data to YAML format and saves it to the specified output file.
$ python pyjson.py example.json --yaml output.yaml

# Example 4: Query JSON data using dot notation
# This example queries the JSON data using a dot notation path.
$ python pyjson.py example.json --query a.b.0.c

# Example 5: Validate JSON file format
# This example validates the format of the specified JSON file.
$ python pyjson.py example.json --validate

# Example 6: Merge multiple JSON files
# This example merges the specified JSON files into one.
$ python pyjson.py --merge file1.json file2.json file3.json

# Example 7: Compare two JSON files
# This example compares the specified JSON files and displays the differences.
$ python pyjson.py --diff file1.json file2.json

# Example 8: Export JSON to XML format
# This example exports the JSON data to XML format and saves it to the specified output file.
$ python pyjson.py example.json --export-xml output.xml

# Example 9: Display statistics about the JSON structure
# This example displays statistics about the structure of the JSON data.
$ python pyjson.py example.json --stats

# Example 10: Flatten nested JSON structures
# This example flattens the nested JSON structures in the specified file.
$ python pyjson.py example.json --flatten

# Example 11: Unflatten JSON structures
# This example unflattens the JSON structures in the specified file.
$ python pyjson.py example.json --unflatten

# Example 12: Remove a specific key from the JSON data
# This example removes the specified key from the JSON data.
$ python pyjson.py example.json --remove-key key_to_remove

# Example 13: Rename a key in the JSON data
# This example renames the specified key in the JSON data.
$ python pyjson.py example.json --rename-key old_key new_key