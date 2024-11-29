# Example 1: Check if a specific package is installed
# This example checks if the specified package is installed.
$ python package.py --check requests

# Example 2: Install a specific package
# This example installs the specified package.
$ python package.py --install requests

# Example 3: Install a specific version of a package
# This example installs the specified version of the package.
$ python package.py --install requests --version 2.25.1

# Example 4: Upgrade a specific package to the latest version
# This example upgrades the specified package to the latest version.
$ python package.py --upgrade requests

# Example 5: Uninstall a specific package
# This example uninstalls the specified package.
$ python package.py --uninstall requests

# Example 6: List all installed packages
# This example lists all installed Python packages.
$ python package.py --list-installed

# Example 7: Generate a requirements.txt file
# This example generates a requirements.txt file for the current environment.
$ python package.py --freeze

# Example 8: Generate a requirements.txt file with a custom output file name
# This example generates a requirements.txt file and writes it to the specified output file.
$ python package.py --freeze custom_requirements.txt

# Example 9: Check for updates based on a requirements.txt file
# This example checks for updates for packages listed in the requirements.txt file.
$ python package.py --check-updates

# Example 10: Check for updates based on a custom requirements file
# This example checks for updates for packages listed in the specified requirements file.
$ python package.py --check-updates custom_requirements.txt