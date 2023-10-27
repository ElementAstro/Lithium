/*
 * error_code.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-8-10

Description: All of the error code

**************************************************/

#pragma once

enum class LIError
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
    LoadError,         // 动态库加载错误
    UnLoadError        // 动态卸载错误
};

enum class DeviceError
{
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

enum class ServerError
{
    None,
    InvalidParameters,
    InvalidFormat
};