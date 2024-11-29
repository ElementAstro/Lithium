cmake_minimum_required(VERSION 3.20)

set(project_name lithium.server)

project(${project_name})

include_directories(.)

# 添加源文件列表
set(SOURCES
    App.cpp
    rooms/File.cpp
    rooms/Peer.cpp
    rooms/Room.cpp
    rooms/Lobby.cpp
    utils/Nickname.cpp
    utils/Statistics.cpp
)

# 添加头文件列表
set(HEADERS
    AppComponent.hpp
    controller/FileController.hpp
    controller/RoomsController.hpp
    controller/StaticController.hpp
    controller/StatisticsController.hpp
    rooms/File.hpp
    rooms/Peer.hpp
    rooms/Room.hpp
    rooms/Lobby.hpp
    utils/Nickname.hpp
    utils/Statistics.hpp
    dto/DTOs.hpp
    dto/Config.hpp
)

add_library(${project_name}-lib STATIC ${SOURCES} ${HEADERS})

target_link_libraries(${project_name}-lib
    PUBLIC oatpp
    PUBLIC oatpp-websocket
    PUBLIC oatpp-openssl
    PUBLIC oatpp-zlib
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

# add_executable(${project_name}-exe ${SOURCES})
# target_link_libraries(${project_name}-exe ${project_name}-lib)
# add_dependencies(${project_name}-exe ${project_name}-lib)
# target_compile_definitions(
#     ${project_name}-exe
#     PRIVATE ENABLE_SERVER_STANDALONE
# )