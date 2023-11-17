cmake_minimum_required(VERSION 3.13)
project(hydrogencore C CXX)

find_package(Nova)
add_library(${PROJECT_NAME} OBJECT "")

include(CPack)

set(CMAKE_CXX_STANDARD 20)

configure_file(hydrogenapi.h.in ${CMAKE_BINARY_DIR}/hydrogenapi.h @ONLY)

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    ${CMAKE_BINARY_DIR}/hydrogenapi.h
    hydrogendevapi.hpp
    hydrogenutility.hpp
    lilxml.hpp
    base64.hpp
    hydrogencom.hpp
    sharedblob.hpp

    hydrogenbase.h
    hydrogenbasetypes.h

    basedevice.h
    parentdevice.h

    hydrogenstandardproperty.h
)

list(APPEND ${PROJECT_NAME}_PRIVATE_HEADERS
    base64_luts.hpp
    hydrogenlilxml.hpp
    hydrogenuserio.hpp
    userio.cpp

    parentdevice_p.h
    basedevice_p.h

    watchdeviceproperty.h
)

# Sources
list(APPEND ${PROJECT_NAME}_SOURCES
    hydrogenutility.cpp
    base64.cpp
    userio.cpp
    hydrogencom.cpp
    hydrogendevapi.cpp
    lilxml.cpp
    hydrogenuserio.cpp
    sharedblob.cpp

    hydrogenbase.cpp

    parentdevice.cpp
    basedevice.cpp
    watchdeviceproperty.cpp

    hydrogenstandardproperty.cpp
)

if(UNIX OR WIN32 OR LINUX)
    list(APPEND ${PROJECT_NAME}_PRIVATE_HEADERS
        sharedblob_parse.hpp
        shm_open_anon.hpp)
    list(APPEND ${PROJECT_NAME}_SOURCES
        sharedblob_parse.cpp
        shm_open_anon.cpp)
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
        libastro.hpp
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
    ${CMAKE_CURRENT_BINARY_DIR}       # hydrogenapi.h
)

install(FILES
    ${${PROJECT_NAME}_HEADERS}
    DESTINATION
    ${INCLUDE_INSTALL_DIR}/libhydrogen
    COMPONENT Devel
)
