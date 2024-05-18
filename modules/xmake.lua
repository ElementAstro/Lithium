-- xmake.lua for Lithium Builtin Modules
-- This project is licensed under the terms of the GPL3 license.
--
-- Project Name: Lithium Builtin Modules
-- Description:  A collection of useful system functions
-- Author: Max Qian
-- License: GPL3

-- 项目信息
set_project("lithium_builtin")
set_version("1.0.0")

-- 编译配置
set_languages("c99", "cxx20")

-- 递归添加子目录
function add_subdirectories_recursively(start_dir)
    local dirs = os.dirs(start_dir .. "/*")
    for _, dir in ipairs(dirs) do
        local cmakelists_path = path.join(dir, "CMakeLists.txt")
        local package_json_path = path.join(dir, "package.json")
        if os.isfile(cmakelists_path) and os.isfile(package_json_path) then
            print("Adding module subdirectory: " .. dir)
            includes(dir)
        end
    end
end

-- 添加子目录
add_subdirectories_recursively(os.scriptdir())

-- 示例模块配置
target("example_module")
    set_kind("shared")
    add_files("example_module/*.c")
