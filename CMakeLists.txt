# CMakeLists.txt for Lithium
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Lithium
# Description: Lithium - Open Astrophotography Terminal
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)

project(Lithium)
enable_language(C CXX)

if(COMMAND cmake_policy)
		cmake_policy(SET CMP0003 NEW)
		if(POLICY CMP0043)
			cmake_policy(SET CMP0043 NEW)
		endif()
endif()

# root directory of the project
set(Lithium_PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR})
set(lithium_src_dir ${Lithium_PROJECT_ROOT_DIR}/src)
set(lithium_module_dir ${lithium_src_dir}/atom)
set(lithium_client_dir ${lithium_src_dir}/client)
set(lithium_component_dir ${lithium_src_dir}/addon)
set(lithium_task_dir ${lithium_src_dir}/task)

add_custom_target(CmakeAdditionalFiles
	SOURCES
	${lithium_src_dir}/../cmake_modules/compiler_options.cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules/")

# compiler options
include(cmake_modules/compiler_options.cmake)

include(GNUInstallDirs)
include(FeatureSummary)
include(CheckSymbolExists)
include(CheckIncludeFileCXX)

# Clang Format support
if(UNIX OR APPLE)
    set(FORMAT_CODE OFF CACHE BOOL "Enable Clang Format")

    if(FORMAT_CODE MATCHES ON)
        FILE(GLOB_RECURSE ALL_SOURCE_FILES *.c *.cpp *.h)

        FOREACH(SOURCE_FILE ${ALL_SOURCE_FILES})
            STRING(FIND ${SOURCE_FILE} ${CMAKE_SOURCE_DIR} DIR_FOUND)

            if(NOT ${DIR_FOUND} EQUAL 0)
                LIST(REMOVE_ITEM ALL_SOURCE_FILES ${SOURCE_FILE})
            endif()
        ENDFOREACH()

        FIND_PROGRAM(CLANGFORMAT_EXE NAMES clang-format-5.0)

        if(CLANGFORMAT_EXE)
            ADD_CUSTOM_TARGET(clang-format COMMAND ${CLANGFORMAT_EXE} -style=file -i ${ALL_SOURCE_FILES})
        endif()
    endif()
endif()

#################################################################################
#
# Lithium version

set(CMAKE_LITHIUM_VERSION_MAJOR 1)
set(CMAKE_LITHIUM_VERSION_MINOR 0)
set(CMAKE_LITHIUM_VERSION_RELEASE 0)

set(LITHIUM_SOVERSION ${CMAKE_LITHIUM_VERSION_MAJOR})
set(CMAKE_LITHIUM_VERSION_STRING "${CMAKE_LITHIUM_VERSION_MAJOR}.${CMAKE_LITHIUM_VERSION_MINOR}.${CMAKE_LITHIUM_VERSION_RELEASE}")
set(LITHIUM_VERSION ${CMAKE_LITHIUM_VERSION_MAJOR}.${CMAKE_LITHIUM_VERSION_MINOR}.${CMAKE_LITHIUM_VERSION_RELEASE})

if(APPLE)
    set(CMAKE_SHARED_LINKER_FLAGS "-undefined dynamic_lookup")
endif(APPLE)

#################################################################################
#
# Lithium includes

include_directories(${CMAKE_SOURCE_DIR}/libs/)
include_directories(${CMAKE_SOURCE_DIR}/driverlibs/)
include_directories(${lithium_src_dir})
include_directories(${lithium_module_dir})
include_directories(${lithium_src_dir})
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp)
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp-swagger)
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp-websocket)

find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    message("-- Found OpenSSL ${OPENSSL_VERSION}: ${OPENSSL_LIBRARIES}")
else()
    message("-- OpenSSL Not Found")
endif()

find_package(CFITSIO REQUIRED)
find_package(ZLIB REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(fmt REQUIRED)

#################################################################################
#
# Subdirectories of Lithium

# Build all dependencies
# 构建所有需要的依赖库
add_subdirectory(libs/)

# Build all tools
# 所有的工具组件
add_subdirectory(tools)

# Build Atom core library
# 构建Atom核心库
add_subdirectory(${lithium_module_dir})

# 内置的模组
include_directories(modules)
add_subdirectory(modules)

add_subdirectory(driver)

add_subdirectory(${lithium_src_dir}/carbon)
add_subdirectory(${lithium_src_dir}/config)
add_subdirectory(${lithium_src_dir}/server)

#################################################################################
#
# General defines for compatibility across different platforms
if(UNIX AND NOT APPLE)
	if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		set(CMAKE_INSTALL_PREFIX /usr CACHE PATH "Lithium install path" FORCE)
	endif()
endif()

set(USE_FOLDERS TRUE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(ENABLE_ASYNC "Enable Async Server Mode" ON)
option(ENABLE_NATIVE_SERVER "Enable to use INDI native server" OFF)
option(ENABLE_DEBUG "Enable Debug Mode" OFF)

option(ENABLE_FASHHASH "Enable Using emhash8 as fast hash map" OFF)
option(ENABLE_WEB_SERVER "Enable Web Server" ON)
option(ENABLE_WEB_CLIENT "Enable Web Client" ON)

if(ENABLE_ASYNC)
    set(ENABLE_ASYNC_FLAG "1")
endif()
if(ENABLE_DEBUG)
    set(ENABLE_DEBUG_FLAG "1")
endif()
if(ENABLE_NATIVE_SERVER)
    set(ENABLE_NATIVE_SERVER_FLAG "1")
endif()
if(ENABLE_FASHHASH)
    set(ENABLE_FASHHASH_FLAG "1")
endif()
if(ENABLE_WEB_SERVER)
    set(ENABLE_WEB_SERVER_FLAG "1")
endif()
if(ENABLE_WEB_CLIENT)
    set(ENABLE_WEB_CLIENT_FLAG "1")
endif()

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#################################################################################

if(WIN32)
    set(CMAKE_INSTALL_PREFIX "C:/Program Files/LithiumServer")
elseif(LINUX)
    set(CMAKE_INSTALL_PREFIX "/usr/lithium")
endif()

set(component_module
    ${lithium_src_dir}/device/server/ascom.cpp
    ${lithium_src_dir}/device/server/hydrogen.cpp
    ${lithium_src_dir}/device/server/hydrogen_driver.cpp
    ${lithium_src_dir}/device/server/connector.cpp

    ${lithium_src_dir}/device/manager.cpp
    
    ${lithium_src_dir}/device/utils/utils.cpp

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
    # ${lithium_task_dir}/chekcer.cpp
)

set(Lithium_module
    ${lithium_src_dir}/LithiumApp.cpp
)

#################################################################################
# Main

add_library(lithium_server-library STATIC ${component_module} ${config_module} ${debug_module} ${module_module} ${device_module} ${task_module} ${script_module} ${Lithium_module})
add_executable(lithium_server ${lithium_src_dir}/app.cpp)
target_link_directories(lithium_server PUBLIC ${CMAKE_BINARY_DIR}/libs)

target_link_libraries(lithium_server lithium_server-library)
target_link_libraries(lithium_server lithium_webserver)
target_link_libraries(lithium_server oatpp-websocket oatpp-swagger oatpp-openssl oatpp-zlib oatpp)
target_link_libraries(lithium_server loguru)
target_link_libraries(lithium_server libzippp)
target_link_libraries(lithium_server atomstatic)
target_link_libraries(lithium_server-library fmt::fmt)

target_compile_definitions(lithium_server PRIVATE LOGURU_DEBUG_LOGGING)

target_link_libraries(lithium_server ${CFITSIO_LIBRARIES})
target_link_libraries(lithium_server OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(lithium_server ${ZLIB_LIBRARIES})
target_link_libraries(lithium_server sqlite3)
target_link_libraries(lithium_server cpp_httplib)
target_link_libraries(lithium_server backward)
target_link_libraries(lithium_server tinyxml2)
target_link_libraries(lithium_server pocketpy)

if(WIN32)
    target_link_libraries(lithium_server pdh iphlpapi winmm crypt32 wsock32 ws2_32)
    find_package(dlfcn-win32 REQUIRED)
    target_link_libraries(lithium_server dlfcn-win32::dl)
elseif(UNIX OR LINUX OR APPLE)
    target_link_libraries(lithium_server dl)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Set output name for Lithium executable
set_target_properties(
    lithium_server
    PROPERTIES
    OUTPUT_NAME lithium_server
)
