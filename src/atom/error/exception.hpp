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

namespace atom::error {

/**
 * @brief Custom exception class with detailed information about the error.
 */
class Exception : public std::exception {
public:
    /**
     * @brief Constructs an Exception object.
     * @param file The file where the exception occurred.
     * @param line The line number in the file where the exception occurred.
     * @param func The function where the exception occurred.
     * @param args Additional arguments to provide context for the exception.
     */
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

    /**
     * @brief Returns a C-style string describing the exception.
     * @return A pointer to a string describing the exception.
     */
    const char *what() const noexcept override;

    /**
     * @brief Gets the file where the exception occurred.
     * @return The file where the exception occurred.
     */
    const std::string &getFile() const;

    /**
     * @brief Gets the line number where the exception occurred.
     * @return The line number where the exception occurred.
     */
    int getLine() const;

    /**
     * @brief Gets the function where the exception occurred.
     * @return The function where the exception occurred.
     */
    const std::string &getFunction() const;

    /**
     * @brief Gets the message associated with the exception.
     * @return The message associated with the exception.
     */
    const std::string &getMessage() const;

    /**
     * @brief Gets the ID of the thread where the exception occurred.
     * @return The ID of the thread where the exception occurred.
     */
    std::thread::id getThreadId() const;

private:
    /**
     * @brief Gets the current time as a formatted string.
     * @return The current time as a formatted string.
     */
    std::string getCurrentTime() const;

    /**
     * @brief Gets the stack trace when the exception occurred.
     * @return The stack trace when the exception occurred.
     */
    std::string getStackTrace() const;

    std::string file_; /**< The file where the exception occurred. */
    int line_; /**< The line number in the file where the exception occurred. */
    std::string func_;    /**< The function where the exception occurred. */
    std::string message_; /**< The message associated with the exception. */
    mutable std::string
        full_message_; /**< The full message including additional context. */
    std::thread::id
        thread_id_; /**< The ID of the thread where the exception occurred. */
};

#define THROW_EXCEPTION(...) \
    throw atom::error::Exception(__FILE__, __LINE__, __func__, __VA_ARGS__)

class ObjectAlreadyExist : public std::logic_error {
public:
    explicit ObjectAlreadyExist(const std::string &msg)
        : std::logic_error(msg) {};
};

class ObjectNotExist : public std::logic_error {
public:
    explicit ObjectNotExist(const std::string &msg) : std::logic_error(msg) {};
};

class ObjectUninitialized : public std::logic_error {
public:
    explicit ObjectUninitialized(const std::string &msg)
        : std::logic_error(msg) {};
};

class Uninitialization : public std::logic_error {
public:
    explicit Uninitialization(const std::string &msg)
        : std::logic_error(msg) {};
};

class WrongArgument : public std::logic_error {
public:
    explicit WrongArgument(const std::string &msg) : std::logic_error(msg) {};
};

class InvalidArgument : public std::logic_error {
public:
    explicit InvalidArgument(const std::string &msg) : std::logic_error(msg) {};
};

class MissingArgument : public std::logic_error {
public:
    explicit MissingArgument(const std::string &msg) : std::logic_error(msg) {};
};

class UnlawfulOperation : public std::logic_error {
public:
    explicit UnlawfulOperation(const std::string &msg)
        : std::logic_error(msg) {};
};

class Unkown : public std::logic_error {
public:
    explicit Unkown(const std::string &msg) : std::logic_error(msg) {};
};

class SystemCollapse : public std::runtime_error {
public:
    explicit SystemCollapse(const std::string &msg)
        : std::runtime_error(msg) {};
};

class NullPointer : public std::logic_error {
public:
    explicit NullPointer(const std::string &msg) : std::logic_error(msg) {};
};

class NotFound : public std::logic_error {
public:
    explicit NotFound(const std::string &msg) : std::logic_error(msg) {};
};

class FileNotFound : public std::logic_error {
public:
    explicit FileNotFound(const std::string &msg) : std::logic_error(msg) {};
};

class FileNotReadable : public std::logic_error {
public:
    explicit FileNotReadable(const std::string &msg) : std::logic_error(msg) {};
};

class FileNotWritable : public std::logic_error {
public:
    explicit FileNotWritable(const std::string &msg) : std::logic_error(msg) {};
};

class FileUnknown : public std::logic_error {
public:
    explicit FileUnknown(const std::string &msg) : std::logic_error(msg) {};
};

class Conflict : public std::logic_error {
public:
    explicit Conflict(const std::string &msg) : std::logic_error(msg) {};
};

class FailToLoadDll : public std::runtime_error {
public:
    explicit FailToLoadDll(const std::string &msg) : std::runtime_error(msg) {};
};

class FailToUnloadDll : public std::runtime_error {
public:
    explicit FailToUnloadDll(const std::string &msg)
        : std::runtime_error(msg) {};
};

class FailToGetFunction : public std::runtime_error {
public:
    explicit FailToGetFunction(const std::string &msg)
        : std::runtime_error(msg) {};
};

class FailToCreateObject : public std::runtime_error {
public:
    explicit FailToCreateObject(const std::string &msg)
        : std::runtime_error(msg) {};
};

class FailToDestroyObject : public std::runtime_error {
public:
    explicit FailToDestroyObject(const std::string &msg)
        : std::runtime_error(msg) {};
};

class FailToCallFunction : public std::runtime_error {
public:
    explicit FailToCallFunction(const std::string &msg)
        : std::runtime_error(msg) {};
};

class FailToCallMemberFunction : public std::runtime_error {
public:
    explicit FailToCallMemberFunction(const std::string &msg)
        : std::runtime_error(msg) {};
};

class FailToCallStaticFunction : public std::runtime_error {
public:
    explicit FailToCallStaticFunction(const std::string &msg)
        : std::runtime_error(msg) {};
};
}  // namespace atom::error

#endif
