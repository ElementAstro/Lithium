#ifndef EXCEPTION_H
#define EXCEPTION_H
#include <stdexcept>

#define LithiumCore_Exception_VERSION "1.4.0"
#define LithiumCore_Exception_EDIT_TIME "2023/8/17"
#define LithiumCore_Exception_AUTHOR "Max Qian"

namespace LithiumCore
{
	namespace Utilities
	{
		class Uninitialization_Error;
		class WrongArgument_Error;
		class UnlawfulOperation_Error;
		class Unkown_ErrorUnkown_Error;
		class SystemCollapse_Error;
		class NullPointer_Error;
		class NotFound_Error;
		class Conflict_Error;
		class FailToLoadDll_Error;
		class FailToUnloadDll_Error;
		class FailToGetFunction_Error;
	}
}

// 类的声明，和部分定义
namespace LithiumCore::Utilities
{
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
}

#endif
