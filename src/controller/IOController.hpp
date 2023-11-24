/*
 * IOController.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-7-13

Description: IO Route

**************************************************/

#ifndef Lithium_IOCONTROLLER_HPP
#define Lithium_IOCONTROLLER_HPP

#include "config.h"

#include "atom/io/io.hpp"
#include "atom/io/file.hpp"
#include "atom/io/compress.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "data/IODto.hpp"
#include "data/StatusDto.hpp"

#include "atom/type/json.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class IOController : public oatpp::web::server::api::ApiController
{
public:
    IOController(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
    {
    }

public:
    static std::shared_ptr<IOController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    {
        return std::make_shared<IOController>(objectMapper);
    }

    ENDPOINT_INFO(getUICreateDirectory)
    {
        info->summary = "Create a directory with specific path";
        info->addConsumes<Object<CreateDirectoryDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
#if ENABLE_ASYNC
    ENDPOINT_ASYNC("GET", "/api/io/directory/create", getUICreateDirectory)
    {
        ENDPOINT_ASYNC_INIT(getUICreateDirectory)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<CreateDirectoryDTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUICreateDirectory::returnResponse);
        }

        Action returnResponse(const oatpp::Object<CreateDirectoryDTO>& body)
        {
            auto res = StatusDto::createShared();
            if(body->path.getValue("") == "")
            {
                res->error = "Invalid Parameters";
                res->message = "Directory path is required";
            }
            else
            {
                auto path = body->path.getValue("");
                if(!Lithium::File::is_full_path(path))
                {
                    res->error = "Invalid Parameters";
                    res->message = "Directory path must be a absolute path";
                }
                else
                {
                    if(!Lithium::File::create_directory(path))
                    {
                        res->error = "IO Failed";
                        res->message = "Failed to create directory";
                    }
                }
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };
#else

#endif

    ENDPOINT_INFO(getUIRemoveDirectory)
    {
        info->summary = "Remove a directory with specific path";
        info->addConsumes<Object<CreateDirectoryDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
#if ENABLE_ASYNC
    ENDPOINT_ASYNC("GET", "/api/io/directory/remove", getUIRemoveDirectory)
    {
        ENDPOINT_ASYNC_INIT(getUIRemoveDirectory)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<CreateDirectoryDTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUIRemoveDirectory::returnResponse);
        }

        Action returnResponse(const oatpp::Object<CreateDirectoryDTO>& body)
        {
            auto res = StatusDto::createShared();
            if(body->path.getValue("") == "")
            {
                res->error = "Invalid Parameters";
                res->message = "Directory path is required";
            }
            else
            {
                auto path = body->path.getValue("");
                if(!Lithium::File::is_full_path(path))
                {
                    res->error = "Invalid Parameters";
                    res->message = "Directory path must be a absolute path";
                }
                else
                {
                    if(!Lithium::File::remove_directory(path))
                    {
                        res->error = "IO Failed";
                        res->message = "Failed to remove directory";
                    }
                }
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };
#else

#endif

    ENDPOINT_INFO(getUIRenameDirectory)
    {
        info->summary = "Rename a directory with specific path and new name";
        info->addConsumes<Object<RenameDirectoryDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
#if ENABLE_ASYNC
    ENDPOINT_ASYNC("GET", "/api/io/directory/rename", getUIRenameDirectory)
    {
        ENDPOINT_ASYNC_INIT(getUIRenameDirectory)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<RenameDirectoryDTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUIRenameDirectory::returnResponse);
        }

        Action returnResponse(const oatpp::Object<RenameDirectoryDTO>& body)
        {
            auto res = StatusDto::createShared();
            if(body->path.getValue("") == "" || body->name.getValue("") == "")
            {
                res->error = "Invalid Parameters";
                res->message = "Directory path and name are required";
            }
            else
            {
                auto path = body->path.getValue("");
                if(!Lithium::File::is_full_path(path))
                {
                    res->error = "Invalid Parameters";
                    res->message = "Directory path must be a absolute path";
                }
                else
                {
                    auto name = body->name.getValue("");
                    if(!Lithium::File::isFolderNameValid(name))
                    {
                        res->error = "Invalid Parameters";
                        res->message = "New folder name must be valid";
                    }
                    else
                    {
                        if(!Lithium::File::rename_directory(path,name))
                        {
                            res->error = "IO Failed";
                            res->message = "Failed to rename directory";
                        }
                    }
                }
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };
#else

#endif

    ENDPOINT_INFO(getUIMoveDirectory)
    {
        info->summary = "Move a directory with specific path and new path";
        info->addConsumes<Object<MoveDirectoryDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
#if ENABLE_ASYNC
    ENDPOINT_ASYNC("GET", "/api/io/directory/move", getUIMoveDirectory)
    {
        ENDPOINT_ASYNC_INIT(getUIMoveDirectory)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<MoveDirectoryDTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUIMoveDirectory::returnResponse);
        }

        Action returnResponse(const oatpp::Object<MoveDirectoryDTO>& body)
        {
            auto res = StatusDto::createShared();
            if(body->old_path.getValue("") == "" || body->new_path.getValue("") == "")
            {
                res->error = "Invalid Parameters";
                res->message = "Directory old path and new path are required";
            }
            else
            {
                auto old_path = body->old_path.getValue("");
                auto new_path = body->new_path.getValue("");
                if(!Lithium::File::is_full_path(old_path) || !Lithium::File::is_full_path(new_path))
                {
                    res->error = "Invalid Parameters";
                    res->message = "Directory path must be a absolute path";
                }
                else
                {
                    if(!Lithium::File::move_directory(old_path,new_path))
                    {
                        res->error = "IO Failed";
                        res->message = "Failed to move directory";
                    }
                }
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };
#else

#endif

    ENDPOINT_INFO(getUICopyFile)
    {
        info->summary = "Copy a file to a new path";
        info->addConsumes<Object<CopyFileDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
#if ENABLE_ASYNC
    ENDPOINT_ASYNC("GET", "/api/io/file/copy", getUICopyFile)
    {
        ENDPOINT_ASYNC_INIT(getUICopyFile)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<CopyFileDTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUICopyFile::returnResponse);
        }

        Action returnResponse(const oatpp::Object<CopyFileDTO>& body)
        {
            auto res = StatusDto::createShared();
            if(body->old_path.getValue("") == "" || body->new_path.getValue("") == "")
            {
                res->error = "Invalid Parameters";
                res->message = "File old path and new path are required";
            }
            else
            {
                auto old_path = body->old_path.getValue("");
                auto new_path = body->new_path.getValue("");
                if(!Lithium::File::is_full_path(old_path) || !Lithium::File::is_full_path(new_path))
                {
                    res->error = "Invalid Parameters";
                    res->message = "Directory path must be a absolute path";
                }
                else
                {
                    if(!Lithium::File::copy_file(old_path,new_path))
                    {
                        res->error = "IO Failed";
                        res->message = "Failed to copy file";
                    }
                }
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };
#else

#endif

    ENDPOINT_INFO(getUIMoveFile)
    {
        info->summary = "Move a file to a new path";
        info->addConsumes<Object<MoveFileDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
#if ENABLE_ASYNC
    ENDPOINT_ASYNC("GET", "/api/io/file/move", getUIMoveFile)
    {
        ENDPOINT_ASYNC_INIT(getUIMoveFile)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<MoveFileDTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUIMoveFile::returnResponse);
        }

        Action returnResponse(const oatpp::Object<MoveFileDTO>& body)
        {
            auto res = StatusDto::createShared();
            if(body->old_path.getValue("")  == "" || body->new_path.getValue("") == "")
            {
                res->error = "Invalid Parameters";
                res->message = "File old path and new path are required";
            }
            else
            {
                auto old_path = body->old_path.getValue("");
                auto new_path = body->new_path.getValue("");
                if(!Lithium::File::is_full_path(old_path) || !Lithium::File::is_full_path(new_path))
                {
                    res->error = "Invalid Parameters";
                    res->message = "Directory path must be a absolute path";
                }
                else
                {
                    if(!Lithium::File::copy_file(old_path,new_path))
                    {
                        res->error = "IO Failed";
                        res->message = "Failed to move file";
                    }
                }
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };
#else

#endif

    ENDPOINT_INFO(getUIRenameFile)
    {
        info->summary = "Move a file to a new path";
        info->addConsumes<Object<RenameFileDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
#if ENABLE_ASYNC
    ENDPOINT_ASYNC("GET", "/api/io/file/rename", getUIRenameFile)
    {
        ENDPOINT_ASYNC_INIT(getUIRenameFile)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<RenameFileDTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUIRenameFile::returnResponse);
        }

        Action returnResponse(const oatpp::Object<RenameFileDTO>& body)
        {
            auto res = StatusDto::createShared();
            if(body->old_name.getValue("")  == "" || body->new_name.getValue("")  == "")
            {
                res->error = "Invalid Parameters";
                res->message = "File old name and new name are required";
            }
            else
            {
                auto old_name = body->old_name.getValue("");
                auto new_name = body->new_name.getValue("");

                if(!Lithium::File::rename_file(old_name,new_name))
                {
                    res->error = "IO Failed";
                    res->message = "Failed to rename file";
                }
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };
#else

#endif

    ENDPOINT_INFO(getUIRemoveFile)
    {
        info->summary = "Remove a file with full path";
        info->addConsumes<Object<RemoveFileDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
#if ENABLE_ASYNC
    ENDPOINT_ASYNC("GET", "/api/io/file/remove", getUIRemoveFile)
    {
        ENDPOINT_ASYNC_INIT(getUIRemoveFile)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<RemoveFileDTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUIRemoveFile::returnResponse);
        }

        Action returnResponse(const oatpp::Object<RemoveFileDTO>& body)
        {
            auto res = StatusDto::createShared();
            if(body->name.getValue("") == "")
            {
                res->error = "Invalid Parameters";
                res->message = "File name is required";
            }
            else
            {
                auto name = body->name.getValue("");
                if(!Lithium::File::remove_file(name))
                {
                    res->error = "IO Failed";
                    res->message = "Failed to remove file";
                }
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };
#else

#endif
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif // Lithium_IOCONTROLLER_HPP