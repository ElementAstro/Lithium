/*
 * exception.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Better Exception Library

**************************************************/

#ifndef ATOM_ERROR_EXCEPTION_HPP
#define ATOM_ERROR_EXCEPTION_HPP

#include <stdexcept>
#include <string>

namespace Atom::Error
{
	class ObjectAlreadyExist : public std::logic_error
	{
	public:
		ObjectAlreadyExist(const std::string &msg) : std::logic_error(msg){};
	};

	class ObjectNotExist : public std::logic_error
	{
	public:
		ObjectNotExist(const std::string &msg) : std::logic_error(msg){};
	};

	class ObjectUninitialized : public std::logic_error
	{
	public:
		ObjectUninitialized(const std::string &msg) : std::logic_error(msg){};
	};

	class Uninitialization : public std::logic_error
	{
	public:
		Uninitialization(const std::string &msg) : std::logic_error(msg){};
	};

	class WrongArgument : public std::logic_error
	{
	public:
		WrongArgument(const std::string &msg) : std::logic_error(msg){};
	};

	class InvalidArgument : public std::logic_error
	{
	public:
		InvalidArgument(const std::string &msg) : std::logic_error(msg){};
	};

	class MissingArgument : public std::logic_error
	{
	public:
		MissingArgument(const std::string &msg) : std::logic_error(msg){};
	};

	class UnlawfulOperation : public std::logic_error
	{
	public:
		UnlawfulOperation(const std::string &msg) : std::logic_error(msg){};
	};

	class Unkown : public std::logic_error
	{
	public:
		Unkown(const std::string &msg) : std::logic_error(msg){};
	};

	class SystemCollapse : public std::runtime_error
	{
	public:
		SystemCollapse(const std::string &msg) : std::runtime_error(msg){};
	};

	class NullPointer : public std::logic_error
	{
	public:
		NullPointer(const std::string &msg) : std::logic_error(msg){};
	};

	class NotFound : public std::logic_error
	{
	public:
		NotFound(const std::string &msg) : std::logic_error(msg){};
	};

	class FileNotFound : public std::logic_error
	{
	public:
		FileNotFound(const std::string &msg) : std::logic_error(msg){};
	};

	class FileNotReadable : public std::logic_error
	{
	public:
		FileNotReadable(const std::string &msg) : std::logic_error(msg){};
	};

	class FileNotWritable : public std::logic_error
	{
	public:
		FileNotWritable(const std::string &msg) : std::logic_error(msg){};
	};

	class FileUnknown : public std::logic_error
	{
	public:
		FileUnknown(const std::string &msg) : std::logic_error(msg){};
	};

	class Conflict : public std::logic_error
	{
	public:
		Conflict(const std::string &msg) : std::logic_error(msg){};
	};

	class FailToLoadDll : public std::runtime_error
	{
	public:
		FailToLoadDll(const std::string &msg) : std::runtime_error(msg){};
	};

	class FailToUnloadDll : public std::runtime_error
	{
	public:
		FailToUnloadDll(const std::string &msg) : std::runtime_error(msg){};
	};

	class FailToGetFunction : public std::runtime_error
	{
	public:
		FailToGetFunction(const std::string &msg) : std::runtime_error(msg){};
	};

	class FailToCreateObject : public std::runtime_error
	{
	public:
		FailToCreateObject(const std::string &msg) : std::runtime_error(msg){};
	};

	class FailToDestroyObject : public std::runtime_error
	{
	public:
		FailToDestroyObject(const std::string &msg) : std::runtime_error(msg){};
	};

	class FailToCallFunction : public std::runtime_error
	{
	public:
		FailToCallFunction(const std::string &msg) : std::runtime_error(msg){};
	};

	class FailToCallMemberFunction : public std::runtime_error
	{
	public:
		FailToCallMemberFunction(const std::string &msg) : std::runtime_error(msg){};
	};

	class FailToCallStaticFunction : public std::runtime_error
	{
	public:
		FailToCallStaticFunction(const std::string &msg) : std::runtime_error(msg){};
	};
}

#endif
