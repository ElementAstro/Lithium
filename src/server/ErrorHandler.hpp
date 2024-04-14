/*
 * ErrorHandle.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Error Handle (404 or 500)

**************************************************/

#ifndef ERRORHANDLER_HPP
#define ERRORHANDLER_HPP

#include "data/StatusDto.hpp"

#include "oatpp/web/protocol/http/outgoing/ResponseFactory.hpp"
#include "oatpp/web/server/handler/ErrorHandler.hpp"

class ErrorHandler : public oatpp::web::server::handler::ErrorHandler {
private:
    typedef oatpp::web::protocol::http::outgoing::Response OutgoingResponse;
    typedef oatpp::web::protocol::http::Status Status;
    typedef oatpp::web::protocol::http::outgoing::ResponseFactory
        ResponseFactory;

private:
    std::shared_ptr<oatpp::data::mapping::ObjectMapper> m_objectMapper;

public:
    ErrorHandler(const std::shared_ptr<oatpp::data::mapping::ObjectMapper>
                     &objectMapper);

    std::shared_ptr<OutgoingResponse> handleError(
        const Status &status, const oatpp::String &message,
        const Headers &headers) override;
};

#endif  // ERRORHANDLER_HPP