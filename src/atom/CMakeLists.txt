# CMakeLists.txt for Atom
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom
# Description: Atom Library for all of the Element Astro Project
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom C CXX)

CHECK_INCLUDE_FILE(format HAS_STD_FORMAT)

add_subdirectory(async)
add_subdirectory(components)
add_subdirectory(connection)
add_subdirectory(driver)
add_subdirectory(event)
add_subdirectory(io)
add_subdirectory(log)
add_subdirectory(server)
add_subdirectory(search)
add_subdirectory(system)
add_subdirectory(task)
add_subdirectory(type)
add_subdirectory(utils)
add_subdirectory(web)

if(NOT HAS_STD_FORMAT)
    find_package(fmt REQUIRED)
endif()
# Sources
list(APPEND ${PROJECT_NAME}_SOURCES
    error/error_stack.cpp

    log/logger.cpp
    log/global_logger.cpp
)

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    error/error_code.hpp
    error/error_stack.hpp

    log/logger.hpp
    log/global_logger.hpp
)

# Private Headers
list(APPEND ${PROJECT_NAME}_PRIVATE_HEADERS

)

list(APPEND ${PROJECT_NAME}_LIBS
    loguru
    cpp_httplib
    libzippp
    atom-async
    atom-task
    atom-io
    atom-driver
    atom-event
    atom-component
    atom-type
    atom-utils
    atom-search
    atom-web
    atom-system
    atom-server
    )

# Build Object Library
add_library(${PROJECT_NAME}_OBJECT OBJECT)
set_property(TARGET ${PROJECT_NAME}_OBJECT PROPERTY POSITION_INDEPENDENT_CODE 1)

target_compile_definitions(${PROJECT_NAME}_OBJECT PRIVATE "-DHAVE_LIBNOVA")

if(WIN32)
target_link_libraries(${PROJECT_NAME}_OBJECT setupapi wsock32 ws2_32 shlwapi iphlpapi)
endif()
target_link_libraries(${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})

target_sources(${PROJECT_NAME}_OBJECT
    PUBLIC
    ${${PROJECT_NAME}_HEADERS}
    PRIVATE
    ${${PROJECT_NAME}_SOURCES}
    ${${PROJECT_NAME}_PRIVATE_HEADERS}
)

target_link_libraries(${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})

add_library(${PROJECT_NAME}static STATIC)

target_link_libraries(${PROJECT_NAME}static ${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})
target_link_libraries(${PROJECT_NAME}static ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(${PROJECT_NAME}static PUBLIC .)

set_target_properties(${PROJECT_NAME}static PROPERTIES
    VERSION ${CMAKE_HYDROGEN_VERSION_STRING}
    SOVERSION ${HYDROGEN_SOVERSION}
    OUTPUT_NAME ${PROJECT_NAME} # this same name like shared library - backwards compatibility
)

install(TARGETS ${PROJECT_NAME}static
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)