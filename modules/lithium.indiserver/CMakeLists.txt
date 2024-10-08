# CMakeLists.txt for lithium.indiserver
# This project is licensed under the terms of the GPL3 license.
#
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(lithium.indiserver)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add source and header files in one place for better management
set(SOURCE_FILES
    src/driverlist.cpp
    src/indiserver.cpp
    src/collection.cpp
    src/iconnector.cpp
)

set(HEADER_FILES
    include/collection.hpp
    include/driverlist.hpp
    include/indiserver.hpp
    include/collection.hpp
    include/iconnector.hpp
)

# Specify the external libraries
set(LIBS
    loguru
    atom-system
    atom-io
    tinyxml2
    atom-component
    atom-error
    atom-utils
)

if (WIN32)
    set(LIBS ${LIBS} ws2_32 version iphlpapi)
endif()

# Create the module library
add_library(lithium.indiserver SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(lithium.indiserver PUBLIC ${LIBS})

# Include directories
target_include_directories(lithium.indiserver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Set target properties for the library
set_target_properties(lithium.indiserver PROPERTIES
    PREFIX ""
    SUFFIX ".so"
    VERSION "1.0.0"
)

# Option to set installation directory
set(INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib")
install(TARGETS lithium.indiserver
    LIBRARY DESTINATION ${INSTALL_DIR}
)

# Install header files
install(FILES ${HEADER_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/include")
