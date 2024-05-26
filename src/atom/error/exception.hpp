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

#include "stacktrace.hpp"

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

    std::string file_; /**< The file where the exception occurred. */
    int line_; /**< The line number in the file where the exception occurred. */
    std::string func_;    /**< The function where the exception occurred. */
    std::string message_; /**< The message associated with the exception. */
    mutable std::string
        full_message_; /**< The full message including additional context. */
    std::thread::id
        thread_id_; /**< The ID of the thread where the exception occurred. */
    StackTrace stack_trace_;
};

#define THROW_EXCEPTION(...) \
    throw atom::error::Exception(__FILE__, __LINE__, __func__, __VA_ARGS__)

// Special Exception

// -------------------------------------------------------------------
// Common
// -------------------------------------------------------------------

class RuntimeError : public Exception {
public:
    using Exception::Exception;
};

#define THROW_RUNTIME_ERROR(...) \
    throw atom::error::RuntimeError(__FILE__, __LINE__, __func__, __VA_ARGS__)

class UnlawfulOperation : public Exception {
public:
    using Exception::Exception;
};

#define THROW_UNLAWFUL_OPERATION(...)                                  \
    throw atom::error::UnlawfulOperation(__FILE__, __LINE__, __func__, \
                                         __VA_ARGS__)

class Unkown : public Exception {
public:
    using Exception::Exception;
};

// -------------------------------------------------------------------
// Object
// -------------------------------------------------------------------

class ObjectAlreadyExist : public Exception {
public:
    using Exception::Exception;
};

#define THROW_OBJ_ALREADY_EXIST(...)                                    \
    throw atom::error::ObjectAlreadyExist(__FILE__, __LINE__, __func__, \
                                          __VA_ARGS__)

class ObjectAlreadyInitialized : public Exception {
public:
    using Exception::Exception;
};

#define THROW_OBJ_ALREADY_INITIALIZED(...)                                    \
    throw atom::error::ObjectAlreadyInitialized(__FILE__, __LINE__, __func__, \
                                                __VA_ARGS__)

class ObjectNotExist : public Exception {
public:
    using Exception::Exception;
};

#define THROW_OBJ_NOT_EXIST(...) \
    throw atom::error::ObjectNotExist(__FILE__, __LINE__, __func__, __VA_ARGS__)

class ObjectUninitialized : public Exception {
public:
    using Exception::Exception;
};

#define THROW_UNKOWN(...) \
    throw atom::error::Unkown(__FILE__, __LINE__, __func__, __VA_ARGS__)

class SystemCollapse : public Exception {
public:
    using Exception::Exception;
};

#define THROW_SYSTEM_COLLAPSE(...) \
    throw atom::error::SystemCollapse(__FILE__, __LINE__, __func__, __VA_ARGS__)

class NullPointer : public Exception {
public:
    using Exception::Exception;
};

#define THROW_NULL_POINTER(...) \
    throw atom::error::NullPointer(__FILE__, __LINE__, __func__, __VA_ARGS__)

class NotFound : public Exception {
public:
    using Exception::Exception;
};

#define THROW_NOT_FOUND(...) \
    throw atom::error::NotFound(__FILE__, __LINE__, __func__, __VA_ARGS__)

// -------------------------------------------------------------------
// Argument
// -------------------------------------------------------------------

#define THROW_OBJ_UNINITIALIZED(...)                                     \
    throw atom::error::ObjectUninitialized(__FILE__, __LINE__, __func__, \
                                           __VA_ARGS__)

class WrongArgument : public Exception {
public:
    using Exception::Exception;
};

#define THROW_WRONG_ARGUMENT(...) \
    throw atom::error::WrongArgument(__FILE__, __LINE__, __func__, __VA_ARGS__)

class InvalidArgument : public Exception {
public:
    using Exception::Exception;
};

#define THROW_INVALID_ARGUMENT(...)                                  \
    throw atom::error::InvalidArgument(__FILE__, __LINE__, __func__, \
                                       __VA_ARGS__)

class MissingArgument : public Exception {
public:
    using Exception::Exception;
};

#define THROW_MISSING_ARGUMENT(...)                                  \
    throw atom::error::MissingArgument(__FILE__, __LINE__, __func__, \
                                       __VA_ARGS__)

// -------------------------------------------------------------------
// File
// -------------------------------------------------------------------

class FileNotFound : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FILE_NOT_FOUND(...) \
    throw atom::error::FileNotFound(__FILE__, __LINE__, __func__, __VA_ARGS__)

class FileNotReadable : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FILE_NOT_READABLE(...)                                 \
    throw atom::error::FileNotReadable(__FILE__, __LINE__, __func__, \
                                       __VA_ARGS__)

class FileNotWritable : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FILE_NOT_WRITABLE(...)                                 \
    throw atom::error::FileNotWritable(__FILE__, __LINE__, __func__, \
                                       __VA_ARGS__)

class FailToOpenFile : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_OPEN_FILE(...) \
    throw atom::error::FailToOpenFile(__FILE__, __LINE__, __func__, __VA_ARGS__)

class FailToCloseFile : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_CLOSE_FILE(...)                                \
    throw atom::error::FailToCloseFile(__FILE__, __LINE__, __func__, \
                                       __VA_ARGS__)

class FailToLoadDll : public Exception {
public:
    using Exception::Exception;
};

// -------------------------------------------------------------------
// Dynamic Library
// -------------------------------------------------------------------

#define THROW_FAIL_TO_LOAD_DLL(...) \
    throw atom::error::FailToLoadDll(__FILE__, __LINE__, __func__, __VA_ARGS__)

class FailToUnloadDll : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_UNLOAD_DLL(...)                                \
    throw atom::error::FailToUnloadDll(__FILE__, __LINE__, __func__, \
                                       __VA_ARGS__)

class FailToLoadSymbol : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_LOAD_SYMBOL(...)                                \
    throw atom::error::FailToLoadSymbol(__FILE__, __LINE__, __func__, \
                                        __VA_ARGS__)
}  // namespace atom::error

#endif
