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

# Include directories
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/libs/)
include_directories(${CMAKE_SOURCE_DIR}/driverlibs/)
include_directories(${lithium_src_dir})
include_directories(${lithium_module_dir})
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp)
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp-swagger)
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp-websocket)

# Find packages
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(fmt REQUIRED)

# Configure config.h
configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# Add subdirectories
add_subdirectory(libs)
add_subdirectory(modules)
add_subdirectory(${lithium_module_dir})
add_subdirectory(driver)
add_subdirectory(${lithium_src_dir}/config)
add_subdirectory(tests)

# Set source files
set(component_module
    ${lithium_component_dir}/addons.cpp
    ${lithium_component_dir}/compiler.cpp
    ${lithium_component_dir}/loader.cpp
    ${lithium_component_dir}/manager.cpp
    ${lithium_component_dir}/sandbox.cpp
    ${lithium_component_dir}/sort.cpp
)

set(config_module
    ${lithium_src_dir}/config/configor.cpp
)

set(debug_module
    ${lithium_src_dir}/debug/terminal.cpp
    ${lithium_src_dir}/debug/suggestion.cpp
    ${lithium_src_dir}/debug/command.cpp
    ${lithium_src_dir}/debug/console.cpp
)

set(script_module
    ${lithium_src_dir}/script/manager.cpp
    ${lithium_src_dir}/script/custom/sys.cpp
    ${lithium_src_dir}/script/custom/config.cpp
    ${lithium_src_dir}/script/sheller.cpp
)

set(task_module
    ${lithium_task_dir}/manager.cpp
    ${lithium_task_dir}/generator.cpp
    ${lithium_task_dir}/container.cpp
    ${lithium_task_dir}/tick.cpp
    ${lithium_task_dir}/loader.cpp
    ${lithium_task_dir}/list.cpp
    ${lithium_task_dir}/pool.cpp
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
    ${script_module}
    ${task_module}
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
        lithium_webserver
        oatpp-websocket
        oatpp-swagger
        oatpp-openssl
        oatpp-zlib
        oatpp
        loguru
        libzippp
        atomstatic
        carbon
        fmt::fmt
        OpenSSL::SSL
        OpenSSL::Crypto
        ${ZLIB_LIBRARIES}
        sqlite3
        cpp_httplib
        backward
        tinyxml2
        pocketpy
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
