/*
 * AsyncIOController.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: IO Route

**************************************************/

#ifndef LITHIUM_ASYNC_IO_CONTROLLER_HPP
#define LITHIUM_ASYNC_IO_CONTROLLER_HPP

#include "atom/io/compress.hpp"
#include "atom/io/file.hpp"
#include "atom/io/io.hpp"

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include "data/IODto.hpp"
#include "data/StatusDto.hpp"

#include "atom/type/json.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  //<- Begin Codegen

class IOController : public oatpp::web::server::api::ApiController {
public:
    IOController(const std::shared_ptr<ObjectMapper>& objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper) {}

    // ----------------------------------------------------------------
    // Pointer creator
    // ----------------------------------------------------------------

    static std::shared_ptr<IOController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
        return std::make_shared<IOController>(objectMapper);
    }

    // ----------------------------------------------------------------
    // IO Http Handler
    // ----------------------------------------------------------------

    ENDPOINT_INFO(getUICreateDirectory) {
        info->summary = "Create a directory with specific path";
        info->addConsumes<Object<CreateDirectoryDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/io/directory/create", getUICreateDirectory) {
        ENDPOINT_ASYNC_INIT(getUICreateDirectory);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<CreateDirectoryDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUICreateDirectory::returnResponse);
        }

        Action returnResponse(const oatpp::Object<CreateDirectoryDTO>& body) {
            OATPP_ASSERT_HTTP(body->path.getValue("") != "", Status::CODE_400,
                              "Missing Parameters");
            auto res = StatusDto::createShared();
            res->command = "createDirectory";
            auto path = body->path.getValue("");
            auto isAbsolute = body->isAbsolute.getValue(false);
            if (isAbsolute && !atom::io::isAbsolutePath(path)) {
                res->status = "error";
                res->error = "Invalid Parameters";
                res->message = "Directory path must be a absolute path";
            } else {
                if (!atom::io::createDirectory(path)) {
                    res->status = "error";
                    res->error = "IO Failed";
                    res->code = 500;
                    res->message = "Failed to create directory";
                } else {
                    res->status = "success";
                    res->message = "Successfully created directory";
                    res->code = 200;
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIRemoveDirectory) {
        info->summary = "Remove a directory with specific path";
        info->addConsumes<Object<CreateDirectoryDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/io/directory/remove", getUIRemoveDirectory) {
        ENDPOINT_ASYNC_INIT(getUIRemoveDirectory);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<CreateDirectoryDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIRemoveDirectory::returnResponse);
        }

        Action returnResponse(const oatpp::Object<CreateDirectoryDTO>& body) {
            OATPP_ASSERT_HTTP(body->path.getValue("") != "", Status::CODE_400,
                              "Missing Parameters");

            auto res = StatusDto::createShared();
            res->command = "removeDirectory";
            auto path = body->path.getValue("");
            auto isAbsolute = body->isAbsolute.getValue(false);

            if (isAbsolute && !atom::io::isAbsolutePath(path)) {
                res->status = "error";
                res->code = 500;
                res->error = "Invalid Parameters";
                res->message = "Directory path must be a absolute path";
            } else {
                if (!atom::io::removeDirectory(path)) {
                    res->status = "error";
                    res->code = 500;
                    res->error = "IO Failed";
                    res->message = "Failed to remove directory";
                } else {
                    res->status = "success";
                    res->message = "Successfully removed directory";
                    res->code = 200;
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIRenameDirectory) {
        info->summary = "Rename a directory with specific path and new name";
        info->addConsumes<Object<RenameDirectoryDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/io/directory/rename", getUIRenameDirectory) {
        ENDPOINT_ASYNC_INIT(getUIRenameDirectory);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RenameDirectoryDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIRenameDirectory::returnResponse);
        }

        Action returnResponse(const oatpp::Object<RenameDirectoryDTO>& body) {
            OATPP_ASSERT_HTTP(body->path.getValue("") != "", Status::CODE_400,
                              "Missing Parameters");
            OATPP_ASSERT_HTTP(body->name.getValue("") != "", Status::CODE_400,
                              "Missing Parameters");

            auto res = StatusDto::createShared();
            res->command = "renameDirectory";

            auto path = body->path.getValue("");
            auto isAbsolute = body->isAbsolute.getValue(false);
            auto name = body->name.getValue("");

            if (!atom::io::isFolderNameValid(name)) {
                res->status = "error";
                res->code = 500;
                res->error = "Invalid Parameters";
                res->message = "New folder name must be valid";
            }
            if (isAbsolute && !atom::io::isAbsolutePath(path)) {
                res->status = "error";
                res->code = 500;
                res->error = "Invalid Parameters";
                res->message = "Directory path must be a absolute path";
            } else {
                if (!atom::io::renameDirectory(path, name)) {
                    res->status = "error";
                    res->code = 500;
                    res->error = "IO Failed";
                    res->message = "Failed to rename directory";
                } else {
                    res->status = "success";
                    res->message = "Successfully renamed directory";
                    res->code = 200;
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIMoveDirectory) {
        info->summary = "Move a directory with specific path and new path";
        info->addConsumes<Object<MoveDirectoryDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/io/directory/move", getUIMoveDirectory) {
        ENDPOINT_ASYNC_INIT(getUIMoveDirectory);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<MoveDirectoryDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIMoveDirectory::returnResponse);
        }

        Action returnResponse(const oatpp::Object<MoveDirectoryDTO>& body) {
            OATPP_ASSERT_HTTP(body->path.getValue("") != "", Status::CODE_400,
                              "Missing Parameters");
            OATPP_ASSERT_HTTP(body->new_path.getValue("") != "",
                              Status::CODE_400, "Missing Parameters");

            auto res = StatusDto::createShared();
            res->command = "moveDirectory";
            auto old_path = body->path.getValue("");
            auto new_path = body->new_path.getValue("");
            if ((!atom::io::isAbsolutePath(old_path) ||
                 !atom::io::isAbsolutePath(new_path))) {
                res->status = "error";
                res->code = 500;
                res->error = "Invalid Parameters";
                res->message = "Directory path must be a absolute path";
            } else {
                if (!atom::io::moveDirectory(old_path, new_path)) {
                    res->status = "error";
                    res->code = 500;
                    res->error = "IO Failed";
                    res->message = "Failed to move directory";
                } else {
                    res->status = "success";
                    res->message = "Successfully moved directory";
                    res->code = 200;
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUICopyFile) {
        info->summary = "Copy a file to a new path";
        info->addConsumes<Object<CopyFileDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/io/file/copy", getUICopyFile) {
        ENDPOINT_ASYNC_INIT(getUICopyFile);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<CopyFileDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUICopyFile::returnResponse);
        }

        Action returnResponse(const oatpp::Object<CopyFileDTO>& body) {
            OATPP_ASSERT_HTTP(body->path.getValue("") != "", Status::CODE_400,
                              "Missing Parameters");
            OATPP_ASSERT_HTTP(body->new_path.getValue("") != "",
                              Status::CODE_400, "Missing Parameters");

            auto res = StatusDto::createShared();
            res->command = "copyFile";
            auto old_path = body->path.getValue("");
            auto new_path = body->new_path.getValue("");
            auto isAbsolute = body->isAbsolute.getValue(false);

            if (isAbsolute && !atom::io::isAbsolutePath(old_path) ||
                !atom::io::isAbsolutePath(new_path)) {
                res->status = "error";
                res->code = 500;
                res->error = "Invalid Parameters";
                res->message = "Directory path must be a absolute path";
            } else {
                if (!atom::io::copyFile(old_path, new_path)) {
                    res->status = "error";
                    res->code = 500;
                    res->error = "IO Failed";
                    res->message = "Failed to copy file";
                } else {
                    res->status = "success";
                    res->message = "Successfully moved file";
                    res->code = 200;
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIMoveFile) {
        info->summary = "Move a file to a new path";
        info->addConsumes<Object<MoveFileDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/io/file/move", getUIMoveFile) {
        ENDPOINT_ASYNC_INIT(getUIMoveFile);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<MoveFileDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIMoveFile::returnResponse);
        }

        Action returnResponse(const oatpp::Object<MoveFileDTO>& body) {
            OATPP_ASSERT_HTTP(body->path.getValue("") != "", Status::CODE_400,
                              "Missing Parameters");
            OATPP_ASSERT_HTTP(body->new_path.getValue("") != "",
                              Status::CODE_400, "Missing Parameters");

            auto res = StatusDto::createShared();
            res->command = "moveFile";

            auto old_path = body->path.getValue("");
            auto new_path = body->new_path.getValue("");
            auto isAbsolute = body->isAbsolute.getValue(false);

            if (isAbsolute && !atom::io::isAbsolutePath(old_path) ||
                !atom::io::isAbsolutePath(new_path)) {
                res->status = "error";
                res->code = 500;
                res->error = "Invalid Parameters";
                res->message = "Directory path must be a absolute path";
            } else {
                if (!atom::io::copyFile(old_path, new_path)) {
                    res->status = "error";
                    res->code = 500;
                    res->error = "IO Failed";
                    res->message = "Failed to move file";
                } else {
                    res->status = "success";
                    res->message = "Successfully moved file";
                    res->code = 200;
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIRenameFile) {
        info->summary = "Move a file to a new path";
        info->addConsumes<Object<RenameFileDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/io/file/rename", getUIRenameFile) {
        ENDPOINT_ASYNC_INIT(getUIRenameFile);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RenameFileDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIRenameFile::returnResponse);
        }

        Action returnResponse(const oatpp::Object<RenameFileDTO>& body) {
            OATPP_ASSERT_HTTP(body->path.getValue("") != "", Status::CODE_400,
                              "Missing Parameters");
            OATPP_ASSERT_HTTP(body->new_name.getValue("") != "",
                              Status::CODE_400, "Missing Parameters");

            auto res = StatusDto::createShared();
            res->command = "renameFile";

            auto old_name = body->path.getValue("");
            auto new_name = body->new_name.getValue("");
            auto isAbsolute = body->isAbsolute.getValue(false);

            if (isAbsolute && !atom::io::isAbsolutePath(old_name)) {
                res->status = "error";
                res->code = 500;
                res->error = "Invalid Parameters";
                res->message = "Directory path must be a absolute path";
            } else {
                if (!atom::io::renameFile(old_name, new_name)) {
                    res->status = "error";
                    res->code = 500;
                    res->error = "IO Failed";
                    res->message = "Failed to rename file";
                } else {
                    res->status = "success";
                    res->message = "Successfully renamed file";
                    res->code = 200;
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIRemoveFile) {
        info->summary = "Remove a file with full path";
        info->addConsumes<Object<RemoveFileDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/io/file/remove", getUIRemoveFile) {
        ENDPOINT_ASYNC_INIT(getUIRemoveFile);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RemoveFileDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIRemoveFile::returnResponse);
        }

        Action returnResponse(const oatpp::Object<RemoveFileDTO>& body) {
            OATPP_ASSERT_HTTP(body->path.getValue("") != "", Status::CODE_400,
                              "Missing Parameters");

            auto res = StatusDto::createShared();
            res->command = "removeFile";

            auto name = body->path.getValue("");
            auto isAbsolute = body->isAbsolute.getValue(false);

            if (isAbsolute && !atom::io::isAbsolutePath(name)) {
                res->status = "error";
                res->code = 500;
                res->error = "Invalid Parameters";
                res->message = "Directory path must be a absolute path";
            }
            if (!atom::io::removeFile(name)) {
                res->error = "IO Failed";
                res->message = "Failed to remove file";
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };
};

#include OATPP_CODEGEN_END(ApiController)  //<- End Codegen

#endif  // LITHIUM_ASYNC_IO_CONTROLLER_HPP