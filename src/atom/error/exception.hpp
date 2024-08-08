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
#include <sstream>
#include <string>
#include <thread>

#include "macro.hpp"
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
        : file_(file), line_(line), func_(func) {
        std::ostringstream oss;
        ((oss << std::forward<Args>(args)), ...);
        message_ = oss.str();
    }

    /**
     * @brief Returns a C-style string describing the exception.
     * @return A pointer to a string describing the exception.
     */
    auto what() const ATOM_NOEXCEPT -> const char * override;

    /**
     * @brief Gets the file where the exception occurred.
     * @return The file where the exception occurred.
     */
    auto getFile() const -> std::string;

    /**
     * @brief Gets the line number where the exception occurred.
     * @return The line number where the exception occurred.
     */
    auto getLine() const -> int;

    /**
     * @brief Gets the function where the exception occurred.
     * @return The function where the exception occurred.
     */
    auto getFunction() const -> std::string;

    /**
     * @brief Gets the message associated with the exception.
     * @return The message associated with the exception.
     */
    auto getMessage() const -> std::string;

    /**
     * @brief Gets the ID of the thread where the exception occurred.
     * @return The ID of the thread where the exception occurred.
     */
    auto getThreadId() const -> std::thread::id;

private:
    std::string file_; /**< The file where the exception occurred. */
    int line_; /**< The line number in the file where the exception occurred. */
    std::string func_;    /**< The function where the exception occurred. */
    std::string message_; /**< The message associated with the exception. */
    mutable std::string
        full_message_; /**< The full message including additional context. */
    std::thread::id thread_id_ =
        std::this_thread::get_id(); /**< The ID of the thread where the
                                       exception occurred. */
    StackTrace stack_trace_;
};

#define THROW_EXCEPTION(...)                                     \
    throw atom::error::Exception(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                 ATOM_FUNC_NAME, __VA_ARGS__)

// Special Exception

// -------------------------------------------------------------------
// Common
// -------------------------------------------------------------------

class RuntimeError : public Exception {
public:
    using Exception::Exception;
};

#define THROW_RUNTIME_ERROR(...)                                    \
    throw atom::error::RuntimeError(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                    ATOM_FUNC_NAME, __VA_ARGS__)

class UnlawfulOperation : public Exception {
public:
    using Exception::Exception;
};

#define THROW_UNLAWFUL_OPERATION(...)                                    \
    throw atom::error::UnlawfulOperation(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                         ATOM_FUNC_NAME, __VA_ARGS__)

class OutOfRange : public Exception {
public:
    using Exception::Exception;
};

#define THROW_OUT_OF_RANGE(...)                                   \
    throw atom::error::OutOfRange(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                  ATOM_FUNC_NAME, __VA_ARGS__);

class OverflowException : public Exception {
public:
    using Exception::Exception;
};

#define THROW_OVERFLOW(...)                                              \
    throw atom::error::OverflowException(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                         ATOM_FUNC_NAME, __VA_ARGS__);

class UnderflowException : public Exception {
public:
    using Exception::Exception;
};

#define THROW_UNDERFLOW(...)                                              \
    throw atom::error::UnderflowException(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                          ATOM_FUNC_NAME, __VA_ARGS__);

class Unkown : public Exception {
public:
    using Exception::Exception;
};

#define THROW_UNKOWN(...)                                                     \
    throw atom::error::Unkown(ATOM_FILE_NAME, ATOM_FILE_LINE, ATOM_FUNC_NAME, \
                              __VA_ARGS__);

// -------------------------------------------------------------------
// Object
// -------------------------------------------------------------------

class ObjectAlreadyExist : public Exception {
public:
    using Exception::Exception;
};

#define THROW_OBJ_ALREADY_EXIST(...)                                      \
    throw atom::error::ObjectAlreadyExist(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                          ATOM_FUNC_NAME, __VA_ARGS__)

class ObjectAlreadyInitialized : public Exception {
public:
    using Exception::Exception;
};

#define THROW_OBJ_ALREADY_INITIALIZED(...)       \
    throw atom::error::ObjectAlreadyInitialized( \
        ATOM_FILE_NAME, ATOM_FILE_LINE, ATOM_FUNC_NAME, __VA_ARGS__)

class ObjectNotExist : public Exception {
public:
    using Exception::Exception;
};

#define THROW_OBJ_NOT_EXIST(...)                                      \
    throw atom::error::ObjectNotExist(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                      ATOM_FUNC_NAME, __VA_ARGS__)

class ObjectUninitialized : public Exception {
public:
    using Exception::Exception;
};

class SystemCollapse : public Exception {
public:
    using Exception::Exception;
};

#define THROW_SYSTEM_COLLAPSE(...)                                    \
    throw atom::error::SystemCollapse(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                      ATOM_FUNC_NAME, __VA_ARGS__)

class NullPointer : public Exception {
public:
    using Exception::Exception;
};

#define THROW_NULL_POINTER(...)                                    \
    throw atom::error::NullPointer(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                   ATOM_FUNC_NAME, __VA_ARGS__)

class NotFound : public Exception {
public:
    using Exception::Exception;
};

#define THROW_NOT_FOUND(...)                                    \
    throw atom::error::NotFound(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                ATOM_FUNC_NAME, __VA_ARGS__)

// -------------------------------------------------------------------
// Argument
// -------------------------------------------------------------------

#define THROW_OBJ_UNINITIALIZED(...)                                       \
    throw atom::error::ObjectUninitialized(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                           ATOM_FUNC_NAME, __VA_ARGS__)

class WrongArgument : public Exception {
public:
    using Exception::Exception;
};

#define THROW_WRONG_ARGUMENT(...)                                    \
    throw atom::error::WrongArgument(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                     ATOM_FUNC_NAME, __VA_ARGS__)

class InvalidArgument : public Exception {
public:
    using Exception::Exception;
};

#define THROW_INVALID_ARGUMENT(...)                                    \
    throw atom::error::InvalidArgument(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                       ATOM_FUNC_NAME, __VA_ARGS__)

class MissingArgument : public Exception {
public:
    using Exception::Exception;
};

#define THROW_MISSING_ARGUMENT(...)                                    \
    throw atom::error::MissingArgument(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                       ATOM_FUNC_NAME, __VA_ARGS__)

// -------------------------------------------------------------------
// File
// -------------------------------------------------------------------

class FileNotFound : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FILE_NOT_FOUND(...)                                   \
    throw atom::error::FileNotFound(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                    ATOM_FUNC_NAME, __VA_ARGS__)

class FileNotReadable : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FILE_NOT_READABLE(...)                                   \
    throw atom::error::FileNotReadable(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                       ATOM_FUNC_NAME, __VA_ARGS__)

class FileNotWritable : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FILE_NOT_WRITABLE(...)                                   \
    throw atom::error::FileNotWritable(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                       ATOM_FUNC_NAME, __VA_ARGS__)

class FailToOpenFile : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_OPEN_FILE(...)                                  \
    throw atom::error::FailToOpenFile(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                      ATOM_FUNC_NAME, __VA_ARGS__)

class FailToCloseFile : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_CLOSE_FILE(...)                                  \
    throw atom::error::FailToCloseFile(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                       ATOM_FUNC_NAME, __VA_ARGS__)

class FailToLoadDll : public Exception {
public:
    using Exception::Exception;
};

// -------------------------------------------------------------------
// Dynamic Library
// -------------------------------------------------------------------

#define THROW_FAIL_TO_LOAD_DLL(...)                                  \
    throw atom::error::FailToLoadDll(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                     ATOM_FUNC_NAME, __VA_ARGS__)

class FailToUnloadDll : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_UNLOAD_DLL(...)                                  \
    throw atom::error::FailToUnloadDll(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                       ATOM_FUNC_NAME, __VA_ARGS__)

class FailToLoadSymbol : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_LOAD_SYMBOL(...)                                  \
    throw atom::error::FailToLoadSymbol(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                        ATOM_FUNC_NAME, __VA_ARGS__)

// -------------------------------------------------------------------
// Proccess Library
// -------------------------------------------------------------------

class FailToCreateProcess : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_CREATE_PROCESS(...)                                  \
    throw atom::error::FailToCreateProcess(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                           ATOM_FUNC_NAME, __VA_ARGS__)

class FailToTerminateProcess : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_TERMINATE_PROCESS(...)                                  \
    throw atom::error::FailToTerminateProcess(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                              ATOM_FUNC_NAME, __VA_ARGS__)

// -------------------------------------------------------------------
// JSON Error
// -------------------------------------------------------------------

class JsonParseError : public Exception {
public:
    using Exception::Exception;
};

#define THROW_JSON_PARSE_ERROR(...)                                   \
    throw atom::error::JsonParseError(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                      ATOM_FUNC_NAME, __VA_ARGS__)

class JsonValueError : public Exception {
public:
    using Exception::Exception;
};

#define THROW_JSON_VALUE_ERROR(...)                                   \
    throw atom::error::JsonValueError(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                      ATOM_FUNC_NAME, __VA_ARGS__)
}  // namespace atom::error

#endif
