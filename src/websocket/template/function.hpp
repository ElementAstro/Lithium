#pragma once

#define FUNCTION_BEGIN                  \
    json res = {{"command", __func__}}; \
    try                                 \
    {

#define FUNCTION_END                                                         \
    }                                                                        \
    catch (const json::exception &e)                                         \
    {                                                                        \
        RESPONSE_EXCEPTION(res, ServerError::InvalidParameters, e.what());   \
    }                                                                        \
    catch (const std::exception &e)                                          \
    {                                                                        \
        RESPONSE_EXCEPTION(res, ServerError::UnknownError, e.what());        \
    }                                                                        \
    catch (...)                                                              \
    {                                                                        \
        RESPONSE_EXCEPTION(res, ServerError::UnknownError, "Unknown Error"); \
    }\
