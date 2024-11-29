# Example 1: Generate CMakeLists.txt from a JSON configuration file
# This example generates a CMakeLists.txt file based on the project settings specified in project_config.json.
# The generated CMakeLists.txt file will be saved in the current directory.
$ python3 cmake_generator.py generate --json project_config.json

# Example 2: Generate CMakeLists.txt from a JSON configuration file and save to a specific directory
# This example generates a CMakeLists.txt file based on the project settings specified in project_config.json.
# The generated CMakeLists.txt file will be saved in the specified output directory (output_dir).
$ python3 cmake_generator.py generate --json project_config.json --output-dir output_dir

# Example 3: Generate FindXXX.cmake for a specified library
# This example generates a FindXXX.cmake file for the specified library (e.g., Boost).
# The generated FindBoost.cmake file will be saved in the default directory (cmake/).
$ python3 cmake_generator.py find --library Boost

# Example 4: Generate FindXXX.cmake for a specified library and save to a specific directory
# This example generates a FindXXX.cmake file for the specified library (e.g., OpenCV).
# The generated FindOpenCV.cmake file will be saved in the specified output directory (custom_cmake_dir).
$ python3 cmake_generator.py find --library OpenCV --output-dir custom_cmake_dir