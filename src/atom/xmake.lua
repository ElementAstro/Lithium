set_project("atom")
set_version("1.0.0")
set_xmakever("2.5.1")

-- Python Support
option("build_python")
    set_default(false)
    set_showmenu(true)
option_end()

-- Subdirectories
includes("algorithm", "async", "components", "connection", "error", "function", "io", "log", "search", "sysinfo", "system", "type", "utils", "web")

-- Define libraries
local atom_packages = {
    "loguru",
    "cpp-httplib",
    "libzippp"
}

local atom_libs = {
    "atom-function",
    "atom-algorithm",
    "atom-async",
    "atom-task",
    "atom-io",
    "atom-component",
    "atom-type",
    "atom-utils",
    "atom-search",
    "atom-web",
    "atom-system"
}

-- Object Library
target("atom_object")
    set_kind("object")
    add_files("log/logger.cpp", "log/syslog.cpp")
    add_headerfiles("log/logger.hpp", "log/syslog.hpp")
    add_deps(table.unpack(atom_libs))
    add_packages(table.unpack(atom_packages))
    if is_plat("windows") then
        add_syslinks("setupapi", "wsock32", "ws2_32", "shlwapi", "iphlpapi")
    end
target_end()

-- Static Library
target("atom")
    set_kind("static")
    add_deps("atom_object")
    add_packages(table.unpack(atom_libs))
    set_version("1.0.0", {build = "%Y%m%d%H%M"})
    on_install(function (target)
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end)
target_end()

-- Python Support
if has_config("build_python") then
    add_requires("python", "pybind11")
    target("atom_python")
        set_kind("shared")
        add_files("python_binding.cpp")
        add_packages("python", "pybind11")
    target_end()
end
