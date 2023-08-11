#include "ErrorHandler.hpp"

ErrorHandler::ErrorHandler(const std::shared_ptr<oatpp::data::mapping::ObjectMapper> &objectMapper)
    : m_objectMapper(objectMapper)
{
}

std::shared_ptr<ErrorHandler::OutgoingResponse>
ErrorHandler::handleError(const Status &status, const oatpp::String &message, const Headers &headers)
{
    auto error = StatusDto::createShared();
    error->status = "ERROR";
    error->code = status.code;
    error->message = message;

    if (status.code == 404)
    {
        auto response = ResponseFactory::createResponse(Status::CODE_404, R"(
            <!DOCTYPE html>
            <html lang="en">
            <head>
            <meta charset="UTF-8">
            <title>404 Not Found</title>
            <style>
                /* CSS样式 */
                body {
                background-color: #f1f1f1;
                font-family: Arial, sans-serif;
                text-align: center;
                padding: 150px;
                }
                
                h1 {
                font-size: 60px;
                color: #555;
                }
                
                p {
                font-size: 18px;
                color: #777;
                }
                
                a {
                color: #06c;
                text-decoration: none;
                }
                
                .container {
                max-width: 600px;
                margin: auto;
                background-color: #fff;
                border-radius: 5px;
                padding: 40px;
                box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
                }
                
                .image {
                margin-bottom: 30px;
                }
            </style>
            <script>
                // JavaScript代码
                document.addEventListener("DOMContentLoaded", function() {
                var backButton = document.getElementById("backButton");
                backButton.addEventListener("click", function(e) {
                    e.preventDefault();
                    history.back();
                });
                });
            </script>
            </head>
            <body>
            <div class="container">
                <h1>Oops! 404</h1>
                <p>抱歉，页面未找到。</p>
                <p><a href="#" id="backButton">返回上一页</a></p>
            </div>
            </body>
            </html>
        )");
        return response;
    }
    else
    {
        auto response = ResponseFactory::createResponse(status, error, m_objectMapper);

        for (const auto &pair : headers.getAll())
        {
            response->putHeader(pair.first.toString(), pair.second.toString());
        }
        return response;
    }
}