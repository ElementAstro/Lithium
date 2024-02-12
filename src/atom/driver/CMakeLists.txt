cmake_minimum_required(VERSION 3.20)
project(atom-driver C CXX)

find_package(Threads REQUIRED)

include_directories(.)
CHECK_INCLUDE_FILE(format HAS_STD_FORMAT)

if(NOT HAS_STD_FORMAT)
    find_package(fmt REQUIRED)
endif()
# Sources
list(APPEND ${PROJECT_NAME}_SOURCES
    fitskeyword.cpp

    device.cpp
    camera.cpp
    telescope.cpp
    focuser.cpp
    filterwheel.cpp
    guider.cpp
)

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    fitskeyword.h

    device.hpp
    camera.hpp
    telescope.hpp
    focuser.hpp
    filterwheel.hpp
    guider.hpp
    solver.hpp
)

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
target_link_libraries(${PROJECT_NAME}_OBJECT atom-component)
target_link_libraries(${PROJECT_NAME}_OBJECT atom-server)
add_library(${PROJECT_NAME} SHARED)

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