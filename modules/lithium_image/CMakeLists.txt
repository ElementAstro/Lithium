cmake_minimum_required(VERSION 3.20)
project(lithium_image C CXX)

CHECK_INCLUDE_FILE(format HAS_STD_FORMAT)

if(NOT HAS_STD_FORMAT)
    find_package(fmt REQUIRED)
endif()

list(APPEND ${PROJECT_NAME}_LIBS
    lithiumpluginstatic
    loguru
    ${CFITSIO_LIBRARIES}
)

# Sources
list(APPEND ${PROJECT_NAME}_SOURCES
    lithium_image.cpp
    lithium_image_main.cpp
)

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    lithium_image.hpp
)

# Private Headers
list(APPEND ${PROJECT_NAME}_PRIVATE_HEADERS

)

# Build Object Library
add_library(${PROJECT_NAME}_OBJECT OBJECT)
set_property(TARGET ${PROJECT_NAME}_OBJECT PROPERTY POSITION_INDEPENDENT_CODE 1)

target_link_libraries(${PROJECT_NAME}_OBJECT loguru lithiumpluginstatic)

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

add_library(${PROJECT_NAME}shared SHARED)

target_link_libraries(${PROJECT_NAME}shared ${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})
target_link_libraries(${PROJECT_NAME}shared ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(${PROJECT_NAME}shared PUBLIC .)
set_target_properties(${PROJECT_NAME}shared PROPERTIES
    VERSION ${CMAKE_HYDROGEN_VERSION_STRING}
    SOVERSION ${HYDROGEN_SOVERSION}
    OUTPUT_NAME ${PROJECT_NAME} # this same name like shared library - backwards compatibility
)
install(TARGETS ${PROJECT_NAME}shared
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)