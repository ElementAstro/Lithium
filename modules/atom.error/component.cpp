#include "atom/components/component.hpp"
#include "atom/components/registry.hpp"

#include "atom/error/error_code.hpp"

#include "atom/log/loguru.hpp"

using namespace atom::error;

ATOM_MODULE(atom_io, [](Component &component) {
    DLOG_F(INFO, "Loading module {}", component.getName());

    component.defEnum<ErrorCodeBase>(
        "ErrorCodeBase", {
                             {"Success", ErrorCodeBase::Success},
                             {"Failed", ErrorCodeBase::Failed},
                             {"Cancelled", ErrorCodeBase::Cancelled},
                         });

    component.defEnum<DeviceError>(
        "DeviceError",
        {
            {"None", DeviceError::None},
            {"NotConnected", DeviceError::NotConnected},
            {"NotFound", DeviceError::NotFound},
            {"NotSpecific", DeviceError::NotSpecific},
            {"NotSupported", DeviceError::NotSupported},
            {"InvalidValue", DeviceError::InvalidValue},
            {"MissingValue", DeviceError::MissingValue},
            {"NotConnected", DeviceError::NotConnected},
            {"NotSupported", DeviceError::NotSupported},
            {"InitializationError", DeviceError::InitializationError},
            {"ResourceExhausted", DeviceError::ResourceExhausted},
            {"GotoError", DeviceError::GotoError},
            {"HomeError", DeviceError::HomeError},
            {"ParkError", DeviceError::ParkError},
            {"UnParkError", DeviceError::UnParkError},
            {"ParkedError", DeviceError::ParkedError},
            {"ExposureError", DeviceError::ExposureError},
            {"GainError", DeviceError::GainError},
            {"ISOError", DeviceError::ISOError},
            {"OffsetError", DeviceError::OffsetError},
            {"CoolingError", DeviceError::CoolingError},
            {"Busy", DeviceError::Busy},
        });
    DLOG_F(INFO, "Loaded module {}", component.getName());
});
