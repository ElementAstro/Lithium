# CMakeLists.txt for Atom-Connection
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-Connection
# Description: Connection Between Lithium Drivers, TCP and IPC
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-connnection C CXX)

# Sources
list(APPEND ${PROJECT_NAME}_SOURCES
    fifoclient.cpp
    fifoserver.cpp
    sockethub.cpp
    tcpclient.cpp
    udp_server.cpp
    udpclient.cpp
)

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    fifoclient.hpp
    fifoserver.hpp
    sockethub.hpp
    tcpclient.hpp
    udp_server.hpp
    udpclient.hpp
)

if (ENABLE_LIBSSH)
list(APPEND ${PROJECT_NAME}_SOURCES
    sshclient.cpp
    sshserver.cpp
)
list(APPEND ${PROJECT_NAME}_HEADERS
    sshclient.hpp
    sshserver.hpp
)
endif()

set(${PROJECT_NAME}_LIBS
    loguru
    ${CMAKE_THREAD_LIBS_INIT}
)

if (ENABLE_SSH)
find_package(LibSSH REQUIRED)
list(APPEND ${PROJECT_NAME}_LIBS
    ${LIBSSH_LIBRARIES}
)
link_directories(${LIBSSH_LIBRARY_DIRS})
endif()

# Build Object Library
add_library(${PROJECT_NAME}_OBJECT OBJECT)
set_property(TARGET ${PROJECT_NAME}_OBJECT PROPERTY POSITION_INDEPENDENT_CODE 1)

target_sources(${PROJECT_NAME}_OBJECT
    PUBLIC
    ${${PROJECT_NAME}_HEADERS}
    PRIVATE
    ${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})

add_library(${PROJECT_NAME} STATIC)

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})
target_include_directories(${PROJECT_NAME} PUBLIC .)



set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${CMAKE_HYDROGEN_VERSION_STRING}
    SOVERSION ${HYDROGEN_SOVERSION}
    OUTPUT_NAME ${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if (ATOM_BUILD_PYTHON)
pybind11_add_module(${PROJECT_NAME}-py _pybind.cpp)
target_link_libraries(${PROJECT_NAME}-py PRIVATE ${PROJECT_NAME})
if (WIN32)
target_link_libraries(${PROJECT_NAME}-py PRIVATE ws2_32)
endif()
endif()
