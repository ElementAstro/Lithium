/*
 * exception.hpp
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

Date: 2023-11-10

Description: Better Exception Library

**************************************************/

#pragma once

#include <stdexcept>

namespace Atom::Utils::Exception
{
	class ObjectAlreadyExist_Error : public std::logic_error
	{
	public:
		ObjectAlreadyExist_Error(const char *msg) : std::logic_error(msg){};
	};

	class ObjectNotExist_Error : public std::logic_error
	{
	public:
		ObjectNotExist_Error(const char *msg) : std::logic_error(msg){};
	};

	class ObjectUninitialized_Error : public std::logic_error
	{
	public:
		ObjectUninitialized_Error(const char *msg) : std::logic_error(msg){};
	};

	class Uninitialization_Error : public std::logic_error
	{
	public:
		Uninitialization_Error(const char *msg) : std::logic_error(msg){};
	};

	class WrongArgument_Error : public std::logic_error
	{
	public:
		WrongArgument_Error(const char *msg) : std::logic_error(msg){};
	};

	class InvalidArgument_Error : public std::logic_error
	{
	public:
		InvalidArgument_Error(const char *msg) : std::logic_error(msg){};
	};

	class MissingArgument_Error : public std::logic_error
	{
	public:
		MissingArgument_Error(const char *msg) : std::logic_error(msg){};
	};

	class UnlawfulOperation_Error : public std::logic_error
	{
	public:
		UnlawfulOperation_Error(const char *msg) : std::logic_error(msg){};
	};

	class Unkown_Error : public std::logic_error
	{
	public:
		Unkown_Error(const char *msg) : std::logic_error(msg){};
	};

	class SystemCollapse_Error : public std::runtime_error
	{
	public:
		SystemCollapse_Error(const char *msg) : std::runtime_error(msg){};
	};

	class NullPointer_Error : public std::logic_error
	{
	public:
		NullPointer_Error(const char *msg) : std::logic_error(msg){};
	};

	class NotFound_Error : public std::logic_error
	{
	public:
		NotFound_Error(const char *msg) : std::logic_error(msg){};
	};

	class FileNotFound_Error : public std::logic_error
	{
	public:
		FileNotFound_Error(const char *msg) : std::logic_error(msg){};
	};

	class FileNotReadable_Error : public std::logic_error
	{
	public:
		FileNotReadable_Error(const char *msg) : std::logic_error(msg){};
	};

	class FileNotWritable_Error : public std::logic_error
	{
	public:
		FileNotWritable_Error(const char *msg) : std::logic_error(msg){};
	};

	class FileUnknown_Error : public std::logic_error
	{
	public:
		FileUnknown_Error(const char *msg) : std::logic_error(msg){};
	};

	class Conflict_Error : public std::logic_error
	{
	public:
		Conflict_Error(const char *msg) : std::logic_error(msg){};
	};

	class FailToLoadDll_Error : public std::runtime_error
	{
	public:
		FailToLoadDll_Error(const char *msg) : std::runtime_error(msg){};
	};

	class FailToUnloadDll_Error : public std::runtime_error
	{
	public:
		FailToUnloadDll_Error(const char *msg) : std::runtime_error(msg){};
	};

	class FailToGetFunction_Error : public std::runtime_error
	{
	public:
		FailToGetFunction_Error(const char *msg) : std::runtime_error(msg){};
	};

	class FailToCreateObject_Error : public std::runtime_error
	{
	public:
		FailToCreateObject_Error(const char *msg) : std::runtime_error(msg){};
	};

	class FailToDestroyObject_Error : public std::runtime_error
	{
	public:
		FailToDestroyObject_Error(const char *msg) : std::runtime_error(msg){};
	};

	class FailToCallFunction_Error : public std::runtime_error
	{
	public:
		FailToCallFunction_Error(const char *msg) : std::runtime_error(msg){};
	};

	class FailToCallMemberFunction_Error : public std::runtime_error
	{
	public:
		FailToCallMemberFunction_Error(const char *msg) : std::runtime_error(msg){};
	};

	class FailToCallStaticFunction_Error : public std::runtime_error
	{
	public:
		FailToCallStaticFunction_Error(const char *msg) : std::runtime_error(msg){};
	};
}
