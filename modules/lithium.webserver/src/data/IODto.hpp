/*
 * IODto.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-25

Description: Data Transform Object for IO Controller

**************************************************/

#ifndef IODTO_HPP
#define IODTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)  ///< Begin DTO codegen section

class BaseIODto : public oatpp::DTO {
    DTO_INIT(BaseIODto, DTO)
    DTO_FIELD_INFO(path) {
        info->description =
            "the path of the directory you want to create or remove, must be "
            "full path";
        info->required = true;
    }
    DTO_FIELD(String, path);

    DTO_FIELD_INFO(isAbsolute) {
        info->description = "whether the path is absolute or not";
        info->required = true;
    }
    DTO_FIELD(Boolean, isAbsolute);
};

class CreateDirectoryDTO : public BaseIODto {
    DTO_INIT(CreateDirectoryDTO, DTO)
};

class RenameDirectoryDTO : public BaseIODto {
    DTO_INIT(RenameDirectoryDTO, DTO)

    DTO_FIELD_INFO(name) {
        info->description =
            "the new name of the directory you want to rename, must be valid";
        info->required = true;
    }
    DTO_FIELD(String, name);
};

class MoveDirectoryDTO : public BaseIODto {
    DTO_INIT(MoveDirectoryDTO, DTO)

    DTO_FIELD_INFO(new_path) {
        info->description =
            "the new path of the directory you want to move to, must be valid";
        info->required = true;
    }
    DTO_FIELD(String, new_path);
};

class CopyFileDTO : public BaseIODto {
    DTO_INIT(CopyFileDTO, DTO)

    DTO_FIELD_INFO(new_path) {
        info->description = "the new path of the file, must be valid";
        info->required = true;
    }
    DTO_FIELD(String, new_path);
};

class MoveFileDTO : public BaseIODto {
    DTO_INIT(MoveFileDTO, DTO)

    DTO_FIELD_INFO(new_path) {
        info->description = "the new path of the file, must be valid";
        info->required = true;
    }
    DTO_FIELD(String, new_path);
};

class RenameFileDTO : public BaseIODto {
    DTO_INIT(RenameFileDTO, DTO)

    DTO_FIELD_INFO(new_name) {
        info->description = "the new name of the file, must be valid";
        info->required = true;
    }
    DTO_FIELD(String, new_name);
};

class RemoveFileDTO : public BaseIODto {
    DTO_INIT(RemoveFileDTO, DTO)
};

#include OATPP_CODEGEN_END(DTO)  ///< End DTO codegen section

#endif
