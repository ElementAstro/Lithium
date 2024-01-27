cmake_minimum_required(VERSION 3.20)
project(atom-web C CXX)

# Sources
set(${PROJECT_NAME}_SOURCES
    address.cpp
    downloader.cpp
    httpclient.cpp
    httplite.cpp
    utils.cpp
    time.cpp
)

# Headers
set(${PROJECT_NAME}_HEADERS
    address.hpp
    downloader.hpp
    httpclient.hpp
    httplite.hpp
    utils.hpp
    time.hpp
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

find_package(OpenSSL REQUIRED)
add_library(cpp_httplib httplib.cpp)
target_link_libraries(cpp_httplib OpenSSL::SSL OpenSSL::Crypto)
if(WIN32)
target_link_libraries(cpp_httplib crypt32 wsock32 ws2_32)
endif()
if(LINUX)
    set_target_properties(cpp_httplib PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    )
endif()
