cmake_minimum_required(VERSION 3.20)

set(project_name lithium.server)

project(${project_name})

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

add_library(${project_name}-lib
        App.cpp
        AppComponent.hpp
        controller/FileController.hpp
        controller/RoomsController.hpp
        controller/StaticController.hpp
        controller/StatisticsController.hpp
        rooms/File.cpp
        rooms/File.hpp
        rooms/Peer.cpp
        rooms/Peer.hpp
        rooms/Room.cpp
        rooms/Room.hpp
        rooms/Lobby.cpp
        rooms/Lobby.hpp
        utils/Nickname.cpp
        utils/Nickname.hpp
        utils/Statistics.cpp
        utils/Statistics.hpp
        dto/DTOs.hpp
        dto/Config.hpp
)

## link libs
find_package(OpenSSL 1.1 REQUIRED)

target_link_libraries(${project_name}-lib

        # Oat++ libraries
        PUBLIC oatpp
        PUBLIC oatpp-websocket
        PUBLIC oatpp-openssl
        PUBLIC oatpp-zlib

        # OpenSSL libraries
        PUBLIC OpenSSL::SSL
        PUBLIC OpenSSL::Crypto

)

#################################################################
## define certificates path

add_definitions(
        -DCERT_PEM_PATH="${CMAKE_CURRENT_LIST_DIR}/../cert/test_key.pem"
        -DCERT_CRT_PATH="${CMAKE_CURRENT_LIST_DIR}/../cert/test_cert.crt"
        -DFRONT_PATH="${CMAKE_CURRENT_LIST_DIR}/../front"
)

#################################################################
## add executables

# add_executable(${project_name}-exe
#        App.cpp
#)
#target_link_libraries(${project_name}-exe ${project_name}-lib)
#add_dependencies(${project_name}-exe ${project_name}-lib)
#target_compile_definitions(
#       ${project_name}-exe
#        PRIVATE ENABLE_SERVER_STANDALONE
#)
