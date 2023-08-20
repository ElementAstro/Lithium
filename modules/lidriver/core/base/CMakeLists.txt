cmake_minimum_required(VERSION 3.13)
project(lithiumdevice C CXX)

# Dependency
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})

add_library(${PROJECT_NAME} OBJECT "")

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    lithiumbase.h
    lithiumbasetypes.h

    basedevice.h
    parentdevice.h

    lithiumstandardproperty.h

    
)

list(APPEND ${PROJECT_NAME}_PRIVATE_HEADERS
    parentdevice_p.h
    basedevice_p.h

    watchdeviceproperty.h

    
)

# Sources
list(APPEND ${PROJECT_NAME}_SOURCES
    lithiumbase.cpp

    parentdevice.cpp
    basedevice.cpp
    watchdeviceproperty.cpp

    lithiumstandardproperty.cpp

    
)

# Setup Target
target_sources(${PROJECT_NAME}
    PUBLIC
    ${${PROJECT_NAME}_HEADERS}
    PRIVATE
    ${${PROJECT_NAME}_SOURCES}
    ${${PROJECT_NAME}_PRIVATE_HEADERS}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC . property ${CMAKE_CURRENT_BINARY_DIR}/../..
)

target_link_libraries(${PROJECT_NAME} lithiumcore)
target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES})

install(FILES
    ${${PROJECT_NAME}_HEADERS}
    DESTINATION
    ${INCLUDE_INSTALL_DIR}/liblithium
    COMPONENT Devel
)
