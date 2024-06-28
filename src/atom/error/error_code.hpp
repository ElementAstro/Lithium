/*
 * error_code.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-8-10

Description: All of the error code

**************************************************/

#ifndef ATOM_ERROR_CODE_HPP
#define ATOM_ERROR_CODE_HPP

// 基础错误码（可选）
enum class ErrorCodeBase {
    Success = 0,    // 成功
    Failed = 1,     // 失败
    Cancelled = 2,  // 操作被取消
};

// 文件操作错误
enum class FileError : int {
    None = static_cast<int>(ErrorCodeBase::Success),
    NotFound = 100,           // 文件未找到
    OpenError = 101,          // 无法打开
    AccessDenied = 102,       // 访问被拒绝
    ReadError = 103,          // 读取错误
    WriteError = 104,         // 写入错误
    PermissionDenied = 105,   // 权限被拒绝
    ParseError = 106,         // 解析错误
    InvalidPath = 107,        // 无效路径
    FileExists = 108,         // 文件已存在
    DirectoryNotEmpty = 109,  // 目录非空
    TooManyOpenFiles = 110,   // 打开的文件过多
    DiskFull = 111,           // 磁盘已满
    LoadError = 112,          // 动态库加载错误
    UnLoadError = 113,        // 动态卸载错误
    LockError = 114,          // 文件锁错误
    FormatError = 115,        // 文件格式错误
};

// 设备错误
enum class DeviceError : int {
    None = static_cast<int>(ErrorCodeBase::Success),
    NotSpecific = 200,
    NotFound = 201,      // 设备未找到
    NotSupported = 202,  // 不支持的设备
    NotConnected = 203,  // 设备未连接
    MissingValue = 204,  // 缺少必要的值
    InvalidValue = 205,  // 无效的值
    Busy = 206,          // 设备忙

    // 相机特有错误
    ExposureError = 210,
    GainError = 211,
    OffsetError = 212,
    ISOError = 213,
    CoolingError = 214,

    // 望远镜特有错误
    GotoError = 220,
    ParkError = 221,
    UnParkError = 222,
    ParkedError = 223,
    HomeError = 224,

    InitializationError = 230,  // 初始化错误
    ResourceExhausted = 231,    // 资源耗尽
};

// 设备警告
// （保持现有结构，根据需要添加更多警告类型）

// 服务器错误
enum class ServerError : int {
    None = static_cast<int>(ErrorCodeBase::Success),
    InvalidParameters = 300,
    InvalidFormat = 301,
    MissingParameters = 302,

    RunFailed = 303,

    UnknownError = 310,
    UnknownCommand = 311,
    UnknownDevice = 312,
    UnknownDeviceType = 313,
    UnknownDeviceName = 314,
    UnknownDeviceID = 315,

    NetworkError = 320,         // 网络错误
    TimeoutError = 321,         // 请求超时
    AuthenticationError = 322,  // 认证失败
};

#endif  // ATOM_ERROR_CODE_HPP