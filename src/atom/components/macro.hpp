#define CHECK_PARAM(key)                                                                \
    if (!params.contains(key))                                                          \
    {                                                                                   \
        LOG_F(ERROR, "Failed to execute {}: Invalid Parameters", __func__);             \
        return {                                                                        \
            {"command", __func__},                                                      \
            {"error", "Invalid Parameters"},                                            \
            {"status", "error"},                                                        \
            {"code", 1000},                                                             \
            {"message", std::format("Invalid Parameters, {} need {}", __func__, key)}}; \
    }\

#define CHECK_PARAMS(...)                                                                       \
    {                                                                                           \
        std::vector<std::string> __params = {__VA_ARGS__};                                      \
        for (const auto &key : __params)                                                        \
        {                                                                                       \
            if (!params.contains(key))                                                          \
            {                                                                                   \
                LOG_F(ERROR, "Failed to execute {}: Invalid Parameters", __func__);             \
                return {                                                                        \
                    {"command", __func__},                                                      \
                    {"error", "Invalid Parameters"},                                            \
                    {"status", "error"},                                                        \
                    {"code", 1000},                                                             \
                    {"message", std::format("Invalid Parameters, {} need {}", __func__, key)}}; \
            }                                                                                   \
        }                                                                                       \
    }                                                                                           \
    