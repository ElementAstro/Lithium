cmake_minimum_required(VERSION 3.20)
project(lithium_search C CXX)

option(ENABLE_DEBUG "Enable Debug Mode" OFF)
if(ENABLE_DEBUG)
    set(ENABLE_DEBUG_FLAG "1")
endif()

option(ENABLE_SQLITE_FLAG "Enable Sqlite" ON)
if (ENABLE_SQLITE_FLAG)
    find_package(SQLite3 REQUIRED)
    message("-- Found SQLite3 ${SQLite3_VERSION}: ${SQLite3_LIBRARIES}")
    set(ENABLE_SQLITE "1")
else()
    message("-- SQLite3 Not Found. Will not build sqlite search module.")
endif()

option(ENABLE_MYSQL_FLAG "Enable MySQL" OFF)
if (ENABLE_MYSQL_FLAG)
    find_package(MySQL REQUIRED)
    message("-- Found MySQL ${MySQL_VERSION}: ${MySQL_LIBRARIES}")
    set(ENABLE_MYSQL "1")
else()
    message("-- MySQL Not Found. Will not build mysql search module.")
endif()

option(ENABLE_POSTGRESQL_FLAG "Enable PostgreSQL" OFF)
if (ENABLE_POSTGRESQL_FLAG)
    find_package(PostgreSQL REQUIRED)
    message("-- Found PostgreSQL ${PostgreSQL_VERSION}: ${PostgreSQL_LIBRARIES}")
    set(ENABLE_POSTGRESQL "1")
else()
    message("-- PostgreSQL Not Found. Will not build postgresql search module.")
endif()

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

CHECK_INCLUDE_FILE(format HAS_STD_FORMAT)

if(NOT HAS_STD_FORMAT)
    find_package(fmt REQUIRED)
endif()

list(APPEND ${PROJECT_NAME}_LIBS
    lithiumpluginstatic
    loguru
    ${CFITSIO_LIBRARIES}
)

if(ENABLE_SLQLITE_FLAG)
    list(APPEND ${PROJECT_NAME}_LIBS
        ${SQLite3_LIBRARIES}
    )
endif()

if(ENABLE_MYSQL_FLAG)
    list(APPEND ${PROJECT_NAME}_LIBS
        ${MySQL_LIBRARIES}
    )
endif()

if(ENABLE_POSTGRESQL_FLAG)
    list(APPEND ${PROJECT_NAME}_LIBS
        ${PostgreSQL_LIBRARIES}
    )
endif()
# Sources
list(APPEND ${PROJECT_NAME}_SOURCES
    main.cpp

    ongc/dso.cpp
    ongc/ongc.cpp

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