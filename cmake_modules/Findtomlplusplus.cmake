# Findtomlplusplus.cmake

# 首先检查是否已经定义了变量 tomlplusplus_FOUND
if (tomlplusplus_FOUND)
    return()
endif()

# 定义 tomldir 用于指定 toml++ 的安装路径
set(tomldir "" CACHE PATH "Path to the tomlplusplus installation directory")

# 设置默认的库名和头文件路径
set(tomlplusplus_LIBRARIES "")
set(tomlplusplus_INCLUDE_DIRS "")

# 检查给定路径是否存在 toml++ 库文件
find_path(tomlplusplus_INCLUDE_DIRS "toml.hpp" HINTS ${tomldir}/include)

# 如果找到头文件路径，则设置 tomlplusplus_FOUND 为真
if (tomlplusplus_INCLUDE_DIRS)
    set(tomlplusplus_FOUND TRUE)
endif()

# 检查给定路径是否存在 toml++ 库文件
find_library(tomlplusplus_LIBRARIES NAMES tomlplusplus libtomlplusplus HINTS ${tomldir}/lib)

# 如果找到库文件，则设置 tomlplusplus_FOUND 为真
if (tomlplusplus_LIBRARIES)
    set(tomlplusplus_FOUND TRUE)
endif()

# 导出结果变量
if (tomlplusplus_FOUND)
    set(tomlplusplus_INCLUDE_DIRS ${tomlplusplus_INCLUDE_DIRS} CACHE PATH "Path to the tomlplusplus include directory.")
    set(tomlplusplus_LIBRARIES ${tomlplusplus_LIBRARIES} CACHE FILEPATH "Path to the tomlplusplus library.")
endif()
