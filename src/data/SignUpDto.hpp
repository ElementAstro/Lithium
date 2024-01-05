/*
 * SignUpDto.hpp
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

Date: 2023-7-25

Description: SignUp Data Transform Object

**************************************************/

#ifndef SIGNUPDTO_HPP
#define SIGNUPDTO_HPP

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class SignUpDto : public oatpp::DTO
{

	DTO_INIT(SignUpDto, DTO)

	DTO_FIELD(String, userName, "username");
	DTO_FIELD(String, email, "email");
	DTO_FIELD(String, password, "password");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* SIGNUPDTO_HPP */
