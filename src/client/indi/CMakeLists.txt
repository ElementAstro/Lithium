cmake_minimum_required(VERSION 3.20)
project(lithium.client.indi_modules)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(${CMAKE_SOURCE_DIR}/cmake/ScanModule.cmake)

# Common libraries for all modules
set(COMMON_LIBS
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
    set(COMMON_LIBS ${COMMON_LIBS} indiclient)
endif()

# Define function to create a target
function(create_indi_module MODULE_NAME SOURCE_FILES)
    add_library(${MODULE_NAME} SHARED ${SOURCE_FILES})
    set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
    target_link_libraries(${MODULE_NAME} PUBLIC ${COMMON_LIBS})
endfunction()

# Add source files for each module and create targets

# Camera module
#set(CAMERA_SOURCES
#    camera.cpp
#)
#create_indi_module(lithium.client.camera_indi "${CAMERA_SOURCES}")

# Telescope module
set(TELESCOPE_SOURCES
    telescope.cpp
)
create_indi_module(lithium.client.telescope_indi "${TELESCOPE_SOURCES}")

# Focuser module
set(FOCUSER_SOURCES
    focuser.cpp
)
create_indi_module(lithium.client.focuser_indi "${FOCUSER_SOURCES}")

# Filterwheel module
set(FILTERWHEEL_SOURCES
    filterwheel.cpp
)
create_indi_module(lithium.client.filterwheel_indi "${FILTERWHEEL_SOURCES}")
