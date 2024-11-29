# Example 1: Generate PyBind11 bindings for an executable with default settings
# This example generates PyBind11 bindings for the specified executable and saves the output in the default directory 'bindings'.
$ python3 exebind.py /path/to/executable

# Example 2: Generate PyBind11 bindings with a custom module name
# This example generates PyBind11 bindings for the specified executable with a custom module name 'custom_module'.
$ python3 exebind.py /path/to/executable --module-name custom_module

# Example 3: Generate PyBind11 bindings and save to a custom output directory
# This example generates PyBind11 bindings for the specified executable and saves the output in the specified directory 'custom_bindings'.
$ python3 exebind.py /path/to/executable --output-dir custom_bindings

# Example 4: Generate PyBind11 bindings with a custom module name and save to a custom output directory
# This example generates PyBind11 bindings for the specified executable with a custom module name 'custom_module' and saves the output in the specified directory 'custom_bindings'.
$ python3 exebind.py /path/to/executable --module-name custom_module --output-dir custom_bindings