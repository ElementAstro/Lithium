#pragma once

#define RESPONSE_ERROR(res, code, msg)                             \
    do                                                             \
    {                                                              \
        LOG_F(ERROR, "{}: {}", __func__, msg);                     \
        res["error"] = magic_enum::enum_name(code);                \
        res["message"] = msg;                                      \
        res["timestamp"] = Atom::Utils::GetChinaTimestampString(); \
        sendMessage(res.dump());                                   \
        return;                                                    \
    } while (0)

#define RESPONSE_ERROR_C(res, code, msg)                           \
    do                                                             \
    {                                                              \
        LOG_F(ERROR, "{}: {}", __func__, msg);                     \
        res["error"] = magic_enum::enum_name(code);                \
        res["message"] = msg;                                      \
        res["timestamp"] = Atom::Utils::GetChinaTimestampString(); \
        sendMessage(res.dump());                                   \
        return nullptr;                                            \
    } while (0)

#define RESPONSE_EXCEPTION(res, code, msg)            \
    do                                                \
    {                                                 \
        LOG_F(ERROR, "{}: {}", __func__, msg);        \
        res["error"] = magic_enum::enum_name(code);   \
        res["message"] = msg;                         \
        res["timestamp"] = GetChinaTimestampString(); \
        sendMessage(res.dump());                      \
        return;                                       \
    } while (0)

#define RESPONSE_EXCEPTION_C(res, code, msg)                       \
    do                                                             \
    {                                                              \
        LOG_F(ERROR, "{}: {}", __func__, msg);                     \
        res["error"] = magic_enum::enum_name(code);                \
        res["message"] = msg;                                      \
        res["timestamp"] = Atom::Utils::GetChinaTimestampString(); \
        sendMessage(res.dump());                                   \
        return nullptr;                                            \
    } while (0)
