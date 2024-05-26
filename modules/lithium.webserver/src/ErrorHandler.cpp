/*
 * ErrorHandle.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Error Handle (404 or 500)

**************************************************/

#include "ErrorHandler.hpp"

ErrorHandler::ErrorHandler(
    const std::shared_ptr<oatpp::data::mapping::ObjectMapper> &objectMapper)
    : m_objectMapper(objectMapper) {}

std::shared_ptr<ErrorHandler::OutgoingResponse> ErrorHandler::handleError(
    const Status &status, const oatpp::String &message,
    const Headers &headers) {
    auto error = StatusDto::createShared();
    error->status = "ERROR";
    error->code = status.code;
    error->message = message;
    error->command = "";

    auto response =
        ResponseFactory::createResponse(status, error, m_objectMapper);

    for (const auto &pair : headers.getAll()) {
        response->putHeader(pair.first.toString(), pair.second.toString());
    }
    return response;
}
