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

enum class LIError {
    None,              // 无错误
    NotFound,          // 文件未找到
    OpenError,         // 无法打开
    AccessDenied,      // 访问被拒绝
    ReadError,         // 读取错误
    WriteError,        // 写入错误
    PermissionDenied,  // 权限被拒绝
    ParseError,   // 解析错误，通常由json.hpp抛出异常然后捕获
    InvalidPath,  // 无效路径
    FileExists,   // 文件已存在
    DirectoryNotEmpty,  // 目录非空
    TooManyOpenFiles,   // 打开的文件过多
    DiskFull,           // 磁盘已满
    LoadError,          // 动态库加载错误
    UnLoadError         // 动态卸载错误
};

enum class DeviceError {
    None,
    NotSpecific,
    NotFound,
    NotSupported,
    NotConnected,
    MissingValue,
    InvalidValue,
    Busy,

    // For Camera
    ExposureError,
    GainError,
    OffsetError,
    ISOError,
    CoolingError,

    // For Telescope
    GotoError,
    ParkError,
    UnParkError,
    ParkedError,
    HomeError
};

enum class DeviceWarning {
    // For Camera
    ExposureWarning,
    GainWarning,
    OffsetWarning,
    ISOWarning,
    CoolingWarning,

    // For Telescope
    GotoWarning,
    ParkWarning,
    UnParkWarning,
    ParkedWarning,
    HomeWarning
};

enum class ServerError {
    None,
    InvalidParameters,
    InvalidFormat,
    MissingParameters,

    RunFailed,

    UnknownError,
    UnknownCommand,
    UnknownDevice,
    UnknownDeviceType,
    UnknownDeviceName,
    UnknownDeviceID
};

#endif
