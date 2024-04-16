#ifndef ATOM_DRIVER_MARCO_HPP
#define ATOM_DRIVER_MARCO_HPP

#define CHECK_PARAM(x)                                                    \
    if (!params.contains(x)) {                                            \
        LOG_F(ERROR, "{} {}: Missing {} value", getName(), __func__, x);  \
        return createErrorResponse(                                       \
            __func__,                                                     \
            {{"error", magic_enum::enum_name(DeviceError::MissingValue)}, \
             {"value", x}},                                               \
            "Missing value");                                             \
    }

#define GET_STR_VARIABLE(x)                                               \
    auto _ #x = getVariable<std::string>(#x);                             \
    if (!_ #x.has_value()) {                                              \
        LOG_F(ERROR, "{} {}: Missing {} value", getName(), __func__, #x); \
        return false;                                                     \
    }                                                                     \
    auto x = _ #x.value();

#define GET_INT_VARIABLE(x)                                               \
    auto _ #x = getVariable<int>(#x);                                     \
    if (!_ #x.has_value()) {                                              \
        LOG_F(ERROR, "{} {}: Missing {} value", getName(), __func__, #x); \
        return false;                                                     \
    }                                                                     \
    auto x = _ #x.value();

#endif