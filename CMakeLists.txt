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
include(cmake/options.cmake)

# Set policies
include(cmake/policies.cmake)

# Set project directories
set(Lithium_PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR})
set(lithium_src_dir ${Lithium_PROJECT_ROOT_DIR}/src)
set(atom_module ${lithium_src_dir}/atom)
set(lithium_client_dir ${lithium_src_dir}/client)
set(lithium_component_dir ${lithium_src_dir}/addon)
set(lithium_task_dir ${lithium_src_dir}/task)

#add_custom_target(CmakeAdditionalFiles
#	SOURCES
#	${lithium_src_dir}/../cmake/compiler_options.cmake)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/")
include(cmake/compiler_options.cmake)

# ------------------ CPM Begin ------------------

set(CPM_DOWNLOAD_VERSION 0.35.6)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake")
    file(DOWNLOAD
        "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
        ${CPM_DOWNLOAD_LOCATION}
    )
endif()

include(${CPM_DOWNLOAD_LOCATION})

# ------------------ CPM End ------------------

# ------------------ Conan Begin ------------------

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

# ------------------ Conan End ------------------

# Include directories
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/libs/)
include_directories(${lithium_src_dir})
include_directories(${atom_module})
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp/oatpp)
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp-swagger/oatpp-swagger)
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp-websocket/oatpp-websocket)
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp-openssl/oatpp-openssl)

# Find packages
include(cmake/find_packages.cmake)

# Configure Python environment
include(cmake/python_environment.cmake)

# Configure config.h
configure_file(${lithium_src_dir}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(BUILD_SHARED_LIBS ON)

# Add subdirectories
add_subdirectory(libs)
add_subdirectory(modules)

add_subdirectory(${atom_module})

add_subdirectory(${lithium_src_dir}/config)
add_subdirectory(${lithium_src_dir}/task)
add_subdirectory(${lithium_src_dir}/server)
add_subdirectory(${lithium_src_dir}/utils)
add_subdirectory(${lithium_src_dir}/addon)
add_subdirectory(${lithium_src_dir}/client)
add_subdirectory(${lithium_src_dir}/target)
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
    ${lithium_src_dir}/device/template/device.cpp
)

set(script_module
    ${lithium_src_dir}/script/manager.cpp
    ${lithium_src_dir}/script/pycaller.cpp
    ${lithium_src_dir}/script/pycaller.hpp
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
        pybind11::embed
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
include(cmake/install_paths.cmake)

# Enable folder grouping in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
