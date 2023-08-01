/*
 * IODto.hpp
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

Date: 2023-7-25

Description: Data Transform Object for IO Controller

**************************************************/

#ifndef IODTO_HPP
#define IODTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO) ///< Begin DTO codegen section

class CreateDirectoryDTO : public oatpp::DTO
{
    DTO_INIT(CreateDirectoryDTO, DTO)
    DTO_FIELD_INFO(path)
    {
        info->description = "the path of the directory you want to create or remove, must be full path";
        info->required = true;
    }
    DTO_FIELD(String, path);
};

class RenameDirectoryDTO : public oatpp::DTO
{
    DTO_INIT(RenameDirectoryDTO, DTO)
    DTO_FIELD_INFO(path)
    {
        info->description = "the path of the directory you want to create , must be full path";
        info->required = true;
    }
    DTO_FIELD(String, path);

    DTO_FIELD_INFO(name)
    {
        info->description = "the new name of the directory you want to rename, must be valid";
        info->required = true;
    }
    DTO_FIELD(String, name);
};

class MoveDirectoryDTO : public oatpp::DTO
{
    DTO_INIT(MoveDirectoryDTO, DTO)
    DTO_FIELD_INFO(old_path)
    {
        info->description = "the path of the directory you want to create , must be full path";
        info->required = true;
    }
    DTO_FIELD(String, old_path);

    DTO_FIELD_INFO(new_path)
    {
        info->description = "the new path of the directory you want to move to, must be valid";
        info->required = true;
    }
    DTO_FIELD(String, new_path);
};

class CopyFileDTO : public oatpp::DTO
{
    DTO_INIT(CopyFileDTO, DTO)
    DTO_FIELD_INFO(old_path)
    {
        info->description = "the path of the file you want to copy, must be full path";
        info->required = true;
    }
    DTO_FIELD(String, old_path);

    DTO_FIELD_INFO(new_path)
    {
        info->description = "the new path of the file, must be valid";
        info->required = true;
    }
    DTO_FIELD(String, new_path);
};

class MoveFileDTO : public oatpp::DTO
{
    DTO_INIT(MoveFileDTO, DTO)
    DTO_FIELD_INFO(old_path)
    {
        info->description = "the path of the file you want to move, must be full path";
        info->required = true;
    }
    DTO_FIELD(String, old_path);

    DTO_FIELD_INFO(new_path)
    {
        info->description = "the new path of the file, must be valid";
        info->required = true;
    }
    DTO_FIELD(String, new_path);
};

class RenameFileDTO : public oatpp::DTO
{
    DTO_INIT(RenameFileDTO, DTO)
    DTO_FIELD_INFO(old_name)
    {
        info->description = "the old name of the file";
        info->required = true;
    }
    DTO_FIELD(String, old_name);

    DTO_FIELD_INFO(new_name)
    {
        info->description = "the new name of the file, must be valid";
        info->required = true;
    }
    DTO_FIELD(String, new_name);
};

class RemoveFileDTO : public oatpp::DTO
{
    DTO_INIT(RemoveFileDTO, DTO)

    DTO_FIELD_INFO(name)
    {
        info->description = "the name of the file, must be valid";
        info->required = true;
    }
    DTO_FIELD(String, name);
};

#include OATPP_CODEGEN_END(DTO) ///< End DTO codegen section

#endif