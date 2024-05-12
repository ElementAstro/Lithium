import os
import sys
import json

def query_user():
    print("Please provide the following information for your new module:")
    module_name = input("Module Name (e.g., atom.utils): ").strip()
    version = input("Version (e.g., 1.0.0): ").strip()
    description = input("Description: ").strip()
    author = input("Author: ").strip()
    license = input("License (e.g., GPL-3.0-or-later): ").strip()
    repo_url = input("Repository URL (e.g., https://github.com/ElementAstro/Lithium): ").strip()

    return module_name, version, description, author, license, repo_url

def create_cmakelists(module_name):
    content = f"""# CMakeLists.txt for {module_name}
cmake_minimum_required(VERSION 3.20)
project({module_name})

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# Add source files
set(SOURCE_FILES
    {module_name}.cpp
)

# Add header files
set(HEADER_FILES
    {module_name}.hpp
)

# Create the module library
add_library({module_name} ${{SOURCE_FILES}} ${{HEADER_FILES}})

# Include directories
target_include_directories({module_name} PUBLIC ${{CMAKE_CURRENT_SOURCE_DIR}})
"""
    return content

def create_header(module_name):
    guard = module_name.upper().replace('.', '_')
    content = f"""#ifndef {guard}_HPP
#define {guard}_HPP

namespace {module_name.replace('.', '::')} {{

    void say_hello();

}} // namespace {module_name.replace('.', '::')}

#endif // {guard}_HPP
"""
    return content

def create_source(module_name):
    content = f"""#include "{module_name}.hpp"
#include <iostream>

namespace {module_name.replace('.', '::')} {{

    void say_hello() {{
        std::cout << "Hello from the {module_name} module!" << std::endl;
    }}

}} // namespace {module_name.replace('.', '::')}
"""
    return content

def create_package_json(module_name, version, description, author, license, repo_url):
    package = {
        "name": module_name,
        "version": version,
        "type": "shared",
        "description": description,
        "license": license,
        "author": author,
        "repository": {
            "type": "git",
            "url": repo_url
        },
        "bugs": {
            "type": "git",
            "url": f"{repo_url}/issues"
        },
        "homepage": {
            "type": "git",
            "url": repo_url
        },
        "keywords": [
            "lithium",
            module_name.split('.')[0],
            module_name.split('.')[1]
        ],
        "scripts": {
            "build": "cmake --build . --config Release -- -j 4",
            "lint": "clang-format -i src/*.cpp src/*.h"
        },
        "modules": [
            {
                "name": module_name.split('.')[-1],
                "entry": "getInstance"
            }
        ]
    }
    return json.dumps(package, indent=4)

def main():

    module_name, version, description, author, license, repo_url = query_user()
    module_dir = os.path.join(os.getcwd(), module_name)

    # Create the directory if it doesn't exist
    os.makedirs(module_dir, exist_ok=True)

    # Create CMakeLists.txt
    with open(os.path.join(module_dir, 'CMakeLists.txt'), 'w') as f:
        f.write(create_cmakelists(module_name))

    # Create module_name.hpp
    with open(os.path.join(module_dir, f'{module_name.split(".")[-1]}.hpp'), 'w') as f:
        f.write(create_header(module_name))

    # Create module_name.cpp
    with open(os.path.join(module_dir, f'{module_name.split(".")[-1]}.cpp'), 'w') as f:
        f.write(create_source(module_name))

    # Create package.json
    with open(os.path.join(module_dir, 'package.json'), 'w') as f:
        f.write(create_package_json(module_name, version, description, author, license, repo_url))

    print(f"Module {module_name} has been created successfully in {module_dir}")

if __name__ == "__main__":
    main()