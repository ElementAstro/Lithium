cmake_minimum_required(VERSION 3.13)
project(lithiumcore C CXX)

find_package(Threads REQUIRED)

add_subdirectory(plugin)

include_directories(.)
include_directories(base)
include_directories(property)
include_directories(utils)
CHECK_INCLUDE_FILE(format HAS_STD_FORMAT)

if(NOT HAS_STD_FORMAT)
    find_package(fmt REQUIRED)
endif()
# Sources
list(APPEND ${PROJECT_NAME}_SOURCES
    fitskeyword.cpp

    device.cpp
    deviceio.cpp
    camera.cpp
    telescope.cpp
    focuser.cpp
    filterwheel.cpp
    guider.cpp
    network/fifoclient.cpp
    network/fifoserver.cpp
    util/utils.cpp
    event/eventloop.cpp

    property/base64.cpp
    property/iproperty.cpp
    property/iphoto.cpp
    property/uuid.cpp

    property/task/conditional_task.cpp
    property/task/daemon_task.cpp
    property/task/loop_task.cpp
    property/task/device_task.cpp
    property/task/plugin_task.cpp
    property/task/task.cpp
)

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    fitskeyword.h

    device.hpp
    deviceio.hpp
    camera.hpp
    telescope.hpp
    focuser.hpp
    filterwheel.hpp
    guider.hpp
    solver.hpp
    network/fifoclient.hpp
    network/fifoserver.hpp
    util/utils.hpp

    property/base64.hpp
    property/iproperty.hpp
    property/iphoto.hpp
    property/sha256.hpp
    property/uuid.hpp

    property/task/conditional_task.hpp
    property/task/device_task.hpp
    property/task/daemon_task.hpp
    property/task/loop_task.hpp
    property/task/task.hpp
)

# Private Headers
list(APPEND ${PROJECT_NAME}_PRIVATE_HEADERS
    # TODO
)

# Build Object Library
add_library(${PROJECT_NAME}_OBJECT OBJECT)
set_property(TARGET ${PROJECT_NAME}_OBJECT PROPERTY POSITION_INDEPENDENT_CODE 1)

target_compile_definitions(${PROJECT_NAME}_OBJECT PRIVATE "-DHAVE_LIBNOVA")

if(WIN32)
target_link_libraries(${PROJECT_NAME}_OBJECT setupapi wsock32 ws2_32 shlwapi iphlpapi)
endif()

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