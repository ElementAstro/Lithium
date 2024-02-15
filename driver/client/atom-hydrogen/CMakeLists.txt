cmake_minimum_required(VERSION 3.13)
project(atom-hydrogen C CXX)

include_directories(${CMAKE_SOURCE_DIR}/modules/hydrogen_core)
include_directories(${CMAKE_SOURCE_DIR}/modules/hydrogen_core/base)
include_directories(${CMAKE_SOURCE_DIR}/modules/hydrogen_core/property)
include_directories(${CMAKE_SOURCE_DIR}/modules/hydrogen_core/util)

list(APPEND ${PROJECT_NAME}_SOURCES
    hydrogencamera.cpp
    hydrogendome.cpp
    hydrogenfilterwheel.cpp
    hydrogenfocuser.cpp
    hydrogentelescope.cpp
)

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    hydrogencamera.hpp
    hydrogenbasic.hpp
    hydrogendome.hpp
    hydrogenfilterwheel.hpp
    hydrogenfocuser.hpp
    hydrogentelescope.hpp
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
target_link_libraries(${PROJECT_NAME}_OBJECT atom-driver hydrogenclient)
add_library(${PROJECT_NAME} STATIC)

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(${PROJECT_NAME} PUBLIC .)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${CMAKE_HYDROGEN_VERSION_STRING}
    SOVERSION ${HYDROGEN_SOVERSION}
    OUTPUT_NAME ${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)