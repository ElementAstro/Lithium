#pragma once

#define RESPONSE_ERROR(res, code, msg)                \
    do                                                \
    {                                                 \
        LOG_F(ERROR, "{}: {}", __func__, msg);        \
        res["error"] = magic_enum::enum_name(code);   \
        res["message"] = msg;                         \
        res["timestamp"] = GetChinaTimestampString(); \
    } while (0)

#define RESPONSE_EXCEPTION(res, code, msg)            \
    do                                                \
    {                                                 \
        LOG_F(ERROR, "{}: {}", __func__, msg);        \
        res["error"] = magic_enum::enum_name(code);   \
        res["message"] = msg;                         \
        res["timestamp"] = GetChinaTimestampString(); \
    } while (0)
