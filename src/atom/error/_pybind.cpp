#include <pybind11/pybind11.h>

#include "error_code.hpp"
#include "error_stack.hpp"
#include "exception.hpp"

namespace py = pybind11;

using namespace atom::error;

PYBIND11_EMBEDDED_MODULE(atom_error, m) {
    py::enum_<LIError>(m, "LIError")
        .value("None", LIError::None)
        .value("NotFound", LIError::NotFound)
        .value("OpenError", LIError::OpenError)
        .value("AccessDenied", LIError::AccessDenied)
        .value("ReadError", LIError::ReadError)
        .value("WriteError", LIError::WriteError)
        .value("PermissionDenied", LIError::PermissionDenied)
        .value("ParseError", LIError::ParseError)
        .value("InvalidPath", LIError::InvalidPath)
        .value("FileExists", LIError::FileExists)
        .value("DirectoryNotEmpty", LIError::DirectoryNotEmpty)
        .value("TooManyOpenFiles", LIError::TooManyOpenFiles)
        .value("DiskFull", LIError::DiskFull)
        .value("LoadError", LIError::LoadError)
        .value("UnLoadError", LIError::UnLoadError);

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
        .value("HomeError", DeviceError::HomeError);

    py::enum_<DeviceWarning>(m, "DeviceWarning")
        .value("ExposureWarning", DeviceWarning::ExposureWarning)
        .value("GainWarning", DeviceWarning::GainWarning)
        .value("OffsetWarning", DeviceWarning::OffsetWarning)
        .value("ISOWarning", DeviceWarning::ISOWarning)
        .value("CoolingWarning", DeviceWarning::CoolingWarning)
        .value("GotoWarning", DeviceWarning::GotoWarning)
        .value("ParkWarning", DeviceWarning::ParkWarning)
        .value("UnParkWarning", DeviceWarning::UnParkWarning)
        .value("ParkedWarning", DeviceWarning::ParkedWarning)
        .value("HomeWarning", DeviceWarning::HomeWarning);

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
        .value("UnknownDeviceID", ServerError::UnknownDeviceID);

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
