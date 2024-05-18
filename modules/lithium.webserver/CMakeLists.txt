# CMakeLists.txt for lithium.webserver
# This project is licensed under the terms of the GPL3 license.
#
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(lithium.webserver)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

set(server_websocket_module
    src/websocket/Hub.cpp
    src/websocket/Connection.cpp
    src/websocket/Registry.cpp
    src/websocket/Session.cpp
)

set(server_module
    src/App.cpp
	src/ErrorHandler.cpp
    src/Runner.cpp

    src/config/HubsConfig.cpp
)

# Add source files
set(SOURCE_FILES
    src/webserver.cpp
    _component.cpp
    _main.cpp
)
# Create the module library
add_library(lithium.webserver SHARED ${SOURCE_FILES} ${server_websocket_module} ${server_module})

target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/libs)

target_link_libraries(${PROJECT_NAME} PRIVATE oatpp-websocket oatpp-swagger oatpp-openssl oatpp-zlib oatpp)
target_link_libraries(${PROJECT_NAME} PRIVATE loguru fmt::fmt)
target_link_libraries(${PROJECT_NAME} PRIVATE atomstatic)

# Include directories
target_include_directories(lithium.webserver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
