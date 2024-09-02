# CMakeLists.txt for Lithium
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Lithium
# Description: Lithium - Open Astrophotography Terminal
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(Lithium VERSION 1.0.0 LANGUAGES C CXX)

# Set project options
option(ENABLE_ASYNC "Enable Async Server Mode" ON)
option(ENABLE_NATIVE_SERVER "Enable to use INDI native server" OFF)
option(ENABLE_DEBUG "Enable Debug Mode" OFF)
option(ENABLE_FASHHASH "Enable Using emhash8 as fast hash map" OFF)
option(ENABLE_WEB_SERVER "Enable Web Server" ON)
option(ENABLE_WEB_CLIENT "Enable Web Client" ON)

# Set compile definitions based on options
if(ENABLE_ASYNC)
    add_compile_definitions(ENABLE_ASYNC_FLAG=1)
endif()
if(ENABLE_DEBUG)
    add_compile_definitions(ENABLE_DEBUG_FLAG=1)
endif()
if(ENABLE_NATIVE_SERVER)
    add_compile_definitions(ENABLE_NATIVE_SERVER_FLAG=1)
endif()
if(ENABLE_FASHHASH)
    add_compile_definitions(ENABLE_FASHHASH_FLAG=1)
endif()
if(ENABLE_WEB_SERVER)
    add_compile_definitions(ENABLE_WEB_SERVER_FLAG=1)
endif()
if(ENABLE_WEB_CLIENT)
    add_compile_definitions(ENABLE_WEB_CLIENT_FLAG=1)
endif()

# Set policies
if(POLICY CMP0003)
    cmake_policy(SET CMP0003 NEW)
endif()
if(POLICY CMP0043)
    cmake_policy(SET CMP0043 NEW)
endif()

# Set project directories
set(Lithium_PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR})
set(lithium_src_dir ${Lithium_PROJECT_ROOT_DIR}/src)
set(lithium_module_dir ${lithium_src_dir}/atom)
set(lithium_client_dir ${lithium_src_dir}/client)
set(lithium_component_dir ${lithium_src_dir}/addon)
set(lithium_task_dir ${lithium_src_dir}/task)

add_custom_target(CmakeAdditionalFiles
	SOURCES
	${lithium_src_dir}/../cmake_modules/compiler_options.cmake)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules/")
include(cmake_modules/compiler_options.cmake)

if (USE_CONAN)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake OPTIONAL) # Optional inclusion

# 检查是否已经安装Conan
find_program(CONAN_CMD conan)
if(NOT CONAN_CMD)
    message(FATAL_ERROR "Conan is not installed. Please install Conan (pip install conan).")
endif()

# 检测Conan默认配置文件是否存在
execute_process(
    COMMAND ${CONAN_CMD} config home
    OUTPUT_VARIABLE CONAN_HOME
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(CONAN_DEFAULT_PROFILE "${CONAN_HOME}/profiles/default")

if(NOT EXISTS "${CONAN_DEFAULT_PROFILE}")
    message(STATUS "Conan default profile not found. Creating a new profile based on platform.")
    # 根据操作系统创建默认配置
    if(WIN32)
        execute_process(COMMAND ${CONAN_CMD} profile detect --force)
    elseif(UNIX)
        execute_process(COMMAND ${CONAN_CMD} profile detect --force)
    else()
        message(FATAL_ERROR "Unsupported platform for Conan profile detection.")
    endif()
endif()

# 如果conanbuildinfo.cmake不存在，执行conan install命令
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    message(STATUS "Running Conan install...")
    execute_process(
        COMMAND ${CONAN_CMD} install ${CMAKE_SOURCE_DIR} --build=missing
        RESULT_VARIABLE result
    )
    if(result)
        message(FATAL_ERROR "Conan install failed with error code: ${result}")
    endif()
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
endif()
endif()

# Include directories
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/libs/)
include_directories(${CMAKE_SOURCE_DIR}/driverlibs/)
include_directories(${lithium_src_dir})
include_directories(${lithium_module_dir})
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp/oatpp)
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp-swagger/oatpp-swagger)
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp-websocket/oatpp-websocket)
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp-openssl/oatpp-openssl)

# Find packages
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(fmt REQUIRED)
find_package(Readline REQUIRED)

find_package(Python COMPONENTS Interpreter REQUIRED)

# Specify the path to requirements.txt
set(REQUIREMENTS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt")

# Define a function to check if a Python package is installed
function(check_python_package package version)
    # Replace hyphens with underscores for the import statement
    string(REPLACE "-" "_" import_name ${package})

    # Check if the package can be imported
    execute_process(
        COMMAND ${Python_EXECUTABLE} -c "import ${import_name}"
        RESULT_VARIABLE result
    )

    if(NOT result EQUAL 0)
        set(result FALSE PARENT_SCOPE)
        return()
    endif()

    # Get the installed package version
    execute_process(
        COMMAND ${Python_EXECUTABLE} -m pip show ${package}
        OUTPUT_VARIABLE package_info
    )

    # Extract version information from the output
    string(FIND "${package_info}" "Version:" version_pos)

    if(version_pos EQUAL -1)
        set(result FALSE PARENT_SCOPE)
        return() # Return false if version not found
    endif()

    # Extract the version string
    string(SUBSTRING "${package_info}" ${version_pos} 1000 version_string)
    string(REGEX REPLACE "Version: ([^ ]+).*" "\\1" installed_version "${version_string}")

    # Compare versions
    if("${installed_version}" VERSION_LESS "${version}")
        set(result FALSE PARENT_SCOPE) # Return false if installed version is less than required
        return()
    endif()

    set(result TRUE PARENT_SCOPE)
endfunction()

if (EXISTS "${CMAKE_BINARY_DIR}/check_marker.txt")
    message(STATUS "Check marker file found, skipping the checks.")
else()
# Create a virtual environment
set(VENV_DIR "${CMAKE_BINARY_DIR}/venv")
execute_process(
    COMMAND ${Python_EXECUTABLE} -m venv ${VENV_DIR}
)

set(PYTHON_EXECUTABLE "${VENV_DIR}/bin/python")
set(PIP_EXECUTABLE "${VENV_DIR}/bin/pip")

# Upgrade pip in the virtual environment
execute_process(
    COMMAND ${PIP_EXECUTABLE} install --upgrade pip
)

# Read the requirements.txt file and install missing packages
file(READ ${REQUIREMENTS_FILE} requirements_content)

# Split the requirements file content into lines
string(REPLACE "\n" ";" requirements_list "${requirements_content}")

# Check and install each package
foreach(requirement ${requirements_list})
    # Skip empty lines
    string(STRIP ${requirement} trimmed_requirement)
    if(trimmed_requirement STREQUAL "")
        continue()
    endif()

    # Get the package name and version (without the version number)
    if(${trimmed_requirement} MATCHES "==")
        string(REPLACE "==" ";" parts ${trimmed_requirement})
    elseif(${trimmed_requirement} MATCHES ">=")
        string(REPLACE ">=" ";" parts ${trimmed_requirement})
    else()
        message(WARNING "Could not parse requirement '${trimmed_requirement}'. Skipping...")
        continue()
    endif()

    list(GET parts 0 package_name)
    list(GET parts 1 package_version)

    # If the package name or version could not be parsed, output a warning and skip
    if(NOT package_name OR NOT package_version)
        message(WARNING "Could not parse requirement '${trimmed_requirement}'. Skipping...")
        continue()
    endif()

    # Check if the package is installed
    message(STATUS "Checking if Python package '${package_name}' is installed...")
    check_python_package(${package_name} ${package_version})
    if(NOT result)
        message(STATUS "Package '${package_name}' is not installed or needs an upgrade. Installing...")
        execute_process(
            COMMAND ${PIP_EXECUTABLE} install ${trimmed_requirement}
            RESULT_VARIABLE install_result
        )
        if(NOT install_result EQUAL 0)
            message(FATAL_ERROR "Failed to install Python package '${package_name}'.")
        endif()
    else()
        message(STATUS "Package '${package_name}' is already installed with a suitable version.")
    endif()
endforeach()
execute_process(
    COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_BINARY_DIR}/check_marker.txt"
    RESULT_VARIABLE result
)
endif()

# Configure config.h
configure_file(${lithium_src_dir}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(BUILD_SHARED_LIBS ON)

# Add subdirectories
add_subdirectory(libs)
add_subdirectory(modules)
add_subdirectory(${lithium_module_dir})
add_subdirectory(${lithium_src_dir}/config)
add_subdirectory(${lithium_src_dir}/task)
add_subdirectory(${lithium_src_dir}/server)
add_subdirectory(${lithium_src_dir}/utils)
add_subdirectory(${lithium_src_dir}/addon)
add_subdirectory(${lithium_src_dir}/client)
add_subdirectory(${lithium_src_dir}/device)
add_subdirectory(tests)

# Set source files
set(component_module
    ${lithium_component_dir}/addons.cpp
    ${lithium_component_dir}/compiler.cpp
    ${lithium_component_dir}/dependency.cpp
    ${lithium_component_dir}/loader.cpp
    ${lithium_component_dir}/manager.cpp
    ${lithium_component_dir}/sandbox.cpp
)

set(config_module
    ${lithium_src_dir}/config/configor.cpp
)

set(debug_module
    ${lithium_src_dir}/debug/terminal.cpp
    ${lithium_src_dir}/debug/suggestion.cpp
    ${lithium_src_dir}/debug/command.cpp
    ${lithium_src_dir}/debug/console.cpp
    ${lithium_src_dir}/debug/history.cpp
    ${lithium_src_dir}/debug/progress.cpp
    ${lithium_src_dir}/debug/output_style.cpp
    ${lithium_src_dir}/debug/check.cpp
)

set(device_module
    ${lithium_src_dir}/device/manager.cpp

    ${lithium_src_dir}/device/template/device.cpp
)

set(script_module
    ${lithium_src_dir}/script/manager.cpp
    ${lithium_src_dir}/script/sheller.cpp
)

set(Lithium_module
    ${lithium_src_dir}/LithiumApp.cpp
    ${lithium_src_dir}/utils/constant.cpp
)

# Build lithium_server-library
add_library(lithium_server-library STATIC
    ${component_module}
    ${config_module}
    ${debug_module}
    ${device_module}
    ${script_module}
    ${Lithium_module}
)

target_link_libraries(lithium_server-library
    PUBLIC
        loguru
        atom-component
)

# Build lithium_server executable
add_executable(lithium_server ${lithium_src_dir}/App.cpp)

target_link_libraries(lithium_server
    PRIVATE
        lithium_server-library
        lithium.server-lib
        lithium-config
        lithium-task
        lithium-addons
        oatpp-websocket
        oatpp-swagger
        oatpp-openssl
        oatpp-zlib
        oatpp
        loguru
        atom
        fmt::fmt
        OpenSSL::SSL
        OpenSSL::Crypto
        ${ZLIB_LIBRARIES}
        sqlite3
        cpp_httplib
        tinyxml2
        pocketpy
        ${Readline_LIBRARIES}
)

if(WIN32)
    find_package(dlfcn-win32 REQUIRED)
    target_link_libraries(lithium_server
        PRIVATE
            pdh
            iphlpapi
            winmm
            crypt32
            wsock32
            ws2_32
            dlfcn-win32::dl
    )
elseif(UNIX OR LINUX OR APPLE)
    target_link_libraries(lithium_server PRIVATE dl)
    find_package(Seccomp REQUIRED)
    if(Seccomp_FOUND)
        include_directories(${Seccomp_INCLUDE_DIRS})
        target_link_libraries(lithium_server PRIVATE ${Seccomp_LIBRARIES})
    endif()
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

target_compile_definitions(lithium_server PRIVATE LOGURU_DEBUG_LOGGING)

# Set output name for Lithium executable
set_target_properties(lithium_server PROPERTIES OUTPUT_NAME lithium_server)

# Set install paths
if(UNIX AND NOT APPLE)
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX /usr CACHE PATH "Lithium install path" FORCE)
    endif()
endif()

if(WIN32)
    set(CMAKE_INSTALL_PREFIX "C:/Program Files/LithiumServer")
elseif(LINUX)
    set(CMAKE_INSTALL_PREFIX "/usr/lithium")
endif()

# Enable folder grouping in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
