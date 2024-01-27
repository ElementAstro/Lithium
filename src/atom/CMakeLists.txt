cmake_minimum_required(VERSION 3.20)
project(atom C CXX)

CHECK_INCLUDE_FILE(format HAS_STD_FORMAT)

add_subdirectory(async)
add_subdirectory(io)
add_subdirectory(log)
add_subdirectory(type)
add_subdirectory(web)
add_subdirectory(components)
add_subdirectory(task)
add_subdirectory(driver)
add_subdirectory(utils)

if(NOT HAS_STD_FORMAT)
    find_package(fmt REQUIRED)
endif()
# Sources
list(APPEND ${PROJECT_NAME}_SOURCES
    error/error_stack.cpp

    io/compress.cpp
    io/file.cpp
    io/io.cpp

    log/logger.cpp
    log/global_logger.cpp

    search/sqlite.cpp

    server/daemon.cpp
    server/serialize.cpp
    server/deserialize.cpp
    server/json_checker.cpp
    server/global_ptr.cpp

    system/system.cpp
    system/crash.cpp
    system/process.cpp
    system/pid.cpp
    system/pidw.cpp
    # system/filew.cpp
    system/dirw.cpp

    type/args.cpp
    type/ini.cpp
    type/iparams.cpp
    type/message.cpp
    server/variables.cpp
)

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    error/error_code.hpp
    error/error_stack.hpp

    io/compress.hpp
    io/file.hpp
    io/glob.hpp
    io/io.hpp

    log/logger.hpp
    log/global_logger.hpp

    search/sqlite.hpp

    server/daemon.hpp
    server/serialize.hpp
    server/deserialize.hpp
    server/json_checker.hpp
    server/global_ptr.hpp
    server/commander.hpp
    server/message_bus.hpp
    server/message_queue.hpp
    server/variables.hpp

    system/system.hpp
    system/crash.hpp
    system/process.hpp
    system/pid.hpp
    system/pidw.hpp
    # system/filew.hpp
    system/dirw.hpp

    type/args.hpp
    type/json.hpp
    type/ini.hpp
    type/iparams.hpp
    type/small_vector.hpp
    type/message.hpp
)

# Private Headers
list(APPEND ${PROJECT_NAME}_PRIVATE_HEADERS

)

list(APPEND ${PROJECT_NAME}_LIBS
    loguru
    cpp_httplib
    libzippp
    atom-asyncstatic
    atom-utilsstatic
    atom-driverstatic
    atom-component
    atom-utilsstatic
    atom-webstatic
    )

# Build Object Library
add_library(${PROJECT_NAME}_OBJECT OBJECT)
set_property(TARGET ${PROJECT_NAME}_OBJECT PROPERTY POSITION_INDEPENDENT_CODE 1)

target_compile_definitions(${PROJECT_NAME}_OBJECT PRIVATE "-DHAVE_LIBNOVA")

if(WIN32)
target_link_libraries(${PROJECT_NAME}_OBJECT setupapi wsock32 ws2_32 shlwapi iphlpapi)
endif()
target_link_libraries(${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})

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