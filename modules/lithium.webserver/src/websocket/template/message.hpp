#pragma once

#define CHECK_STR_PARAM(_params)                                                                                          \
    if (!params->get<std::string>("params", #_params).has_value())                                                        \
    {                                                                                                                     \
        LOG_F(ERROR, "{}::{}: {} is needed!", getName(), __func__, #_params);                                             \
        return MessageHelper::MakeTextMessage(__func__, #_params + "lib_name is needed!", "client", "websocket-checker"); \
    }                                                                                                                     \
    \

#define CHECK_INT_PARAM(_params)                                                                                          \
    if (!params->get<int>("params", #_params).has_value())                                                              \
    {                                                                                                                     \
        LOG_F(ERROR, "{}::{}: {} is needed!", getName(), __func__, #_params);                                             \
        return MessageHelper::MakeTextMessage(__func__, #_params + "lib_name is needed!", "client", "websocket-checker"); \
    }                                                                                                                     \
    \

#define CHECK_BOOL_PARAM(_params)                                                                                         \
    if (!params->get<bool>("params", #_params).has_value())                                                             \
    {                                                                                                                     \
        LOG_F(ERROR, "{}::{}: {} is needed!", getName(), __func__, #_params);                                             \
        return MessageHelper::MakeTextMessage(__func__, #_params + "lib_name is needed!", "client", "websocket-checker"); \
    }                                                                                                                     \
    \
