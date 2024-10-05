#include <pybind11/pybind11.h>
#include <pybind11/enum.h>

#include "atom/error/error_code.hpp"

namespace py = pybind11;
using namespace atom::error;

PYBIND11_MODULE(atom_io, m) {
    py::enum_<ErrorCodeBase>(m, "ErrorCodeBase")
        .value("Success", ErrorCodeBase::Success)
        .value("Failed", ErrorCodeBase::Failed)
        .value("Cancelled", ErrorCodeBase::Cancelled)
        .export_values();

    py::enum_<DeviceError>(m, "DeviceError")
        .value("None", DeviceError::None)
        .value("NotConnected", DeviceError::NotConnected)
        .value("NotFound", DeviceError::NotFound)
        .value("NotSpecific", DeviceError::NotSpecific)
        .value("NotSupported", DeviceError::NotSupported)
        .value("InvalidValue", DeviceError::InvalidValue)
        .value("MissingValue", DeviceError::MissingValue)
        .value("InitializationError", DeviceError::InitializationError)
        .value("ResourceExhausted", DeviceError::ResourceExhausted)
        .value("GotoError", DeviceError::GotoError)
        .value("HomeError", DeviceError::HomeError)
        .value("ParkError", DeviceError::ParkError)
        .value("UnParkError", DeviceError::UnParkError)
        .value("ParkedError", DeviceError::ParkedError)
        .value("ExposureError", DeviceError::ExposureError)
        .value("GainError", DeviceError::GainError)
        .value("ISOError", DeviceError::ISOError)
        .value("OffsetError", DeviceError::OffsetError)
        .value("CoolingError", DeviceError::CoolingError)
        .value("Busy", DeviceError::Busy)
        .export_values();
}