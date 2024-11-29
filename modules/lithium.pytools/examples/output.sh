# Example 1: Pretty print a JSON file
# This example pretty prints the contents of a JSON file.
$ python output.py example.json

# Example 2: Validate a YAML file
# This example validates the syntax of a YAML file without printing its contents.
$ python output.py example.yaml --validate

# Example 3: Pretty print multiple files with overwrite
# This example pretty prints the contents of multiple files and overwrites the output files if they exist.
$ python output.py example1.json example2.yaml --overwrite

# Example 4: Read from standard input
# This example reads data from standard input and pretty prints it as TOML.
$ cat example.toml | python output.py - --format toml

# Example 5: Pretty print a TOML file with custom indentation
# This example pretty prints the contents of a TOML file with a custom indentation level.
$ python output.py example.toml --indent 2

# Example 6: Pretty print an XML file and write to an output file
# This example pretty prints the contents of an XML file and writes the output to a specified file.
$ python output.py example.xml --output output.xml

# Example 7: Pretty print a CSV file
# This example pretty prints the contents of a CSV file.
$ python output.py example.csv

# Example 8: Pretty print an INI file
# This example pretty prints the contents of an INI file.
$ python output.py example.ini

# Example 9: Enable verbose logging
# This example enables verbose logging for detailed output.
$ python output.py example.json --verbose

# Example 10: Pretty print a JSON file with a custom theme
# This example pretty prints the contents of a JSON file using a custom syntax highlighting theme.
$ python output.py example.json --theme dracula