#pragma once

enum class IOError
{
    None,              // 无错误
    NotFound,          // 文件未找到
    OepnError,         // 无法打开
    AccessDenied,      // 访问被拒绝
    ReadError,         // 读取错误
    WriteError,        // 写入错误
    PermissionDenied,  // 权限被拒绝
    ParseError,        // 解析错误，通常由json.hpp抛出异常然后捕获
    InvalidPath,       // 无效路径
    FileExists,        // 文件已存在
    DirectoryNotEmpty, // 目录非空
    TooManyOpenFiles,  // 打开的文件过多
    DiskFull,          // 磁盘已满
    LoadError,          // 动态库加载错误
    UnLoadError         // 动态卸载错误
};
