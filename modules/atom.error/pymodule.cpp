#include <pybind11/pybind11.h>

#include "atom/error/error_code.hpp"
#include "atom/error/exception.hpp"

namespace py = pybind11;

void bind_exceptions(py::module &m) {
    py::register_exception<atom::error::Exception>(m, "Exception");
    py::register_exception<atom::error::SystemErrorException>(
        m, "SystemErrorException");
    py::register_exception<atom::error::NestedException>(m, "NestedException");
    py::register_exception<atom::error::RuntimeError>(m, "RuntimeError");
    py::register_exception<atom::error::LogicError>(m, "LogicError");
    py::register_exception<atom::error::UnlawfulOperation>(m,
                                                           "UnlawfulOperation");
    py::register_exception<atom::error::OutOfRange>(m, "OutOfRange");
    py::register_exception<atom::error::OverflowException>(m,
                                                           "OverflowException");
    py::register_exception<atom::error::UnderflowException>(
        m, "UnderflowException");
    py::register_exception<atom::error::Unkown>(m, "Unkown");
    py::register_exception<atom::error::ObjectAlreadyExist>(
        m, "ObjectAlreadyExist");
    py::register_exception<atom::error::ObjectAlreadyInitialized>(
        m, "ObjectAlreadyInitialized");
    py::register_exception<atom::error::ObjectNotExist>(m, "ObjectNotExist");
    py::register_exception<atom::error::ObjectUninitialized>(
        m, "ObjectUninitialized");
    py::register_exception<atom::error::SystemCollapse>(m, "SystemCollapse");
    py::register_exception<atom::error::NullPointer>(m, "NullPointer");
    py::register_exception<atom::error::NotFound>(m, "NotFound");
    py::register_exception<atom::error::WrongArgument>(m, "WrongArgument");
    py::register_exception<atom::error::InvalidArgument>(m, "InvalidArgument");
    py::register_exception<atom::error::MissingArgument>(m, "MissingArgument");
    py::register_exception<atom::error::FileNotFound>(m, "FileNotFound");
    py::register_exception<atom::error::FileNotReadable>(m, "FileNotReadable");
    py::register_exception<atom::error::FileNotWritable>(m, "FileNotWritable");
    py::register_exception<atom::error::FailToOpenFile>(m, "FailToOpenFile");
    py::register_exception<atom::error::FailToCloseFile>(m, "FailToCloseFile");
    py::register_exception<atom::error::FailToCreateFile>(m,
                                                          "FailToCreateFile");
    py::register_exception<atom::error::FailToDeleteFile>(m,
                                                          "FailToDeleteFile");
    py::register_exception<atom::error::FailToCopyFile>(m, "FailToCopyFile");
    py::register_exception<atom::error::FailToMoveFile>(m, "FailToMoveFile");
    py::register_exception<atom::error::FailToReadFile>(m, "FailToReadFile");
    py::register_exception<atom::error::FailToWriteFile>(m, "FailToWriteFile");
    py::register_exception<atom::error::FailToLoadDll>(m, "FailToLoadDll");
    py::register_exception<atom::error::FailToUnloadDll>(m, "FailToUnloadDll");
    py::register_exception<atom::error::FailToLoadSymbol>(m,
                                                          "FailToLoadSymbol");
    py::register_exception<atom::error::FailToCreateProcess>(
        m, "FailToCreateProcess");
    py::register_exception<atom::error::FailToTerminateProcess>(
        m, "FailToTerminateProcess");
    py::register_exception<atom::error::JsonParseError>(m, "JsonParseError");
    py::register_exception<atom::error::JsonValueError>(m, "JsonValueError");
    py::register_exception<atom::error::CurlInitializationError>(
        m, "CurlInitializationError");
    py::register_exception<atom::error::CurlRuntimeError>(m,
                                                          "CurlRuntimeError");
}

PYBIND11_MODULE(error, m) {
    py::enum_<ErrorCodeBase>(m, "ErrorCodeBase")
        .value("Success", ErrorCodeBase::Success)
        .value("Failed", ErrorCodeBase::Failed)
        .value("Cancelled", ErrorCodeBase::Cancelled);

    py::enum_<FileError>(m, "FileError")
        .value("None", FileError::None)
        .value("NotFound", FileError::NotFound)
        .value("OpenError", FileError::OpenError)
        .value("AccessDenied", FileError::AccessDenied)
        .value("ReadError", FileError::ReadError)
        .value("WriteError", FileError::WriteError)
        .value("PermissionDenied", FileError::PermissionDenied)
        .value("ParseError", FileError::ParseError)
        .value("InvalidPath", FileError::InvalidPath)
        .value("FileExists", FileError::FileExists)
        .value("DirectoryNotEmpty", FileError::DirectoryNotEmpty)
        .value("TooManyOpenFiles", FileError::TooManyOpenFiles)
        .value("DiskFull", FileError::DiskFull)
        .value("LoadError", FileError::LoadError)
        .value("UnLoadError", FileError::UnLoadError)
        .value("LockError", FileError::LockError)
        .value("FormatError", FileError::FormatError)
        .value("PathTooLong", FileError::PathTooLong)
        .value("FileCorrupted", FileError::FileCorrupted)
        .value("UnsupportedFormat", FileError::UnsupportedFormat);

    py::enum_<DeviceError>(m, "DeviceError")
        .value("None", DeviceError::None)
        .value("NotSpecific", DeviceError::NotSpecific)
        .value("NotFound", DeviceError::NotFound)
        .value("NotSupported", DeviceError::NotSupported)
        .value("NotConnected", DeviceError::NotConnected)
        .value("MissingValue", DeviceError::MissingValue)
        .value("InvalidValue", DeviceError::InvalidValue)
        .value("Busy", DeviceError::Busy)
        .value("ExposureError", DeviceError::ExposureError)
        .value("GainError", DeviceError::GainError)
        .value("OffsetError", DeviceError::OffsetError)
        .value("ISOError", DeviceError::ISOError)
        .value("CoolingError", DeviceError::CoolingError)
        .value("GotoError", DeviceError::GotoError)
        .value("ParkError", DeviceError::ParkError)
        .value("UnParkError", DeviceError::UnParkError)
        .value("ParkedError", DeviceError::ParkedError)
        .value("HomeError", DeviceError::HomeError)
        .value("InitializationError", DeviceError::InitializationError)
        .value("ResourceExhausted", DeviceError::ResourceExhausted)
        .value("FirmwareUpdateFailed", DeviceError::FirmwareUpdateFailed)
        .value("CalibrationError", DeviceError::CalibrationError)
        .value("Overheating", DeviceError::Overheating)
        .value("PowerFailure", DeviceError::PowerFailure);

    py::enum_<NetworkError>(m, "NetworkError")
        .value("None", NetworkError::None)
        .value("ConnectionLost", NetworkError::ConnectionLost)
        .value("ConnectionRefused", NetworkError::ConnectionRefused)
        .value("DNSLookupFailed", NetworkError::DNSLookupFailed)
        .value("ProtocolError", NetworkError::ProtocolError)
        .value("SSLHandshakeFailed", NetworkError::SSLHandshakeFailed)
        .value("AddressInUse", NetworkError::AddressInUse)
        .value("AddressNotAvailable", NetworkError::AddressNotAvailable)
        .value("NetworkDown", NetworkError::NetworkDown)
        .value("HostUnreachable", NetworkError::HostUnreachable)
        .value("MessageTooLarge", NetworkError::MessageTooLarge)
        .value("BufferOverflow", NetworkError::BufferOverflow)
        .value("TimeoutError", NetworkError::TimeoutError)
        .value("BandwidthExceeded", NetworkError::BandwidthExceeded)
        .value("NetworkCongested", NetworkError::NetworkCongested);

    py::enum_<DatabaseError>(m, "DatabaseError")
        .value("None", DatabaseError::None)
        .value("ConnectionFailed", DatabaseError::ConnectionFailed)
        .value("QueryFailed", DatabaseError::QueryFailed)
        .value("TransactionFailed", DatabaseError::TransactionFailed)
        .value("IntegrityConstraintViolation",
               DatabaseError::IntegrityConstraintViolation)
        .value("NoSuchTable", DatabaseError::NoSuchTable)
        .value("DuplicateEntry", DatabaseError::DuplicateEntry)
        .value("DataTooLong", DatabaseError::DataTooLong)
        .value("DataTruncated", DatabaseError::DataTruncated)
        .value("Deadlock", DatabaseError::Deadlock)
        .value("LockTimeout", DatabaseError::LockTimeout)
        .value("IndexOutOfBounds", DatabaseError::IndexOutOfBounds)
        .value("ConnectionTimeout", DatabaseError::ConnectionTimeout)
        .value("InvalidQuery", DatabaseError::InvalidQuery);

    py::enum_<MemoryError>(m, "MemoryError")
        .value("None", MemoryError::None)
        .value("AllocationFailed", MemoryError::AllocationFailed)
        .value("OutOfMemory", MemoryError::OutOfMemory)
        .value("AccessViolation", MemoryError::AccessViolation)
        .value("BufferOverflow", MemoryError::BufferOverflow)
        .value("DoubleFree", MemoryError::DoubleFree)
        .value("InvalidPointer", MemoryError::InvalidPointer)
        .value("MemoryLeak", MemoryError::MemoryLeak)
        .value("StackOverflow", MemoryError::StackOverflow)
        .value("CorruptedHeap", MemoryError::CorruptedHeap);

    py::enum_<UserInputError>(m, "UserInputError")
        .value("None", UserInputError::None)
        .value("InvalidInput", UserInputError::InvalidInput)
        .value("OutOfRange", UserInputError::OutOfRange)
        .value("MissingInput", UserInputError::MissingInput)
        .value("FormatError", UserInputError::FormatError)
        .value("UnsupportedType", UserInputError::UnsupportedType)
        .value("InputTooLong", UserInputError::InputTooLong)
        .value("InputTooShort", UserInputError::InputTooShort)
        .value("InvalidCharacter", UserInputError::InvalidCharacter);

    py::enum_<ConfigError>(m, "ConfigError")
        .value("None", ConfigError::None)
        .value("MissingConfig", ConfigError::MissingConfig)
        .value("InvalidConfig", ConfigError::InvalidConfig)
        .value("ConfigParseError", ConfigError::ConfigParseError)
        .value("UnsupportedConfig", ConfigError::UnsupportedConfig)
        .value("ConfigConflict", ConfigError::ConfigConflict)
        .value("InvalidOption", ConfigError::InvalidOption)
        .value("ConfigNotSaved", ConfigError::ConfigNotSaved)
        .value("ConfigLocked", ConfigError::ConfigLocked);

    py::enum_<ProcessError>(m, "ProcessError")
        .value("None", ProcessError::None)
        .value("ProcessNotFound", ProcessError::ProcessNotFound)
        .value("ProcessFailed", ProcessError::ProcessFailed)
        .value("ThreadCreationFailed", ProcessError::ThreadCreationFailed)
        .value("ThreadJoinFailed", ProcessError::ThreadJoinFailed)
        .value("ThreadTimeout", ProcessError::ThreadTimeout)
        .value("DeadlockDetected", ProcessError::DeadlockDetected)
        .value("ProcessTerminated", ProcessError::ProcessTerminated)
        .value("InvalidProcessState", ProcessError::InvalidProcessState)
        .value("InsufficientResources", ProcessError::InsufficientResources)
        .value("InvalidThreadPriority", ProcessError::InvalidThreadPriority);

    py::enum_<ServerError>(m, "ServerError")
        .value("None", ServerError::None)
        .value("InvalidParameters", ServerError::InvalidParameters)
        .value("InvalidFormat", ServerError::InvalidFormat)
        .value("MissingParameters", ServerError::MissingParameters)
        .value("RunFailed", ServerError::RunFailed)
        .value("UnknownError", ServerError::UnknownError)
        .value("UnknownCommand", ServerError::UnknownCommand)
        .value("UnknownDevice", ServerError::UnknownDevice)
        .value("UnknownDeviceType", ServerError::UnknownDeviceType)
        .value("UnknownDeviceName", ServerError::UnknownDeviceName)
        .value("UnknownDeviceID", ServerError::UnknownDeviceID)
        .value("NetworkError", ServerError::NetworkError)
        .value("TimeoutError", ServerError::TimeoutError)
        .value("AuthenticationError", ServerError::AuthenticationError)
        .value("PermissionDenied", ServerError::PermissionDenied)
        .value("ServerOverload", ServerError::ServerOverload)
        .value("MaintenanceMode", ServerError::MaintenanceMode);

    bind_exceptions(m);

    py::class_<atom::error::StackTrace>(m, "StackTrace")
        .def(py::init<>())
        .def("toString", &atom::error::StackTrace::toString);
}