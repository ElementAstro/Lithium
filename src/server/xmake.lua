set_project("lithium.server")
set_version("1.0.0")
set_xmakever("2.5.1")

-- Set the C++ standard
set_languages("cxx17")

-- Add required packages
add_requires("openssl >=1.1", "oatpp", "oatpp-websocket", "oatpp-openssl", "oatpp-zlib")

-- Define libraries
local project_libs = {
    "oatpp",
    "oatpp-websocket",
    "oatpp-openssl",
    "oatpp-zlib",
    "openssl"
}

-- Include directories
add_includedirs(".")

-- Source and header files
local lib_files = {
    "AppComponent.hpp",
    "controller/FileController.hpp",
    "controller/RoomsController.hpp",
    "controller/StaticController.hpp",
    "controller/StatisticsController.hpp",
    "rooms/File.cpp",
    "rooms/File.hpp",
    "rooms/Peer.cpp",
    "rooms/Peer.hpp",
    "rooms/Room.cpp",
    "rooms/Room.hpp",
    "rooms/Lobby.cpp",
    "rooms/Lobby.hpp",
    "utils/Nickname.cpp",
    "utils/Nickname.hpp",
    "utils/Statistics.cpp",
    "utils/Statistics.hpp",
    "dto/DTOs.hpp",
    "dto/Config.hpp"
}

local exe_files = {
    "App.cpp"
}

-- Define certificates path
add_defines("CERT_PEM_PATH=\"$(projectdir)/../cert/test_key.pem\"")
add_defines("CERT_CRT_PATH=\"$(projectdir)/../cert/test_cert.crt\"")
add_defines("FRONT_PATH=\"$(projectdir)/../front\"")

-- Library target
target("lithium.server-lib")
    set_kind("static")
    add_files(table.unpack(lib_files))
    add_packages(table.unpack(project_libs))
target_end()

-- Executable target
target("lithium.server-exe")
    set_kind("binary")
    add_files(table.unpack(exe_files))
    add_deps("lithium.server-lib")
    add_packages(table.unpack(project_libs))
    add_defines("ENABLE_SERVER_STANDALONE")
    set_targetdir("$(buildir)/bin")
target_end()
