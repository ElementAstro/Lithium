# CMakeLists.txt for Lithium
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Lithium-WebServer
# Description: Oatpp http and websocket for Lithium
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)

project(lithium_webserver C CXX)

set(server_websocket_module
    websocket/Hub.cpp
    websocket/Connection.cpp
    websocket/Registry.cpp
    websocket/Session.cpp
)

set(server_module
    App.cpp
	AppComponent.hpp
	ErrorHandler.cpp
    Runner.cpp

    config/HubsConfig.cpp
)

#################################################################################
# Main

add_library(${PROJECT_NAME} STATIC ${server_websocket_module} ${server_module})
target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/libs)

target_link_libraries(${PROJECT_NAME} PRIVATE oatpp-websocket oatpp-swagger oatpp-openssl oatpp-zlib oatpp)
target_link_libraries(${PROJECT_NAME} PRIVATE loguru fmt::fmt)
target_link_libraries(${PROJECT_NAME} PRIVATE atomstatic)
