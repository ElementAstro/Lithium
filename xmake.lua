-- xmake.lua for Lithium
-- This project is licensed under the terms of the GPL3 license.
--
-- Project Name: Lithium
-- Description: Lithium - Open Astrophotography Terminal
-- Author: Max Qian
-- License: GPL3

set_project("Lithium")
set_languages("c++20")

if is_plat("macosx") then
    add_shflags("-undefined dynamic_lookup")
end

set_version("1.0.0")

add_includedirs("libs/", "driverlibs/", "src/", "src/atom", "libs/oatpp", "libs/oatpp-swagger", "libs/oatpp-websocket", {public = true})

add_defines("ENABLE_ASYNC_FLAG=1", "ENABLE_DEBUG_FLAG=1", "ENABLE_NATIVE_SERVER_FLAG=1", "ENABLE_FASHHASH_FLAG=1", "ENABLE_WEB_SERVER_FLAG=1", "ENABLE_WEB_CLIENT_FLAG=1")

if is_plat("windows") then
    set_installdir("C:/Program Files/LithiumServer")
elseif is_plat("linux") then
    set_installdir("/usr/lithium")
end

target("lithium_server-library")
    set_kind("static")
    add_files("src/device/server/*.cpp", "src/device/manager.cpp", "src/device/utils/utils.cpp", "src/addon/*.cpp", "src/config/configor.cpp", 
              "src/debug/terminal.cpp", "src/script/*.cpp", "src/script/custom/*.cpp", "src/task/*.cpp", "src/LithiumApp.cpp")
    add_packages("openssl", "cfitsio", "zlib", "sqlite3", "fmt")
    add_deps("atomstatic", "loguru", "libzippp", "oatpp", "oatpp-websocket", "oatpp-swagger", "oatpp-openssl", "oatpp-zlib", "cpp_httplib", "backward", "tinyxml2", "pocketpy")

target("lithium_server")
    set_kind("binary")
    add_files("src/app.cpp")
    add_deps("lithium_server-library")
    add_defines("LOGURU_DEBUG_LOGGING")
    add_packages("openssl", "cfitsio", "zlib", "sqlite3", "fmt")
    add_deps("atomstatic", "loguru", "libzippp", "oatpp", "oatpp-websocket", "oatpp-swagger", "oatpp-openssl", "oatpp-zlib", "cpp_httplib", "backward", "tinyxml2", "pocketpy")
    if is_plat("windows") then
        add_syslinks("pdh", "iphlpapi", "winmm", "crypt32", "wsock32", "ws2_32")
        add_requires("dlfcn-win32")
    else
        add_syslinks("dl")
    end
    set_targetdir("$(buildir)")
    set_filename("lithium_server")

includes("libs", "tools", "modules", "driver")