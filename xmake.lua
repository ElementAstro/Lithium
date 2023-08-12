-- xmake.lua for Lithium
-- This project is licensed under the terms of the GPL3 license.
--
-- Project Name: Lithium
-- Description: Open Astrophotography Terminal
-- Author: Max Qian
-- License: GPL3

add_rules("mode.release", "mode.debug")

-- root directory of the project
set(Lithium_PROJECT_ROOT_DIR, "$(projectdir)")
set(lithium_src_dir, "$(projectdir)/src/")

-- compiler options
if is_plat("windows") then
    add_defines("WIN32")
    add_cxxflags("/EHsc")
elseif is_plat("macosx") or is_plat("linux") then
    add_defines("UNIX")
end
add_defines("USE_FOLDERS")

-- main project
-- this should appear after setting the architecture
target("Lithium")

    set_targetdir("$(projectdir)/build")
    set_objectdir("$(projectdir)/build/$(plat)")

    -- include directories
    add_includedirs(
        "$(projectdir)/libs/",
        "$(lithium_src_dir)",
        "$(lithium_src_dir)/modules",
        "$(projectdir)/src/",
        "$(projectdir)/libs/oatpp",
        "$(projectdir)/libs/oatpp-swagger",
        "$(projectdir)/libs/oatpp-websocket"
    )

    -- source files
    add_files(
        "$(lithium_src_dir)/api/astap.cpp",
        "$(lithium_src_dir)/api/astap.hpp",
        "$(lithium_src_dir)/api/astrometry.cpp",
        "$(lithium_src_dir)/api/astrometry.hpp",
        "$(lithium_src_dir)/api/phd2client.cpp",
        "$(lithium_src_dir)/api/phd2client.hpp",
        "$(lithium_src_dir)/modules/config/configor.cpp",
        "$(lithium_src_dir)/modules/config/configor.hpp",
        "$(lithium_src_dir)/database/database.cpp",
        "$(lithium_src_dir)/database/database.hpp",
        "$(lithium_src_dir)/debug/terminal.cpp",
        "$(lithium_src_dir)/debug/terminal.hpp",
        "$(lithium_src_dir)/modules/device/device.cpp",
        "$(lithium_src_dir)/modules/device/device.hpp",
        "$(lithium_src_dir)/modules/device/device_manager.cpp",
        "$(lithium_src_dir)/modules/device/device_manager.hpp",
        "$(lithium_src_dir)/api/indiclient.cpp",
        "$(lithium_src_dir)/api/indiclient.hpp",
        "$(lithium_src_dir)/driver/indi/indi_exception.hpp",
        "$(lithium_src_dir)/driver/indi/indicamera.cpp",
        "$(lithium_src_dir)/driver/indi/indicamera.hpp",
        "$(lithium_src_dir)/driver/indi/indifocuser.cpp",
        "$(lithium_src_dir)/driver/indi/indifocuser.hpp",
        "$(lithium_src_dir)/driver/indi/inditelescope.cpp",
        "$(lithium_src_dir)/driver/indi/inditelescope.hpp",
        "$(lithium_src_dir)/driver/indi/indifilterwheel.cpp",
        "$(lithium_src_dir)/driver/indi/indifilterwheel.hpp",
        "$(lithium_src_dir)/image/image.cpp",
        "$(lithium_src_dir)/image/image.hpp",
        "$(lithium_src_dir)/image/draw.cpp",
        "$(lithium_src_dir)/modules/io/compress.cpp",
        "$(lithium_src_dir)/modules/io/compress.hpp",
        "$(lithium_src_dir)/modules/io/file.cpp",
        "$(lithium_src_dir)/modules/io/file.hpp",
        "$(lithium_src_dir)/modules/io/glob.hpp",
        "$(lithium_src_dir)/modules/io/io.cpp",
        "$(lithium_src_dir)/modules/io/io.hpp",
        "$(lithium_src_dir)/launcher/crash.cpp",
        "$(lithium_src_dir)/launcher/crash.hpp",
        "$(lithium_src_dir)/logger/aptlogger.cpp",
        "$(lithium_src_dir)/logger/aptlogger.hpp",
        "$(lithium_src_dir)/logger/log_manager.cpp",
        "$(lithium_src_dir)/logger/log_manager.hpp",
        "$(lithium_src_dir)/modules/module/modloader.cpp",
        "$(lithium_src_dir)/modules/module/modloader.hpp",
        "$(lithium_src_dir)/network/downloader.cpp",
        "$(lithium_src_dir)/network/downloader.hpp",
        "$(lithium_src_dir)/network/httpclient.cpp",
        "$(lithium_src_dir)/network/httpclient.hpp",
        "$(lithium_src_dir)/network/socketclient.cpp",
        "$(lithium_src_dir)/network/socketclient.hpp",
        "$(lithium_src_dir)/network/time.cpp",
        "$(lithium_src_dir)/network/time.hpp",
        "$(lithium_src_dir)/liproperty/base64.cpp",
        "$(lithium_src_dir)/liproperty/base64.hpp",
        "$(lithium_src_dir)/liproperty/imessage.cpp",
        "$(lithium_src_dir)/liproperty/iproperty.hpp",
        "$(lithium_src_dir)/liproperty/sha256.hpp",
        "$(lithium_src_dir)/liproperty/uuid.cpp",
        "$(lithium_src_dir)/liproperty/uuid.hpp",
        "$(lithium_src_dir)/modules/server/commander.cpp",
        "$(lithium_src_dir)/modules/server/commander.hpp",
        "$(lithium_src_dir)/websocket/WebSocketServer.cpp",
        "$(lithium_src_dir)/modules/thread/thread.cpp",
        "$(lithium_src_dir)/modules/thread/thread.hpp",
        "$(lithium_src_dir)/modules/thread/threadpool.cpp",
        "$(lithium_src_dir)/modules/thread/threadpool.hpp",
        "$(lithium_src_dir)/modules/system/system.cpp",
        "$(lithium_src_dir)/modules/system/system.hpp",
        "$(lithium_src_dir)/App.cpp",
        "$(lithium_src_dir)/AppComponent.hpp",
        "$(lithium_src_dir)/LithiumApp.cpp",
        "$(lithium_src_dir)/LithiumApp.hpp"
    )

    -- link libraries
    if is_plat("windows") then
        add_links("pdh", "iphlpapi")
        add_cxxflags("/MD")
        add_defines("_CRT_SECURE_NO_WARNINGS")
        add_packages("openssl")
        add_packages("cfitsio")
        add_packages("zlib")
        add_packages("sqlite3")
        add_packages("pugixml")
        add_packages("loguru")
        add_packages("dlfcn-win32")
        add_packages("oatpp-websocket", "oatpp-swagger", "oatpp-openssl", "oatpp")
    elseif is_plat("macosx") then
        add_frameworks(
            "Cocoa",
            "IOKit",
            "Carbon",
            "Security",
            "Foundation"
        )
        add_packages("openssl")
        add_packages("cfitsio")
        add_packages("zlib")
        add_packages("sqlite3")
        add_packages("pugixml")
        add_packages("oatpp-websocket", "oatpp-swagger", "oatpp-openssl", "oatpp")
        add_packages("loguru")
    elseif is_plat("linux") then
        add_packages("openssl")
        add_packages("cfitsio")
        add_packages("zlib")
        add_packages("sqlite3")
        add_packages("pugixml")
        add_packages("oatpp-websocket", "oatpp-swagger", "oatpp-openssl", "oatpp")
        add_packages("loguru")
        add_packages("dl")
    end

    -- set output directory and name for Lithium executable
    if is_plat("windows") then
        set_targetdir("$(projectdir)/build/$(plat)/$(arch)")
        set_targetname("lithium_server")
    else
        set_targetdir("$(projectdir)/build")
        set_targetname("lithium_server")
    end
