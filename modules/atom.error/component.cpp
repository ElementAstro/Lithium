#include "atom/components/component.hpp"
#include "atom/components/registry.hpp"

#include "atom/error/error_code.hpp"
#include "atom/error/error_stack.hpp"

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

    component.def("insert_error", &ErrorStack::insertError, "error",
                  "Insert an error into the error stack.");
    component.def("set_filters", &ErrorStack::setFilteredModules, "error",
                  "Set filters.");
    component.def("clear_filters", &ErrorStack::clearFilteredModules, "error",
                  " Clear filters.");
    component.def("get_filtered_errors", &ErrorStack::getFilteredErrorsByModule,
                  "error", "Get filtered errors by module.");
    component.def("print_filtered_error_stack",
                  &ErrorStack::printFilteredErrorStack, "error",
                  "Print filtered error stack.");
    component.def("get_compressed_errors", &ErrorStack::getCompressedErrors,
                  "error", "Get compressed errors.");

    DLOG_F(INFO, "Loaded module {}", component.getName());
});