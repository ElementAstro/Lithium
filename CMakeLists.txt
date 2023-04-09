# CMakeLists.txt for OpenAPT
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: OpenAPT
# Description: Automated image processing and analysis tool for astrophotography
# Author: Max Qian
# License: GPL3

# Use consistent formatting and naming conventions throughout the code
cmake_minimum_required(VERSION 3.16)

# Add comments to explain what different sections of code do
enable_language(C CXX)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
    if(POLICY CMP0043)
      cmake_policy(SET CMP0043 NEW)
    endif()
endif()

# root directory of the project
set(OPENAPT_PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR})
set(openapt_src_dir ${OPENAPT_PROJECT_ROOT_DIR})

add_custom_target(CmakeAdditionalFiles
  SOURCES
  ${openapt_src_dir}/cmake_modules/compiler_options.cmake)

# Add a GPL3 declaration
# Set the license information in the project here

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules/")

# compiler options
include(cmake_modules/compiler_options.cmake)

#################################################################################
#
# General defines for compatibility across different platforms
if(UNIX AND NOT APPLE)
  if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr CACHE PATH "openapt install path" FORCE)
  endif()
endif()

set(USE_FOLDERS TRUE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/websrc_modification
  COMMAND ${CMAKE_COMMAND} -E compare_files ${CMAKE_SOURCE_DIR}/websrc ${CMAKE_BINARY_DIR}/websrc_modification
  BYPRODUCTS ${CMAKE_BINARY_DIR}/websrc_modification
  VERBATIM # Option to ensure that the command is executed correctly on all platforms
)

# Add a custom target that depends on the modification check
add_custom_target(check_websrc_modification DEPENDS ${CMAKE_BINARY_DIR}/websrc_modification)

# Set up the build directory with the necessary directories for static and templates
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/static)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/templates)

# Use file(GLOB_RECURSE) to automatically detect all files in the directories
file(GLOB_RECURSE STATIC_FILES ${CMAKE_SOURCE_DIR}/websrc/static/*)
file(GLOB_RECURSE TEMPLATE_FILES ${CMAKE_SOURCE_DIR}/websrc/templates/*)

# Add a custom command to copy any modified static and template files to the build directory
add_custom_command(
  TARGET check_websrc_modification
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${STATIC_FILES} ${CMAKE_BINARY_DIR}/static/
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TEMPLATE_FILES} ${CMAKE_BINARY_DIR}/templates/
  COMMAND_EXPAND_LISTS # Option to expand lists when copying files
)

#################################################################################
#
# main project
# this should appear after setting the architecture
project(openapt)

add_subdirectory(libs/)
add_subdirectory(libs/pugixml)
add_subdirectory(libs/lua)

add_subdirectory(modules)

include_directories(${CMAKE_SOURCE_DIR}/libs/crow)
include_directories(${CMAKE_SOURCE_DIR}/libs/)
include_directories(${CMAKE_SOURCE_DIR}/src/)

set(plugins_SRC
    ${openapt_src_dir}/src/plugins/thread.cpp
    ${openapt_src_dir}/src/plugins/thread.hpp

    ${openapt_src_dir}/src/plugins/file.cpp
    ${openapt_src_dir}/src/plugins/file.hpp

    ${openapt_src_dir}/src/plugins/process.cpp
    ${openapt_src_dir}/src/plugins/process.hpp

    ${openapt_src_dir}/src/plugins/compress.cpp
    ${openapt_src_dir}/src/plugins/compress.hpp

    ${openapt_src_dir}/src/plugins/time.cpp
    ${openapt_src_dir}/src/plugins/time.hpp

    ${openapt_src_dir}/src/plugins/io.cpp
    ${openapt_src_dir}/src/plugins/io.hpp

    ${openapt_src_dir}/src/plugins/xmlreader.cpp
    ${openapt_src_dir}/src/plugins/xmlreader.hpp

    ${openapt_src_dir}/src/plugins/crash.cpp
    ${openapt_src_dir}/src/plugins/crash.hpp

    ${openapt_src_dir}/src/plugins/terminal.cpp
    ${openapt_src_dir}/src/plugins/terminal.hpp
)

set(image_SRC
    ${openapt_src_dir}/src/image/image.cpp
)

set(asx_SRC
    ${openapt_src_dir}/src/asx/search.cpp
    ${openapt_src_dir}/src/asx/search.hpp
)

set(api_SRC
    ${openapt_src_dir}/src/api/astap.cpp
    ${openapt_src_dir}/src/api/astap.hpp

    ${openapt_src_dir}/src/api/astrometry.cpp
    ${openapt_src_dir}/src/api/astrometry.hpp

    ${openapt_src_dir}/src/api/indiclient.cpp
    ${openapt_src_dir}/src/api/indiclient.hpp
)

set(indi_SRC
)

set(openapt_SRC
    ${openapt_src_dir}/src/module/modloader.cpp
    ${openapt_src_dir}/src/module/modloader.hpp

    ${openapt_src_dir}/src/module/lualoader.cpp
    ${openapt_src_dir}/src/module/lualoader.hpp

    ${openapt_src_dir}/src/module/pythonloader.cpp
    ${openapt_src_dir}/src/module/pythonloader.hpp

    ${openapt_src_dir}/src/module/compiler.cpp
    ${openapt_src_dir}/src/module/compiler.hpp

    ${openapt_src_dir}/src/package/packageloader.cpp
    ${openapt_src_dir}/src/package/packageloader.hpp

    ${openapt_src_dir}/src/task/define.hpp
    ${openapt_src_dir}/src/task/runner.cpp
    ${openapt_src_dir}/src/task/runner.hpp

    ${openapt_src_dir}/src/device/basic_device.cpp
    ${openapt_src_dir}/src/device/basic_device.hpp

    ${openapt_src_dir}/src/device/manager.cpp
    ${openapt_src_dir}/src/device/manager.hpp

    ${openapt_src_dir}/src/config/configor.cpp
    ${openapt_src_dir}/src/config/configor.hpp
    ${openapt_src_dir}/src/config/achievement.cpp
    ${openapt_src_dir}/src/config/achievement.hpp
    ${openapt_src_dir}/src/config/achievement_list.cpp
    ${openapt_src_dir}/src/config/achievement_list.hpp

    ${openapt_src_dir}/src/openapt.cpp
    ${openapt_src_dir}/src/openapt.hpp
)

if(UNIX)
  # should be linux there
  add_executable(openapt ${plugins_SRC} ${image_SRC} ${api_SRC} ${indi_SRC} ${asx_SRC} ${openapt_SRC})
  
  # Use consistent formatting for target_precompile_headers
  target_precompile_headers(openapt PUBLIC ${openapt_src_dir}/src/openapt.hpp)
  
  # Use consistent formatting for find_package
  find_package(ZLIB REQUIRED)
  
  # Find and link CFITSIO libraries
  find_package(CFITSIO REQUIRED)
  target_link_libraries(openapt ${CFITSIO_LIBRARIES})
  
  # Find and link INDI libraries
  find_package(INDI 2.0.0 REQUIRED)
  include_directories(${INDI_INCLUDE_DIR})
  target_link_libraries(openapt ${INDI_CLIENT_LIBRARIES})
  
  # Check if Python is installed
  find_package(Python REQUIRED COMPONENTS Development)
  if(Python_FOUND)
    message(STATUS "Python version: ${Python_VERSION}")
  else()
    message(SEND_ERROR "Python not found.")
  endif()

  # Add Python headers and library paths
  include_directories(${Python_INCLUDE_DIRS})
  link_directories(${Python_LIBRARY_DIRS})

  # Add compiler flags and link Python libraries
  add_definitions(${Python_DEFINITIONS})
  target_link_libraries(openapt ${Python_LIBRARIES})

  # Find and link OpenSSL libraries
  find_package(OpenSSL REQUIRED)
  if(OpenSSL_FOUND)
    target_link_libraries(openapt ${OPENSSL_LIBRARIES})
    message("-- Using OpenSSL ${OPENSSL_VERSION}")
  else()
    message("-- OpenSSL Not Found")
  endif()

  find_package(SQLite3 REQUIRED)
  target_link_libraries(openapt sqlite3)
  
  # Find and link spdlog libraries
  find_package(spdlog REQUIRED)
  target_link_libraries(openapt spdlog::spdlog_header_only)

  # Link ZLIB and pugixml-static libraries
  target_link_libraries(openapt ${ZLIB_LIBRARIES})
  target_link_libraries(openapt pugixml-static)

  target_link_libraries(openapt lua)

  # Set output name for openapt executable
  set_target_properties(
    openapt
    PROPERTIES
    OUTPUT_NAME openapt
  )

else()
  message(FATAL_ERROR "Unsupported platform")
endif()
