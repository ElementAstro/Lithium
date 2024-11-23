# Minimum required CMake version
cmake_minimum_required(VERSION 3.20)

# Project name and version, using C and C++ languages
project(lithium-addons VERSION 1.0.0 LANGUAGES C CXX)

# Project description and information
# This project is the official addonsuration module for the Lithium server.
# Author: Max Qian
# License: GPL3
# Project Name: Lithium-Addons
# Description: The official addons module for lithium server
# Author: Max Qian
# License: GPL3

if (NOT MINGW OR NOT WIN32)
find_package(Seccomp REQUIRED)
find_package(LibGit2 REQUIRED)

if (LIBGIT2_FOUND)
    message(STATUS "LibGit2 found!")
    include_directories(${LIBGIT2_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "LibGit2 not found!")
endif ()
endif()

# Project sources
set(PROJECT_SOURCES
    addons.cpp
    build_manager.cpp
    compile_command_generator.cpp
    compiler_output_parser.cpp
    compiler.cpp
    dependency.cpp
    generator.cpp
    loader.cpp
    manager.cpp
    sandbox.cpp
    system_dependency.cpp
    toolchain.cpp
    tracker.cpp
    version.cpp

    debug/dump.cpp
    debug/dynamic.cpp
    debug/elf.cpp
    debug/pdb.cpp

    platform/cmake.cpp
    platform/meson.cpp
    platform/xmake.cpp

    remote/github_impl.cpp
    remote/github.cpp

    template/remote.cpp
    template/standalone.cpp
)

# Project headers
set(PROJECT_HEADERS
    addons.hpp
    build_manager.hpp
    compile_command_generator.hpp
    compiler_output_parser.hpp
    compiler.hpp
    dependency.hpp
    generator.hpp
    loader.hpp
    manager.hpp
    sandbox.hpp
    system_dependency.hpp
    toolchain.hpp
    tracker.hpp
    version.hpp

    debug/dump.hpp
    debug/dynamic.hpp
    debug/elf.hpp
    debug/pdb.hpp

    platform/cmake.hpp
    platform/meson.hpp
    platform/xmake.hpp

    remote/github_impl.hpp
    remote/github.hpp

    template/remote.hpp
    template/standalone.hpp
)

if (NOT MINGW OR NOT WIN32)
set(PROJECT_SOURCES ${PROJECT_SOURCES} 
    project/git_impl.cpp
    project/git.cpp
)
set(PROJECT_HEADERS ${PROJECT_HEADERS}
    project/git_impl.hpp
    project/git.hpp
)
endif()

# Required libraries for the project
set(PROJECT_LIBS
    atom-io
    atom-error
    atom-function
    atom-system
    atom-utils
    atom-component
    loguru
    lithium-utils
    ${CMAKE_THREAD_LIBS_INIT}
    ${Seccomp_LIBRARIES}
)

if (NOT MINGW OR NOT WIN32)
set(PROJECT_LIBS ${PROJECT_LIBS}
    ${LIBGIT2_LIBRARIES}
)
endif()

# Create object library
add_library(${PROJECT_NAME}_OBJECT OBJECT ${PROJECT_SOURCES} ${PROJECT_HEADERS})

# Set object library property to be position independent code
set_property(TARGET ${PROJECT_NAME}_OBJECT PROPERTY POSITION_INDEPENDENT_CODE ON)

# Create static library
add_library(${PROJECT_NAME} STATIC $<TARGET_OBJECTS:${PROJECT_NAME}_OBJECT>)

# Set static library properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}  # Version number
    SOVERSION 1                 # Compatibility version
    OUTPUT_NAME ${PROJECT_NAME} # Output name
)

# Include directories so that project headers can be included
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link libraries required by the project
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_LIBS})

# Install target to install the static library to a specified location
install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
