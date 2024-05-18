#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import json
from datetime import datetime

def query_user():
    print("Please provide the following information for your new module:")
    module_name = input("Module Name (e.g., atom.utils): ").strip()
    component_name = input("Component Name (the name of the shared_ptr, should be considered seriously, e.g. SystemComponent): ").strip()
    version = input("Version (e.g., 1.0.0): ").strip()
    description = input("Description: ").strip()
    author = input("Author: ").strip()
    license = input("License (e.g., GPL-3.0-or-later): ").strip()
    repo_url = input("Repository URL (e.g., https://github.com/ElementAstro/Lithium): ").strip()
    cpp_standard = input("C++ Standard (e.g., 20): ").strip() or "20"
    additional_sources = input("Additional source files (comma-separated, e.g., utils.cpp,math.cpp): ").strip()
    additional_headers = input("Additional header files (comma-separated, e.g., utils.hpp,math.hpp): ").strip()

    return module_name, component_name, version, description, author, license, repo_url, cpp_standard, additional_sources, additional_headers

def confirm_details(details):
    print("\nPlease confirm the entered details:")
    for key, value in details.items():
        print(f"{key}: {value}")

    confirm = input("\nIs this information correct? (yes/no/edit): ").strip().lower()
    if confirm == 'edit':
        return False, True
    return confirm == 'yes', False

def create_cmakelists(module_name, cpp_standard, additional_sources, additional_headers):
    base_name = module_name.split('.')[-1]
    all_sources = [f"{base_name}.cpp"] + additional_sources.split(',')
    all_headers = [f"{base_name}.hpp"] + additional_headers.split(',')

    all_sources_list = "\n    ".join(all_sources)
    all_headers_list = "\n    ".join(all_headers)

    content = f"""# CMakeLists.txt for {module_name}
# This project is licensed under the terms of the GPL3 license.
#
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project({module_name})

# Set the C++ standard
set(CMAKE_CXX_STANDARD {cpp_standard})

# Add source files
set(SOURCE_FILES
    {all_sources_list}

    _component.cpp
    _main.cpp
)

# Create the module library
add_library({module_name} SHARED ${{SOURCE_FILES}})

# Include directories
target_include_directories({module_name} PUBLIC ${{CMAKE_CURRENT_SOURCE_DIR}}/include)
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
    content = f"""#include "{module_name.split('.')[-1]}.hpp"
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
            "url": f"{repo_url}/issues"
        },
        "homepage": repo_url,
        "keywords": [
            "lithium",
            *module_name.split('.')
        ],
        "scripts": {
            "build": "cmake --build . --config Release -- -j 4",
            "lint": "clang-format -i src/*.cpp include/*.h"
        },
        "modules": [
            {
                "name": module_name.split('.')[-1],
                "entry": "getInstance"
            }
        ]
    }
    return json.dumps(package, indent=4)

def create_readme(module_name, description, author, version):
    content = f"""# {module_name}

{description}

## Installation

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

Include the `{module_name.split('.')[-1]}.hpp` in your project and use the namespaces and functions provided by the module.

## Contributing

Contributions are welcome! Please read the contributing guidelines and code of conduct before making any changes.

## License

This project is licensed under the {version} License - see the LICENSE file for details.

## Author

{author}
"""
    return content

def create_gitignore():
    content = """# Prerequisites
*.d

# Compiled Object files
*.slo
*.lo
*.o
*.obj

# Precompiled Headers
*.gch
*.pch

# Compiled Dynamic libraries
*.so
*.dylib
*.dll

# Fortran module files
*.mod
*.smod

# Compiled Static libraries
*.lai
*.la
*.a
*.lib

# Executables
*.exe
*.out
*.app


build
.vscode
node_modules

test

*.log
*.xml

.xmake
"""
    return content

def get_current_date():
    # 获取当前日期时间对象
    current_date = datetime.now()
    # 格式化日期为YYYY-MM-DD的形式
    formatted_date = current_date.strftime("%Y-%m-%d")
    return formatted_date

def convert_to_upper_snake_case(s: str) -> str:
    """
    Convert a dot-separated string to UPPERCASE_SNAKE_CASE.

    Args:
        s (str): The input string.

    Returns:
        str: The converted string in UPPERCASE_SNAKE_CASE format.
    """
    return s.replace('.', '_').upper()

def create_component_main(author, module_name, component_name):
    content = f"""/*
 * _main.cpp
 *
 * Copyright (C) 2023-2024 {author}
 */

/*************************************************

Date: {get_current_date()}

Description: Main Entry

**************************************************/

#include "_component.hpp"

#include "atom/type/json.hpp"
using json = nlohmann::json;

extern "C" {{
std::shared_ptr<Component> getInstance([[maybe_unused]] const json &params) {{
    if (params.contains("name") && params["name"].is_string()) {{
        return std::make_shared<{component_name}>(
            params["name"].get<std::string>());
    }}
    return std::make_shared<{component_name}>("{module_name}");
}}
}}
    """
    return content

def create_component_hpp(author, description, module_name, component_name):
    content = f"""
/*
 * _component.hpp
 *
 * Copyright (C) 2023-2024 {author}
 */

/*************************************************

Date: {get_current_date()}

Description: {description}

**************************************************/

#ifndef {convert_to_upper_snake_case(module_name)}_COMPONENT_HPP
#define {convert_to_upper_snake_case(module_name)}_COMPONENT_HPP

#include "atom/components/component.hpp"

class {component_name} : public Component {{
public:
    explicit {component_name}(const std::string& name);
    virtual ~{component_name}();

    bool initialize() override;
    bool destroy() override;
}};
#endif
"""
    return content

def create_component_cpp(author, description,module_name, component_name):
    content = f"""/*
 * _component.cpp
 *
 * Copyright (C) 2023-2024 {author}
 */

/*************************************************

Date: {get_current_date()}

Description: {description}

**************************************************/

#include "_component.hpp"

#include "atom/log/loguru.hpp"

{component_name}::{component_name}(const std::string& name)
    : Component(name) {{
    LOG_F(INFO, "{component_name} Constructed");
}}

{component_name}::~{component_name}() {{
    LOG_F(INFO, "{component_name} Destructed");
}}

bool {component_name}::initialize() {{
    LOG_F(INFO, "{component_name} Initialized");
    return true;
}}

bool {component_name}::destroy() {{
    LOG_F(INFO, "{component_name} Destroyed");
    return true;
}}
"""
    return content

def main():
    while True:
        details = query_user()
        module_name, component_name, version, description, author, license, repo_url, cpp_standard, additional_sources, additional_headers = details

        details_dict = {
            "Module Name": module_name,
            "Version": version,
            "Description": description,
            "Author": author,
            "License": license,
            "Repository URL": repo_url,
            "C++ Standard": cpp_standard,
            "Additional Sources": additional_sources,
            "Additional Headers": additional_headers
        }

        confirmed, edit = confirm_details(details_dict)
        if edit:
            continue
        if confirmed:
            break
        else:
            print("Please re-enter the information...\n")

    module_dir = os.path.join(os.getcwd(), module_name)

    # Create the directory structure
    os.makedirs(module_dir, exist_ok=True)
    os.makedirs(os.path.join(module_dir, "src"), exist_ok=True)
    os.makedirs(os.path.join(module_dir, "include"), exist_ok=True)

    base_name = module_name.split('.')[-1]

    # Create _main.cpp
    main_cpp_path = os.path.join(module_dir, f"_main.cpp")
    with open(main_cpp_path, "w") as f:
        f.write(create_component_main(author, module_name, component_name))

    # Create _component.cpp
    component_cpp_path = os.path.join(module_dir, f"_component.cpp")
    with open(component_cpp_path, "w") as f:
        f.write(create_component_cpp(author,description, module_name, component_name))

    # Create _component.hpp
    component_hpp_path = os.path.join(module_dir, f"_component.hpp")
    with open(component_hpp_path, "w") as f:
        f.write(create_component_hpp(author, description, module_name, component_name))

    # Create CMakeLists.txt
    with open(os.path.join(module_dir, 'CMakeLists.txt'), 'w') as f:
        f.write(create_cmakelists(module_name, cpp_standard, additional_sources, additional_headers))

    # Create module_name.hpp
    with open(os.path.join(module_dir, f'include/{base_name}.hpp'), 'w') as f:
        f.write(create_header(module_name))

    # Create module_name.cpp
    with open(os.path.join(module_dir, f'src/{base_name}.cpp'), 'w') as f:
        f.write(create_source(module_name))

    # Create additional source and header files
    for src in filter(None, additional_sources.split(',')):
        with open(os.path.join(module_dir, f'src/{src}'), 'w') as f:
            f.write(f"// {src}\n")
    for hdr in filter(None, additional_headers.split(',')):
        with open(os.path.join(module_dir, f'include/{hdr}'), 'w') as f:
            f.write(f"// {hdr}\n")

    # Create package.json
    with open(os.path.join(module_dir, 'package.json'), 'w') as f:
        f.write(create_package_json(module_name, version, description, author, license, repo_url))

    # Create README.md
    with open(os.path.join(module_dir, 'README.md'), 'w') as f:
        f.write(create_readme(module_name, description, author, version))

    # Create .gitignore
    with open(os.path.join(module_dir, '.gitignore'), 'w') as f:
        f.write(create_gitignore())

    print(f"Module {module_name} has been created successfully in {module_dir}")

if __name__ == "__main__":
    main()
