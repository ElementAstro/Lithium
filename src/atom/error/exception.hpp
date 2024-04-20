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

#include <exception>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

namespace Atom::Error {

class Exception : public std::exception {
public:
    template <typename... Args>
    Exception(const char *file, int line, const char *func, Args &&...args)
        : file_(file),
          line_(line),
          func_(func),
          thread_id_(std::this_thread::get_id()) {
        std::ostringstream oss;
        ((oss << std::forward<Args>(args)), ...);
        message_ = oss.str();
    }

    const char *what() const noexcept override;

    const std::string &getFile() const;
    int getLine() const;
    const std::string &getFunction() const;
    const std::string &getMessage() const;
    std::thread::id getThreadId() const;

private:
    std::string getCurrentTime() const;

    std::string getStackTrace() const;

    std::string file_;
    int line_;
    std::string func_;
    std::string message_;
    mutable std::string full_message_;
    std::thread::id thread_id_;
};

#define THROW_EXCEPTION(...) \
    throw Exception(__FILE__, __LINE__, __func__, __VA_ARGS__)

class ObjectAlreadyExist : public std::logic_error {
public:
    explicit ObjectAlreadyExist(const std::string &msg)
        : std::logic_error(msg){};
};

class ObjectNotExist : public std::logic_error {
public:
    explicit ObjectNotExist(const std::string &msg) : std::logic_error(msg){};
};

class ObjectUninitialized : public std::logic_error {
public:
    explicit ObjectUninitialized(const std::string &msg)
        : std::logic_error(msg){};
};

class Uninitialization : public std::logic_error {
public:
    explicit Uninitialization(const std::string &msg) : std::logic_error(msg){};
};

class WrongArgument : public std::logic_error {
public:
    explicit WrongArgument(const std::string &msg) : std::logic_error(msg){};
};

class InvalidArgument : public std::logic_error {
public:
    explicit InvalidArgument(const std::string &msg) : std::logic_error(msg){};
};

class MissingArgument : public std::logic_error {
public:
    explicit MissingArgument(const std::string &msg) : std::logic_error(msg){};
};

class UnlawfulOperation : public std::logic_error {
public:
    explicit UnlawfulOperation(const std::string &msg)
        : std::logic_error(msg){};
};

class Unkown : public std::logic_error {
public:
    explicit Unkown(const std::string &msg) : std::logic_error(msg){};
};

class SystemCollapse : public std::runtime_error {
public:
    explicit SystemCollapse(const std::string &msg) : std::runtime_error(msg){};
};

class NullPointer : public std::logic_error {
public:
    explicit NullPointer(const std::string &msg) : std::logic_error(msg){};
};

class NotFound : public std::logic_error {
public:
    explicit NotFound(const std::string &msg) : std::logic_error(msg){};
};

class FileNotFound : public std::logic_error {
public:
    explicit FileNotFound(const std::string &msg) : std::logic_error(msg){};
};

class FileNotReadable : public std::logic_error {
public:
    explicit FileNotReadable(const std::string &msg) : std::logic_error(msg){};
};

class FileNotWritable : public std::logic_error {
public:
    explicit FileNotWritable(const std::string &msg) : std::logic_error(msg){};
};

class FileUnknown : public std::logic_error {
public:
    explicit FileUnknown(const std::string &msg) : std::logic_error(msg){};
};

class Conflict : public std::logic_error {
public:
    explicit Conflict(const std::string &msg) : std::logic_error(msg){};
};

class FailToLoadDll : public std::runtime_error {
public:
    explicit FailToLoadDll(const std::string &msg) : std::runtime_error(msg){};
};

class FailToUnloadDll : public std::runtime_error {
public:
    explicit FailToUnloadDll(const std::string &msg)
        : std::runtime_error(msg){};
};

class FailToGetFunction : public std::runtime_error {
public:
    explicit FailToGetFunction(const std::string &msg)
        : std::runtime_error(msg){};
};

class FailToCreateObject : public std::runtime_error {
public:
    explicit FailToCreateObject(const std::string &msg)
        : std::runtime_error(msg){};
};

class FailToDestroyObject : public std::runtime_error {
public:
    explicit FailToDestroyObject(const std::string &msg)
        : std::runtime_error(msg){};
};

class FailToCallFunction : public std::runtime_error {
public:
    explicit FailToCallFunction(const std::string &msg)
        : std::runtime_error(msg){};
};

class FailToCallMemberFunction : public std::runtime_error {
public:
    explicit FailToCallMemberFunction(const std::string &msg)
        : std::runtime_error(msg){};
};

class FailToCallStaticFunction : public std::runtime_error {
public:
    explicit FailToCallStaticFunction(const std::string &msg)
        : std::runtime_error(msg){};
};
}  // namespace Atom::Error

#endif
