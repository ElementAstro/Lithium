#define GET_PARAM_OR_THROW(params, key, var)                      \
    if (!params.contains(key)) {                                  \
        THROW_MISSING_ARGUMENT(std::string(key) + " is missing"); \
    }                                                             \
    var = params[key];                                            \
    LOG_F(INFO, "{}: {}", key, var);