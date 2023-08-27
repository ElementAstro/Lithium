cmake_minimum_required(VERSION 3.13)
project(hydrogendevice C CXX)

# Dependency
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})

add_library(${PROJECT_NAME} OBJECT "")

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    hydrogenbase.h
    hydrogenbasetypes.h

    basedevice.h
    parentdevice.h

    hydrogenstandardproperty.h

    
)

list(APPEND ${PROJECT_NAME}_PRIVATE_HEADERS
    parentdevice_p.h
    basedevice_p.h

    watchdeviceproperty.h

    
)

# Sources
list(APPEND ${PROJECT_NAME}_SOURCES
    hydrogenbase.cpp

    parentdevice.cpp
    basedevice.cpp
    watchdeviceproperty.cpp

    hydrogenstandardproperty.cpp

    
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

target_link_libraries(${PROJECT_NAME} hydrogencore)
target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES})

install(FILES
    ${${PROJECT_NAME}_HEADERS}
    DESTINATION
    ${INCLUDE_INSTALL_DIR}/libhydrogen
    COMPONENT Devel
)

if(LINUX)
    set_target_properties(${PROJECT_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    )
endif()