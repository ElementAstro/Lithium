/*
 * TweakerDto.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-11-17

Description: Data Transform Object for Tweaker Controller

**************************************************/

#ifndef TWEAKERDTO_HPP
#define TWEAKERDTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO) ///< Begin DTO codegen section

class TDeviceConnectDTO : public oatpp::DTO
{
    DTO_INIT(TDeviceConnectDTO, DTO)
};

class TDriverConnectDTO : public oatpp::DTO
{
    DTO_INIT(TDriverConnectDTO, DTO)
};

class TGlobalParameterDTO : public oatpp::DTO
{
    DTO_INIT(TGlobalParameterDTO, DTO)
};

class TPHD2DTO : public oatpp::DTO
{
    DTO_INIT(TPHD2DTO, DTO)
};

class TPAADTO : public oatpp::DTO
{
    DTO_INIT(TPAADTO, DTO)
};

#include OATPP_CODEGEN_END(DTO) ///< End DTO codegen section

#endif