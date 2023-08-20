cmake_minimum_required(VERSION 3.13)
project(lithiumcore C CXX)

find_package(Nova)

add_library(${PROJECT_NAME} OBJECT "")

include(CPack)

configure_file(lithiumapi.h.in lithiumapi.h @ONLY)

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    ${CMAKE_CURRENT_BINARY_DIR}/lithiumapi.h
    lithiumdevapi.h
    lithiumutility.h
    lilxml.h
    base64.h
    lithiumcom.h
    sharedblob.h
)

list(APPEND ${PROJECT_NAME}_PRIVATE_HEADERS
    base64_luts.h
    lithiumlilxml.h
    lithiumuserio.h
    userio.h
)

# Sources
list(APPEND ${PROJECT_NAME}_SOURCES
    lithiumutility.cpp
    base64.c
    userio.c
    lithiumcom.cpp
    lithiumdevapi.c
    lilxml.cpp
    lithiumuserio.c
)

if(UNIX OR WIN32)
    list(APPEND ${PROJECT_NAME}_PRIVATE_HEADERS
        sharedblob_parse.h
        shm_open_anon.h)
    list(APPEND ${PROJECT_NAME}_SOURCES
        sharedblob.c
        sharedblob_parse.cpp
        shm_open_anon.c)
endif()

target_compile_definitions(${PROJECT_NAME}
  PUBLIC
    $<$<BOOL:${NOVA_FOUND}>:HAVE_LIBNOVA>
    $<$<BOOL:${HAVE_TIMESPEC_GET}>:HAVE_TIMESPEC_GET>
    $<$<BOOL:${HAVE_CLOCK_GETTIME}>:HAVE_CLOCK_GETTIME>
)
if(NOVA_FOUND)
    target_link_libraries(${PROJECT_NAME} ${NOVA_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${NOVA_INCLUDE_DIR})

    list(APPEND ${PROJECT_NAME}_HEADERS
        libastro.h
    )

    list(APPEND ${PROJECT_NAME}_SOURCES
        libastro.cpp
    )
endif()

# Setup Target
target_sources(${PROJECT_NAME}
    PUBLIC
    ${${PROJECT_NAME}_HEADERS}
    PRIVATE
    ${${PROJECT_NAME}_SOURCES}
    ${${PROJECT_NAME}_PRIVATE_HEADERS}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    .
    ${CMAKE_CURRENT_BINARY_DIR}/../.. # config.h
    ${CMAKE_CURRENT_BINARY_DIR}       # lithiumapi.h
)

install(FILES
    ${${PROJECT_NAME}_HEADERS}
    DESTINATION
    ${INCLUDE_INSTALL_DIR}/liblithium
    COMPONENT Devel
)
