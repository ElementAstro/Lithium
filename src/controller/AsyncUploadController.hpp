/*
 * UploadController.cpp
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

Description: Upload Route

**************************************************/

#ifndef Lithium_UPLOADCONTROLLER_HPP
#define Lithium_UPLOADCONTROLLER_HPP

#include "config.h"

#include "oatpp/web/mime/multipart/TemporaryFileProvider.hpp"
#include "oatpp/web/mime/multipart/InMemoryDataProvider.hpp"
#include "oatpp/web/mime/multipart/FileProvider.hpp"
#include "oatpp/web/mime/multipart/Multipart.hpp"
#include "oatpp/web/mime/multipart/Reader.hpp"
#include "oatpp/web/mime/multipart/PartList.hpp"
#include "oatpp/web/mime/multipart/PartReader.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

namespace multipart = oatpp::web::mime::multipart;

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class UploadController : public oatpp::web::server::api::ApiController
{
public:
    UploadController(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
    {
    }

public:
    static std::shared_ptr<UploadController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    {
        return std::make_shared<UploadController>(objectMapper);
    }

    ENDPOINT_INFO(MultipartUploadToFile)
    {
        info->summary = "Upload File To File";
    }
    ENDPOINT_ASYNC("POST", "/api/upload/file", MultipartUploadToFile)
    {

        ENDPOINT_ASYNC_INIT(MultipartUploadToFile)

        /* Coroutine State */
        std::shared_ptr<multipart::PartList> m_multipart;
        std::shared_ptr<oatpp::data::stream::BufferOutputStream> m_bufferStream = std::make_shared<oatpp::data::stream::BufferOutputStream>();

        Action act() override
        {

            m_multipart = std::make_shared<multipart::PartList>(request->getHeaders());
            auto multipartReader = std::make_shared<multipart::AsyncReader>(m_multipart);

            multipartReader->setPartReader("file", multipart::createAsyncFilePartReader("./tmp"));

            multipartReader->setDefaultPartReader(multipart::createAsyncInMemoryPartReader(16 * 1024 /* max-data-size */));

            return request->transferBodyAsync(multipartReader).next(yieldTo(&MultipartUploadToFile::onUploaded));
        }

        Action onUploaded()
        {

            oatpp::String fileData;

            auto file = m_multipart->getNamedPart("file");
            if (file)
            {
                return oatpp::data::stream::transferAsync(file->getPayload()->openInputStream(), m_bufferStream, 0, oatpp::data::buffer::IOBuffer::createShared())
                    .next(yieldTo(&MultipartUploadToFile::sendResponseWithFileData));
            }

            return _return(controller->createDtoResponse(Status::CODE_200, oatpp::Fields<oatpp::Any>({{"code", oatpp::Int32(200)},
                                                                                                      {"message", oatpp::String("OK")},
                                                                                                      {"parts-uploaded", oatpp::Int32(m_multipart->count())},
                                                                                                      { "file-data",
                                                                                                        nullptr /* no file data */ }})));
        }

        Action sendResponseWithFileData()
        {
            return _return(controller->createDtoResponse(Status::CODE_200, oatpp::Fields<oatpp::Any>({{"code", oatpp::Int32(200)},
                                                                                                      {"message", oatpp::String("OK")},
                                                                                                      {"parts-uploaded", oatpp::Int32(m_multipart->count())},
                                                                                                      { "file-data",
                                                                                                        m_bufferStream->toString() }})));
        }
    };

    ENDPOINT_INFO(MultipartUploadToMemory)
    {
        info->summary = "Upload File To Memory";
    }
    ENDPOINT_ASYNC("POST", "test/multipart-all", MultipartUploadToMemory) {

        ENDPOINT_ASYNC_INIT(MultipartUploadToMemory)

        /* Coroutine State */
        std::shared_ptr<multipart::PartList> m_multipart;
        std::shared_ptr<oatpp::data::stream::BufferOutputStream> m_bufferStream = std::make_shared<oatpp::data::stream::BufferOutputStream>();

        Action act() override
        {
            m_multipart = std::make_shared<multipart::PartList>(request->getHeaders());
            auto multipartReader = std::make_shared<multipart::AsyncReader>(m_multipart);

            /* Configure to read part with name "part1" into memory */
            multipartReader->setPartReader("file", multipart::createAsyncInMemoryPartReader(256 /* max-data-size */));

            multipartReader->setDefaultPartReader(multipart::createAsyncInMemoryPartReader(16 * 1024 /* max-data-size */));
            /* Read multipart body */
            return request->transferBodyAsync(multipartReader).next(yieldTo(&MultipartUploadToMemory::onUploaded));
        }

        Action onUploaded()
        {
            /* Get multipart by name */
            auto content = m_multipart->getNamedPart("file");
            /* Asser part not-null */
            OATPP_ASSERT_HTTP(content, Status::CODE_400, "file is null");
            return _return(controller->createResponse(Status::CODE_200, "OK"));
        }

    };
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif // Lithium_UPLOADCONTROLLER_HPP