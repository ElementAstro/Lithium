#pragma once

#define CHECK_VARIABLE(name, message)                                            \
    OATPP_ASSERT_HTTP(body->name.getValue("") != "", Status::CODE_400, message); \
    auto name = body->name.getValue("");