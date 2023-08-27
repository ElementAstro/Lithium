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
)

if(ANDROID)
    message(WARNING "INDI Server is only supported under Linux, BSD, MacOS, and Cygwin while current system is " ${CMAKE_SYSTEM_NAME})
else()
    find_package(Threads REQUIRED)
    find_package(Libev REQUIRED)

    add_executable(${PROJECT_NAME} ${server_SRC} hydrogen_server.cpp)

    target_link_libraries(hydrogenserver hydrogencore ${CMAKE_THREAD_LIBS_INIT} ${LIBEV_LIBRARIES})
    target_include_directories(hydrogenserver SYSTEM PRIVATE ${LIBEV_INCLUDE_DIRS})

    install(TARGETS hydrogenserver RUNTIME DESTINATION bin)
endif(ANDROID)
