#include <pybind11/pybind11.h>

#include "error_code.hpp"
#include "error_stack.hpp"
#include "exception.hpp"

namespace py = pybind11;

using namespace atom::error;
PYBIND11_EMBEDDED_MODULE(atom_error, m) {
    m.doc() = "ATOM error code module";

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
        .value("FormatError", FileError::FormatError);

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
        .value("ResourceExhausted", DeviceError::ResourceExhausted);

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
        .value("AuthenticationError", ServerError::AuthenticationError);

    py::class_<ErrorInfo>(m, "ErrorInfo")
        .def(py::init<>())
        .def_readwrite("errorMessage", &ErrorInfo::errorMessage)
        .def_readwrite("moduleName", &ErrorInfo::moduleName)
        .def_readwrite("functionName", &ErrorInfo::functionName)
        .def_readwrite("line", &ErrorInfo::line)
        .def_readwrite("fileName", &ErrorInfo::fileName)
        .def_readwrite("timestamp", &ErrorInfo::timestamp)
        .def_readwrite("uuid", &ErrorInfo::uuid);

    py::class_<ErrorStack>(m, "ErrorStack")
        .def(py::init<>())
        .def("createShared", &ErrorStack::createShared)
        .def("createUnique", &ErrorStack::createUnique)
        .def("insertError", &ErrorStack::insertError)
        .def("setFilteredModules", &ErrorStack::setFilteredModules)
        .def("clearFilteredModules", &ErrorStack::clearFilteredModules)
        .def("printFilteredErrorStack", &ErrorStack::printFilteredErrorStack)
        .def("getFilteredErrorsByModule",
             &ErrorStack::getFilteredErrorsByModule)
        .def("getCompressedErrors", &ErrorStack::getCompressedErrors);
}
