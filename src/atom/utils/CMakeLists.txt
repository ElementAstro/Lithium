cmake_minimum_required(VERSION 3.20)
project(atom-utils C CXX)

# Sources
set(${PROJECT_NAME}_SOURCES
    aes.cpp
    base64.cpp
    env.cpp
    guid.cpp
    hash_util.cpp
    huffman.cpp
    math.cpp
    random.cpp
    string.cpp
    static_switch.cpp
    time.cpp
    uuid.cpp
)

# Headers
set(${PROJECT_NAME}_HEADERS
    aes.hpp
    base64.hpp
    env.hpp
    exception.hpp
    guid.hpp
    hash_util.hpp
    huffman.hpp
    lock.hpp
    math.hpp
    random.hpp
    refl.hpp
    refl_clang.hpp
    sha256.hpp
    string.hpp
    static_switch.hpp
    stopwatcher.hpp
    switch.hpp
    time.hpp
    type.hpp
    uuid.hpp
)

# Private Headers
set(${PROJECT_NAME}_PRIVATE_HEADERS
)

# Build Object Library
add_library(${PROJECT_NAME}_OBJECT OBJECT)
set_property(TARGET ${PROJECT_NAME}_OBJECT PROPERTY POSITION_INDEPENDENT_CODE 1)

target_link_libraries(${PROJECT_NAME}_OBJECT loguru)

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