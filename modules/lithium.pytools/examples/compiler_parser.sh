# Example 1: Parse GCC compiler output and convert to JSON format
# This example parses the GCC compiler output from the specified file and converts it to JSON format.
# The output will be saved to output.json in the current directory.
$ python3 compiler_parser.py gcc gcc_output.txt --output-format json --output-file output.json

# Example 2: Parse Clang compiler output and convert to CSV format
# This example parses the Clang compiler output from the specified file and converts it to CSV format.
# The output will be saved to output.csv in the current directory.
$ python3 compiler_parser.py clang clang_output.txt --output-format csv --output-file output.csv

# Example 3: Parse MSVC compiler output and convert to XML format
# This example parses the MSVC compiler output from the specified file and converts it to XML format.
# The output will be saved to output.xml in the current directory.
$ python3 compiler_parser.py msvc msvc_output.txt --output-format xml --output-file output.xml

# Example 4: Parse CMake output and convert to JSON format
# This example parses the CMake output from the specified file and converts it to JSON format.
# The output will be saved to cmake_output.json in the current directory.
$ python3 compiler_parser.py cmake cmake_output.txt --output-format json --output-file cmake_output.json

# Example 5: Parse GCC compiler output and filter by errors only
# This example parses the GCC compiler output from the specified file, filters by errors only, and converts it to JSON format.
# The output will be saved to gcc_errors.json in the current directory.
$ python3 compiler_parser.py gcc gcc_output.txt --output-format json --output-file gcc_errors.json --filter error

# Example 6: Parse Clang compiler output and include statistics in the output
# This example parses the Clang compiler output from the specified file, includes statistics in the output, and converts it to JSON format.
# The output will be saved to clang_output_with_stats.json in the current directory.
$ python3 compiler_parser.py clang clang_output.txt --output-format json --output-file clang_output_with_stats.json --stats

# Example 7: Parse MSVC compiler output with concurrency
# This example parses the MSVC compiler output from multiple files concurrently and converts it to JSON format.
# The output will be saved to msvc_output.json in the current directory.
$ python3 compiler_parser.py msvc msvc_output1.txt msvc_output2.txt --output-format json --output-file msvc_output.json --concurrency 8

# Example 8: Parse CMake output and save to a specific directory
# This example parses the CMake output from the specified file and converts it to JSON format.
# The output will be saved to the specified output directory.
$ python3 compiler_parser.py cmake cmake_output.txt --output-format json --output-file cmake_output.json --output-dir output_dir

# Example 9: Parse GCC compiler output and convert to CSV format with filtering by warnings and errors
# This example parses the GCC compiler output from the specified file, filters by warnings and errors, and converts it to CSV format.
# The output will be saved to gcc_warnings_errors.csv in the current directory.
$ python3 compiler_parser.py gcc gcc_output.txt --output-format csv --output-file gcc_warnings_errors.csv --filter warning error

# Example 10: Parse Clang compiler output and display colorized output in the console
# This example parses the Clang compiler output from the specified file and displays the results with colorized output in the console.
$ python3 compiler_parser.py clang clang_output.txt --output-format json --output-file clang_output.json