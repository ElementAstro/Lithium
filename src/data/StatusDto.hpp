/*
 * StatusDto.hpp
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

Description: Status Data Transform Object

**************************************************/

#ifndef STATUSDTO_HPP
#define STATUSDTO_HPP

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class StatusDto : public oatpp::DTO
{

  DTO_INIT(StatusDto, DTO)

  DTO_FIELD_INFO(status)
  {
    info->description = "Short status text";
  }
  DTO_FIELD(String, status);

  DTO_FIELD_INFO(code)
  {
    info->description = "Status code";
  }
  DTO_FIELD(Int32, code);

  DTO_FIELD_INFO(message)
  {
    info->description = "Verbose message";
  }
  DTO_FIELD(String, message);

  DTO_FIELD_INFO(error)
  {
    info->description = "Verbose error message";
  }
  DTO_FIELD(String, error);

  DTO_FIELD_INFO(command)
  {
    info->description = "Command";
  }
  DTO_FIELD(String, command);
};

#include OATPP_CODEGEN_END(DTO)

#endif // STATUSDTO_HPP
