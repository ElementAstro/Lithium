/*
 * AsyncStaticController.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Static Route

**************************************************/

#ifndef LITHIUM_ASYNC_STATIC_CONTROLLER_HPP
#define LITHIUM_ASYNC_STATIC_CONTROLLER_HPP

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>

class StaticController : public oatpp::web::server::api::ApiController {
public:
    StaticController(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper) {}

    // ----------------------------------------------------------------
    // Pointer creator
    // ----------------------------------------------------------------

    static std::shared_ptr<StaticController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                        objectMapper)  // Inject objectMapper component here as
                                       // default parameter
    ) {
        return std::make_shared<StaticController>(objectMapper);
    }

    // ----------------------------------------------------------------
    // Static Loader
    // ----------------------------------------------------------------

    static std::string loadResource(
        const std::string &path,
        const std::unordered_set<std::string> &allowedExtensions,
        bool checkAllowed = true) {
        std::string fullPath;
        if (std::filesystem::path(path).is_absolute()) {
            fullPath = path;
        } else {
            std::filesystem::path currentPath = std::filesystem::current_path();
            fullPath = (currentPath / path).string();
        }

        std::string fileExtension =
            fullPath.substr(fullPath.find_last_of(".") + 1);

        if (checkAllowed &&
            allowedExtensions.find(fileExtension) == allowedExtensions.end()) {
            throw std::runtime_error("File type not allowed: " + fileExtension);
        }

        std::ifstream file(fullPath);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + fullPath);
        }

        try {
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        } catch (const std::exception &e) {
            throw std::runtime_error("Failed to read file: " + fullPath +
                                     ". Error: " + e.what());
        }
    }

#include OATPP_CODEGEN_BEGIN(ApiController)  //<- Begin Codegen

    // ----------------------------------------------------------------
    // All of the Web page requests handled here
    // ----------------------------------------------------------------

    ENDPOINT_INFO(IndexRequestHandler) { info->summary = "'Root' endpoint"; }
    ENDPOINT_ASYNC("GET", "/", IndexRequestHandler) {
        ENDPOINT_ASYNC_INIT(IndexRequestHandler);
        Action act() override {
            auto response = controller->createResponse(
                Status::CODE_200, loadResource("index.html", {"html"}));
            response->putHeader(Header::CONTENT_TYPE, "text/html");
            return _return(response);
        }
    };

    ENDPOINT_INFO(ClientRequestHandler) { info->summary = "'Client' endpoint"; }
    ENDPOINT_ASYNC("GET", "/client", ClientRequestHandler) {
        ENDPOINT_ASYNC_INIT(ClientRequestHandler);
        Action act() override {
            auto response = controller->createResponse(
                Status::CODE_200, loadResource("client/index.html", {"html"}));
            response->putHeader(Header::CONTENT_TYPE, "text/html");
            return _return(response);
        }
    };

    ENDPOINT_INFO(NoVNCRequestHandler) { info->summary = "'NoVNC' endpoint"; }
    ENDPOINT_ASYNC("GET", "/novnc", NoVNCRequestHandler) {
        ENDPOINT_ASYNC_INIT(NoVNCRequestHandler);
        Action act() override {
            auto response = controller->createResponse(
                Status::CODE_200,
                loadResource("module/novnc/index.html", {"html"}));
            response->putHeader(Header::CONTENT_TYPE, "text/html");
            return _return(response);
        }
    };

    ENDPOINT_INFO(WebSSHRequestHandler) { info->summary = "'WebSSH' endpoint"; }
    ENDPOINT_ASYNC("GET", "/webssh", WebSSHRequestHandler) {
        ENDPOINT_ASYNC_INIT(WebSSHRequestHandler);
        Action act() override {
            auto response = controller->createResponse(
                Status::CODE_200,
                loadResource("module/webssh/index.html", {"html"}));
            response->putHeader(Header::CONTENT_TYPE, "text/html");
            return _return(response);
        }
    };

    ENDPOINT_INFO(Static) { info->summary = "'Static File' endpoint"; }
    ENDPOINT_ASYNC("GET", "/static/*", Static) {
        ENDPOINT_ASYNC_INIT(Static);
        Action act() override {
            auto tail = request->getPathTail();
            OATPP_ASSERT_HTTP(tail, Status::CODE_400, "Empty filename");
            oatpp::parser::Caret caret(tail);
            auto pathLabel = caret.putLabel();
            caret.findChar('?');
            auto path = pathLabel.toString();
            auto buffer = loadResource(
                path.getValue(""),
                {"json", "js", "css", "html", "jpg", "png", "robot"});
            OATPP_ASSERT_HTTP(buffer != "", Status::CODE_500,
                              "Can't read file");
            return _return(
                controller->createResponse(Status::CODE_200, buffer));
        }
    };

    // ----------------------------------------------------------------
    // Load Static Files (a little bit silly, but this is working)
    // ----------------------------------------------------------------

    ENDPOINT_INFO(AllStaticCSS) { info->summary = "All 'CSS File' endpoint"; }
    ENDPOINT_ASYNC("GET", "/css/*", AllStaticCSS) {
        ENDPOINT_ASYNC_INIT(AllStaticCSS);
        Action act() override {
            auto tail = request->getPathTail();
            OATPP_ASSERT_HTTP(tail, Status::CODE_400, "Empty filename");
            oatpp::parser::Caret caret(tail);
            auto pathLabel = caret.putLabel();
            caret.findChar('?');
            auto path = "css/" + pathLabel.toString();
            auto buffer = loadResource(
                path.getValue(""),
                {"json", "js", "css", "html", "jpg", "png", "robot", "woff2",
                 "tff", "ico", "svg", "mp3", "oga", "woff", "ttf"});
            OATPP_ASSERT_HTTP(buffer != "", Status::CODE_500,
                              "Can't read file");
            return _return(
                controller->createResponse(Status::CODE_200, buffer));
        }
    };

    ENDPOINT_INFO(AllStaticJS) {
        info->summary = "All 'JavaScript File' endpoint";
    }
    ENDPOINT_ASYNC("GET", "/js/*", AllStaticJS) {
        ENDPOINT_ASYNC_INIT(AllStaticJS);
        Action act() override {
            auto tail = request->getPathTail();
            OATPP_ASSERT_HTTP(tail, Status::CODE_400, "Empty filename");
            oatpp::parser::Caret caret(tail);
            auto pathLabel = caret.putLabel();
            caret.findChar('?');
            auto path = "js/" + pathLabel.toString();
            auto buffer = loadResource(path.getValue(""), {"js"});
            OATPP_ASSERT_HTTP(buffer != "", Status::CODE_500,
                              "Can't read file");
            return _return(
                controller->createResponse(Status::CODE_200, buffer));
        }
    };

    ENDPOINT_INFO(AllStaticJSON) { info->summary = "All 'JSON File' endpoint"; }
    ENDPOINT_ASYNC("GET", "/json/*", AllStaticJSON) {
        ENDPOINT_ASYNC_INIT(AllStaticJSON);
        Action act() override {
            auto tail = request->getPathTail();
            OATPP_ASSERT_HTTP(tail, Status::CODE_400, "Empty filename");
            oatpp::parser::Caret caret(tail);
            auto pathLabel = caret.putLabel();
            caret.findChar('?');
            auto path = "json/" + pathLabel.toString();
            auto buffer = loadResource(path.getValue(""), {"json"});
            OATPP_ASSERT_HTTP(buffer != "", Status::CODE_500,
                              "Can't read file");
            return _return(
                controller->createResponse(Status::CODE_200, buffer));
        }
    };

    ENDPOINT_INFO(AllStaticFont) { info->summary = "All 'Font File' endpoint"; }
    ENDPOINT_ASYNC("GET", "/font/*", AllStaticFont) {
        ENDPOINT_ASYNC_INIT(AllStaticFont);
        Action act() override {
            auto tail = request->getPathTail();
            OATPP_ASSERT_HTTP(tail, Status::CODE_400, "Empty filename");
            oatpp::parser::Caret caret(tail);
            auto pathLabel = caret.putLabel();
            caret.findChar('?');
            auto path = "font/" + pathLabel.toString();
            auto buffer = loadResource(path.getValue(""),
                                       {"tff", "tff", "woff", "woff2", "eot"});
            OATPP_ASSERT_HTTP(buffer != "", Status::CODE_500,
                              "Can't read file");
            return _return(
                controller->createResponse(Status::CODE_200, buffer));
        }
    };

    ENDPOINT_INFO(AllStaticNodeModules) {
        info->summary =
            "All 'Node Modules File' endpoint. This need to be changed to "
            "normal "
            "format";
    }
    ENDPOINT_ASYNC("GET", "/node_modules/*", AllStaticNodeModules) {
        ENDPOINT_ASYNC_INIT(AllStaticNodeModules);
        Action act() override {
            auto tail = request->getPathTail();
            OATPP_ASSERT_HTTP(tail, Status::CODE_400, "Empty filename");
            oatpp::parser::Caret caret(tail);
            auto pathLabel = caret.putLabel();
            caret.findChar('?');
            auto path = "node_modules/" + pathLabel.toString();
            auto buffer = loadResource(path.getValue(""), {"css", "js"});
            OATPP_ASSERT_HTTP(buffer != "", Status::CODE_500,
                              "Can't read file");
            return _return(
                controller->createResponse(Status::CODE_200, buffer));
        }
    };

    ENDPOINT_INFO(AllStaticSound) {
        info->summary = "All 'Sound File' endpoint";
    }
    ENDPOINT_ASYNC("GET", "/sounds/*", AllStaticSound) {
        ENDPOINT_ASYNC_INIT(AllStaticSound);
        Action act() override {
            auto tail = request->getPathTail();
            OATPP_ASSERT_HTTP(tail, Status::CODE_400, "Empty filename");
            oatpp::parser::Caret caret(tail);
            auto pathLabel = caret.putLabel();
            caret.findChar('?');
            auto path = "sounds/" + pathLabel.toString();
            auto buffer = loadResource(path.getValue(""), {"oga", "mp3"});
            OATPP_ASSERT_HTTP(buffer != "", Status::CODE_500,
                              "Can't read file");
            return _return(
                controller->createResponse(Status::CODE_200, buffer));
        }
    };

    ENDPOINT_INFO(AllStaticTextures) {
        info->summary = "All 'Textures File' endpoint";
    }
    ENDPOINT_ASYNC("GET", "/textures/*", AllStaticTextures) {
        ENDPOINT_ASYNC_INIT(AllStaticTextures);
        Action act() override {
            auto tail = request->getPathTail();
            OATPP_ASSERT_HTTP(tail, Status::CODE_400, "Empty filename");
            oatpp::parser::Caret caret(tail);
            auto pathLabel = caret.putLabel();
            caret.findChar('?');
            auto path = "textures/" + pathLabel.toString();
            auto buffer =
                loadResource(path.getValue(""), {"gif", "png", "svg", "jpg"});
            OATPP_ASSERT_HTTP(buffer != "", Status::CODE_500,
                              "Can't read file");
            return _return(
                controller->createResponse(Status::CODE_200, buffer));
        }
    };

    ENDPOINT_INFO(AllStaticWebFonts) {
        info->summary = "All 'WebFonts File' endpoint";
    }
    ENDPOINT_ASYNC("GET", "/webfonts/*", AllStaticWebFonts) {
        ENDPOINT_ASYNC_INIT(AllStaticWebFonts);
        Action act() override {
            auto tail = request->getPathTail();
            OATPP_ASSERT_HTTP(tail, Status::CODE_400, "Empty filename");
            oatpp::parser::Caret caret(tail);
            auto pathLabel = caret.putLabel();
            caret.findChar('?');
            auto path = "webfonts/" + pathLabel.toString();
            auto buffer = loadResource(path.getValue(""),
                                       {"eot", "svg", "ttf", "woff", "woff2"});
            OATPP_ASSERT_HTTP(buffer != "", Status::CODE_500,
                              "Can't read file");
            return _return(
                controller->createResponse(Status::CODE_200, buffer));
        }
    };

    ENDPOINT_INFO(AllStaticWebAssets) {
        info->summary = "All 'Assets File' endpoint";
    }
    ENDPOINT_ASYNC("GET", "/assets/*", AllStaticWebAssets) {
        ENDPOINT_ASYNC_INIT(AllStaticWebAssets);
        Action act() override {
            auto tail = request->getPathTail();
            OATPP_ASSERT_HTTP(tail, Status::CODE_400, "Empty filename");
            oatpp::parser::Caret caret(tail);
            auto pathLabel = caret.putLabel();
            caret.findChar('?');
            auto path = "assets/" + pathLabel.toString();
            auto buffer = loadResource(path.getValue(""), {"css", "js"});
            OATPP_ASSERT_HTTP(buffer != "", Status::CODE_500,
                              "Can't read file");
            return _return(
                controller->createResponse(Status::CODE_200, buffer));
        }
    };

#include OATPP_CODEGEN_END(ApiController)  //<- End Codegen
};

#endif  // LITHIUM_ASYNC_STATIC_CONTROLLER_HPP
