# Example 1: Display all symbols in a binary
# This example displays all symbols in the specified binary file.
$ python nm.py /path/to/binary

# Example 2: Filter symbols by type
# This example filters symbols by the specified type (e.g., 'T' for text symbols).
$ python nm.py /path/to/binary --filter T

# Example 3: Search for a specific symbol by name
# This example searches for a specific symbol by its name.
$ python nm.py /path/to/binary --search symbol_name

# Example 4: Find a symbol by address
# This example finds a symbol by its address.
$ python nm.py /path/to/binary --address 00400000

# Example 5: Display detailed output for symbols
# This example displays detailed output for all symbols in the specified binary file.
$ python nm.py /path/to/binary --detailed

# Example 6: Count symbols by type
# This example counts the number of symbols by their type.
$ python nm.py /path/to/binary --count

# Example 7: Export symbols to a file in TXT format
# This example exports all symbols to a file in TXT format.
$ python nm.py /path/to/binary --export symbols.txt

# Example 8: Export symbols to a file in CSV format
# This example exports all symbols to a file in CSV format.
$ python nm.py /path/to/binary --export symbols.csv

# Example 9: Export symbols to a file in JSON format
# This example exports all symbols to a file in JSON format.
$ python nm.py /path/to/binary --export symbols.json

# Example 10: Export symbols to a file in XML format
# This example exports all symbols to a file in XML format.
$ python nm.py /path/to/binary --export symbols.xml

# Example 11: Display only external symbols
# This example displays only external symbols in the specified binary file.
$ python nm.py /path/to/binary --extern

# Example 12: Demangle C++ symbol names
# This example demangles C++ symbol names in the specified binary file.
$ python nm.py /path/to/binary --demangle

# Example 13: Display symbols with sizes
# This example displays symbols along with their sizes in the specified binary file.
$ python nm.py /path/to/binary --size

# Example 14: Search symbols matching a regex pattern
# This example searches for symbols matching the specified regex pattern.
$ python nm.py /path/to/binary --pattern "regex_pattern"

# Example 15: Increase output verbosity
# This example increases the output verbosity for detailed logging.
$ python nm.py /path/to/binary --verbose