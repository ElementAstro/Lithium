cmake_minimum_required(VERSION 3.20)
project(lithium.client.indi_camera)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(${CMAKE_SOURCE_DIR}/cmake_modules/ScanModule.cmake)

# Add source files
set(SOURCE_FILES
    camera.cpp
)

set(LIBS
    loguru
    atom-system
    atom-io
    atom-utils
    atom-component
    atom-error
    lithium-device-template
)

if (NOT WIN32)

find_package(INDI 2.0 REQUIRED)

endif()

# Create the module library
add_library(lithium.client.indi_camera SHARED ${SOURCE_FILES} ${HEADER_FILES})
set_property(TARGET lithium.client.indi_camera PROPERTY POSITION_INDEPENDENT_CODE 1)
target_link_libraries(lithium.client.indi_camera PUBLIC ${LIBS} indiclient)
