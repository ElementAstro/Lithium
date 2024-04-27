-- 设置项目信息
set_project("atom-connection")
set_version("1.0.0")
set_description("Connection Between Lithium Drivers, TCP and IPC")
set_license("GPL3")

-- 添加构建模式
add_rules("mode.debug", "mode.release")

-- 设置构建选项
option("enable_ssh")
    set_default(false)
    set_showmenu(true)
    set_description("Enable SSH support")
option_end()

option("enable_libssh")
    set_default(false)
    set_showmenu(true)
    set_description("Enable LibSSH support")
option_end()

option("enable_python")
    set_default(false)
    set_showmenu(true)
    set_description("Enable Python bindings")
option_end()

-- 设置构建目标
target("atom-connection")
    set_kind("static")
    add_files("*.cpp")
    add_headerfiles("*.hpp")
    add_packages("loguru")
    if is_plat("windows") then
        add_syslinks("ws2_32")
    end
    if has_config("enable_ssh") then
        add_packages("libssh")
    end
    if has_config("enable_libssh") then
        add_files("sshclient.cpp")
        add_headerfiles("sshclient.hpp")
    end
    if has_config("enable_python") then
        add_rules("python.pybind11_module")
        add_files("_pybind.cpp")
        add_deps("python")
    end

-- 安装目标文件
target("install")
    set_kind("phony")
    add_deps("atom-connection")
    on_install(function (target)
        import("package.tools.install")
        local installx = package.tools.install
        installx.static("atom-connection", {destdir = "/usr/local/lib"})
    end)

-- 构建项目
target("build")
    set_kind("phony")
    add_deps("atom-connection")

-- 清理构建产物
target("clean")
    set_kind("phony")
    add_rules("utils.clean.clean")