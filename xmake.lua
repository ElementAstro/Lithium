set_project("Lithium")
set_version("1.0.0")

-- Project options
option("ENABLE_ASYNC")
    set_default(true)
    set_showmenu(true)
option_end()

option("ENABLE_NATIVE_SERVER")
    set_default(false)
    set_showmenu(true)
option_end()

option("ENABLE_DEBUG")
    set_default(false)
    set_showmenu(true)
option_end()

option("ENABLE_FASHHASH")
    set_default(false)
    set_showmenu(true)
option_end()

option("ENABLE_WEB_SERVER")
    set_default(true)
    set_showmenu(true)
option_end()

option("ENABLE_WEB_CLIENT")
    set_default(true)
    set_showmenu(true)
option_end()

-- Set compile definitions based on options
add_defines("ENABLE_ASYNC_FLAG", "ENABLE_DEBUG_FLAG", "ENABLE_NATIVE_SERVER_FLAG", "ENABLE_FASHHASH_FLAG", "ENABLE_WEB_SERVER_FLAG", "ENABLE_WEB_CLIENT_FLAG")

-- Set policies
set_policy("check.auto_ignore_flags", false)
set_policy("check.auto_map_flags", false)

-- Set project directories
local lithium_src_dir = path.join(os.projectdir(), "src")
local lithium_module_dir = path.join(lithium_src_dir, "atom")
local lithium_client_dir = path.join(lithium_src_dir, "client")
local lithium_component_dir = path.join(lithium_src_dir, "addon")
local lithium_task_dir = path.join(lithium_src_dir, "task")

-- Include directories
add_includedirs(path.join(os.projectdir(), "libs"))
add_includedirs(path.join(os.projectdir(), "driverlibs"))
add_includedirs(lithium_src_dir)
add_includedirs(lithium_module_dir)
add_includedirs(path.join(os.projectdir(), "libs/oatpp/oatpp"))
add_includedirs(path.join(os.projectdir(), "libs/oatpp-swagger/oatpp-swagger"))
add_includedirs(path.join(os.projectdir(), "libs/oatpp-websocket/oatpp-websocket"))
add_includedirs(path.join(os.projectdir(), "libs/oatpp-openssl/oatpp-openssl"))

-- Find packages
add_requires("openssl", "zlib", "sqlite3", "fmt", "loguru", "cpp-httplib", "tinyxml2", "pocketpy")

-- Add subdirectories
includes("libs", "modules", path.join(lithium_module_dir), path.join(lithium_src_dir, "config"), path.join(lithium_src_dir, "task"), path.join(lithium_src_dir, "server"), path.join(lithium_src_dir, "utils"), path.join(lithium_src_dir, "addon"), path.join(lithium_src_dir, "client"))

-- Set source files
local component_module = {
    path.join(lithium_component_dir, "addons.cpp"),
    path.join(lithium_component_dir, "compiler.cpp"),
    path.join(lithium_component_dir, "dependency.cpp"),
    path.join(lithium_component_dir, "loader.cpp"),
    path.join(lithium_component_dir, "manager.cpp"),
    path.join(lithium_component_dir, "sandbox.cpp")
}

local config_module = {
    path.join(lithium_src_dir, "config/configor.cpp")
}

local debug_module = {
    path.join(lithium_src_dir, "debug/terminal.cpp"),
    path.join(lithium_src_dir, "debug/suggestion.cpp"),
    path.join(lithium_src_dir, "debug/command.cpp"),
    path.join(lithium_src_dir, "debug/console.cpp")
}

local script_module = {
    path.join(lithium_src_dir, "script/manager.cpp"),
    path.join(lithium_src_dir, "script/sheller.cpp")
}

local lithium_module = {
    path.join(lithium_src_dir, "LithiumApp.cpp"),
    path.join(lithium_src_dir, "utils/constant.cpp")
}

-- Build lithium_server-library
target("lithium_server-library")
    set_kind("static")
    add_files(table.unpack(component_module))
    add_files(table.unpack(config_module))
    add_files(table.unpack(debug_module))
    add_files(table.unpack(script_module))
    add_files(table.unpack(lithium_module))
    add_packages("loguru")
    add_packages("atom")
target_end()

-- Build lithium_server executable
target("lithium_server")
    set_kind("binary")
    add_files(path.join(lithium_src_dir, "App.cpp"))
    add_deps("lithium_server-library", "lithium-config", "lithium-task")
    add_packages("loguru", "fmt", "openssl", "zlib", "sqlite3", "cpp-httplib", "tinyxml2", "pocketpy")
    add_defines("LOGURU_DEBUG_LOGGING")
    if is_plat("windows") then
        add_packages("dlfcn-win32")
        add_syslinks("pdh", "iphlpapi", "winmm", "crypt32", "wsock32", "ws2_32")

    elseif is_plat("linux", "macosx") then
        add_syslinks("dl")
    else
        raise("Unsupported platform")
    end
    set_targetdir("$(buildir)/bin")
target_end()
