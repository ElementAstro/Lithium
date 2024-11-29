# Example 1: Install project dependencies using npm
# This example installs the project dependencies using npm.
$ python jbuilder.py --package_manager npm --project_dir /path/to/project --install

# Example 2: Build the project using yarn
# This example builds the project using yarn.
$ python jbuilder.py --package_manager yarn --project_dir /path/to/project --build

# Example 3: Clean the project by removing node_modules
# This example cleans the project by removing the node_modules directory.
$ python jbuilder.py --package_manager npm --project_dir /path/to/project --clean

# Example 4: Run tests for the project using npm
# This example runs the tests for the project using npm.
$ python jbuilder.py --package_manager npm --project_dir /path/to/project --test

# Example 5: Lint the project code using yarn
# This example lints the project code using yarn.
$ python jbuilder.py --package_manager yarn --project_dir /path/to/project --lint

# Example 6: Format the project code using npm
# This example formats the project code using npm.
$ python jbuilder.py --package_manager npm --project_dir /path/to/project --format

# Example 7: Start the development server using yarn
# This example starts the development server using yarn.
$ python jbuilder.py --package_manager yarn --project_dir /path/to/project --start

# Example 8: Generate documentation using npm
# This example generates the project documentation using npm.
$ python jbuilder.py --package_manager npm --project_dir /path/to/project --generate_docs

# Example 9: List available npm scripts from package.json
# This example lists all available npm scripts from the package.json file.
$ python jbuilder.py --package_manager npm --project_dir /path/to/project --list_scripts

# Example 10: Build the project with custom build options using yarn
# This example builds the project using yarn with custom build options.
$ python jbuilder.py --package_manager yarn --project_dir /path/to/project --build --build_options --verbose --production

# Example 11: Load build options from a configuration file and build the project using npm
# This example loads build options from a specified configuration file and builds the project using npm.
$ python jbuilder.py --package_manager npm --project_dir /path/to/project --config /path/to/config.json --build