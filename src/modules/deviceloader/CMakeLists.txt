cmake_minimum_required(VERSION 3.13)
project(hydrogenserver CXX)

set(server_SRC
    client_info.cpp
    concurrent.cpp
    driver_info.cpp
    fifo_server.cpp
    io.cpp
    message_queue.cpp
    message.cpp
    property.cpp
    serialize.cpp
    tcp_server.cpp
    time.cpp
    xml_util.cpp
    signal.cpp
    remote_driver.cpp
    local_driver.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/src/core)
include_directories(${CMAKE_SOURCE_DIR}/src/core/base)
include_directories(${CMAKE_SOURCE_DIR}/src/core/io)
include_directories(${CMAKE_SOURCE_DIR}/src/core/property)
include_directories(${CMAKE_SOURCE_DIR}/src/core/timer)
include_directories(${CMAKE_SOURCE_DIR}/src/core/thread)

if(ANDROID)
    message(WARNING "HYDROGEN Server is only supported under Linux, BSD, MacOS, and Cygwin while current system is " ${CMAKE_SYSTEM_NAME})
else()
    find_package(Threads REQUIRED)
    find_package(Libev REQUIRED)
    find_package(UV REQUIRED)
    find_package(fmt REQUIRED)
    find_package(ZLIB REQUIRED)

    add_executable(${PROJECT_NAME} ${server_SRC} hydrogen_server.cpp)

    # target_compile_definitions(${PROJECT_NAME} PRIVATE USE_LIBUV=1)
    target_compile_definitions(${PROJECT_NAME} PRIVATE MAIN_FUNC=1)

    target_link_libraries(hydrogenserver lithiumcorestatic hydrogencore ${CMAKE_THREAD_LIBS_INIT} ${LIBEV_LIBRARIES})
    target_include_directories(hydrogenserver SYSTEM PRIVATE ${LIBEV_INCLUDE_DIRS})

    target_link_libraries(hydrogenserver loguru fmt ${UV_LIBRARY} backward)

    target_link_libraries(hydrogenserver ${ZLIB_LIBRARIES})

    set_target_properties(
        hydrogenserver
        PROPERTIES
        OUTPUT_NAME ../../../hydrogenserver
    )

    install(TARGETS hydrogenserver RUNTIME DESTINATION bin)
endif(ANDROID)
