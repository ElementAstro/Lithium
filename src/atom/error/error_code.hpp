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
    PathTooLong = 116,        // 路径过长
    FileCorrupted = 117,      // 文件损坏
    UnsupportedFormat = 118,  // 不支持的文件格式
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

    InitializationError = 230,   // 初始化错误
    ResourceExhausted = 231,     // 资源耗尽
    FirmwareUpdateFailed = 232,  // 固件更新失败
    CalibrationError = 233,      // 校准错误
    Overheating = 234,           // 设备过热
    PowerFailure = 235,          // 电源故障
};

// 网络错误
enum class NetworkError : int {
    None = static_cast<int>(ErrorCodeBase::Success),
    ConnectionLost = 400,       // 网络连接丢失
    ConnectionRefused = 401,    // 连接被拒绝
    DNSLookupFailed = 402,      // DNS查询失败
    ProtocolError = 403,        // 协议错误
    SSLHandshakeFailed = 404,   // SSL握手失败
    AddressInUse = 405,         // 地址已在使用
    AddressNotAvailable = 406,  // 地址不可用
    NetworkDown = 407,          // 网络已关闭
    HostUnreachable = 408,      // 主机不可达
    MessageTooLarge = 409,      // 消息过大
    BufferOverflow = 410,       // 缓冲区溢出
    TimeoutError = 411,         // 网络超时
    BandwidthExceeded = 412,    // 带宽超限
    NetworkCongested = 413,     // 网络拥塞
};

// 数据库错误
enum class DatabaseError : int {
    None = static_cast<int>(ErrorCodeBase::Success),
    ConnectionFailed = 500,              // 数据库连接失败
    QueryFailed = 501,                   // 查询失败
    TransactionFailed = 502,             // 事务失败
    IntegrityConstraintViolation = 503,  // 违反完整性约束
    NoSuchTable = 504,                   // 表不存在
    DuplicateEntry = 505,                // 重复条目
    DataTooLong = 506,                   // 数据过长
    DataTruncated = 507,                 // 数据被截断
    Deadlock = 508,                      // 死锁
    LockTimeout = 509,                   // 锁超时
    IndexOutOfBounds = 510,              // 索引越界
    ConnectionTimeout = 511,             // 连接超时
    InvalidQuery = 512,                  // 无效查询
};

// 内存管理错误
enum class MemoryError : int {
    None = static_cast<int>(ErrorCodeBase::Success),
    AllocationFailed = 600,  // 内存分配失败
    OutOfMemory = 601,       // 内存不足
    AccessViolation = 602,   // 内存访问违例
    BufferOverflow = 603,    // 缓冲区溢出
    DoubleFree = 604,        // 双重释放
    InvalidPointer = 605,    // 无效指针
    MemoryLeak = 606,        // 内存泄漏
    StackOverflow = 607,     // 栈溢出
    CorruptedHeap = 608,     // 堆损坏
};

// 用户输入错误
enum class UserInputError : int {
    None = static_cast<int>(ErrorCodeBase::Success),
    InvalidInput = 700,      // 无效输入
    OutOfRange = 701,        // 输入值超出范围
    MissingInput = 702,      // 缺少输入
    FormatError = 703,       // 输入格式错误
    UnsupportedType = 704,   // 不支持的输入类型
    InputTooLong = 705,      // 输入过长
    InputTooShort = 706,     // 输入过短
    InvalidCharacter = 707,  // 无效字符
};

// 配置错误
enum class ConfigError : int {
    None = static_cast<int>(ErrorCodeBase::Success),
    MissingConfig = 800,      // 缺少配置文件
    InvalidConfig = 801,      // 无效的配置
    ConfigParseError = 802,   // 配置解析错误
    UnsupportedConfig = 803,  // 不支持的配置
    ConfigConflict = 804,     // 配置冲突
    InvalidOption = 805,      // 无效选项
    ConfigNotSaved = 806,     // 配置未保存
    ConfigLocked = 807,       // 配置被锁定
};

// 进程和线程错误
enum class ProcessError : int {
    None = static_cast<int>(ErrorCodeBase::Success),
    ProcessNotFound = 900,        // 进程未找到
    ProcessFailed = 901,          // 进程失败
    ThreadCreationFailed = 902,   // 线程创建失败
    ThreadJoinFailed = 903,       // 线程合并失败
    ThreadTimeout = 904,          // 线程超时
    DeadlockDetected = 905,       // 检测到死锁
    ProcessTerminated = 906,      // 进程被终止
    InvalidProcessState = 907,    // 无效的进程状态
    InsufficientResources = 908,  // 资源不足
    InvalidThreadPriority = 909,  // 无效的线程优先级
};

// 服务器错误
enum class ServerError : int {
    None = static_cast<int>(ErrorCodeBase::Success),
    InvalidParameters = 300,    // 无效参数
    InvalidFormat = 301,        // 无效格式
    MissingParameters = 302,    // 缺少参数
    RunFailed = 303,            // 运行失败
    UnknownError = 310,         // 未知错误
    UnknownCommand = 311,       // 未知命令
    UnknownDevice = 312,        // 未知设备
    UnknownDeviceType = 313,    // 未知设备类型
    UnknownDeviceName = 314,    // 未知设备名称
    UnknownDeviceID = 315,      // 未知设备ID
    NetworkError = 320,         // 网络错误
    TimeoutError = 321,         // 请求超时
    AuthenticationError = 322,  // 认证失败
    PermissionDenied = 323,     // 权限被拒绝
    ServerOverload = 324,       // 服务器过载
    MaintenanceMode = 325,      // 维护模式
};

#endif