# Example 1: Configure and build a project using CMake with default settings
# This example configures and builds the project located in the 'src' directory,
# using the 'build' directory for build files, and the default CMake generator (Ninja).
$ python3 cbuilder.py --builder cmake --source_dir src --build_dir build

# Example 2: Configure and build a project using CMake with Unix Makefiles generator
# This example configures and builds the project using the Unix Makefiles generator.
$ python3 cbuilder.py --builder cmake --source_dir src --build_dir build --generator "Unix Makefiles"

# Example 3: Configure and build a project using CMake with Release build type
# This example configures and builds the project with the Release build type.
$ python3 cbuilder.py --builder cmake --source_dir src --build_dir build --build_type Release

# Example 4: Configure and build a project using CMake with custom options
# This example configures and builds the project with custom CMake options.
$ python3 cbuilder.py --builder cmake --source_dir src --build_dir build --cmake_options -DENABLE_FEATURE=ON

# Example 5: Configure, build, and install a project using CMake
# This example configures, builds, and installs the project.
$ python3 cbuilder.py --builder cmake --source_dir src --build_dir build --install

# Example 6: Configure, build, and run tests using CMake
# This example configures, builds, and runs tests for the project.
$ python3 cbuilder.py --builder cmake --source_dir src --build_dir build --test

# Example 7: Configure, build, and generate documentation using CMake
# This example configures, builds, and generates documentation for the project.
$ python3 cbuilder.py --builder cmake --source_dir src --build_dir build --generate_docs

# Example 8: Clean the build directory before configuring and building using CMake
# This example cleans the build directory before configuring and building the project.
$ python3 cbuilder.py --builder cmake --source_dir src --build_dir build --clean

# Example 9: Configure and build a project using Meson with default settings
# This example configures and builds the project located in the 'src' directory,
# using the 'build' directory for build files, and the default Meson build type (debug).
$ python3 cbuilder.py --builder meson --source_dir src --build_dir build

# Example 10: Configure and build a project using Meson with release build type
# This example configures and builds the project with the release build type.
$ python3 cbuilder.py --builder meson --source_dir src --build_dir build --build_type release

# Example 11: Configure and build a project using Meson with custom options
# This example configures and builds the project with custom Meson options.
$ python3 cbuilder.py --builder meson --source_dir src --build_dir build --meson_options -Dfeature=enabled

# Example 12: Configure, build, and install a project using Meson
# This example configures, builds, and installs the project.
$ python3 cbuilder.py --builder meson --source_dir src --build_dir build --install

# Example 13: Configure, build, and run tests using Meson
# This example configures, builds, and runs tests for the project.
$ python3 cbuilder.py --builder meson --source_dir src --build_dir build --test

# Example 14: Configure, build, and generate documentation using Meson
# This example configures, builds, and generates documentation for the project.
$ python3 cbuilder.py --builder meson --source_dir src --build_dir build --generate_docs

# Example 15: Clean the build directory before configuring and building using Meson
# This example cleans the build directory before configuring and building the project.
$ python3 cbuilder.py --builder meson --source_dir src --build_dir build --clean

# Example 16: Set environment variables for the build process
# This example sets environment variables for the build process.
$ python3 cbuilder.py --builder cmake --source_dir src --build_dir build --env VAR1=value1 VAR2=value2

# Example 17: Enable verbose output during the build process
# This example enables verbose output during the build process.
$ python3 cbuilder.py --builder cmake --source_dir src --build_dir build --verbose

# Example 18: Specify the number of parallel jobs for building
# This example specifies the number of parallel jobs for building.
$ python3 cbuilder.py --builder cmake --source_dir src --build_dir build --parallel 8