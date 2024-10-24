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

    template <typename... Args>
    static void rethrowNested(Args &&...args) {
        try {
            throw;  // 捕获当前异常
        } catch (...) {
            std::throw_with_nested(Exception(std::forward<Args>(args)...));
        }
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

// System error exception class
class SystemErrorException : public Exception {
public:
    SystemErrorException(const char *file, int line, const char *func,
                         int err_code, std::string msg)
        : Exception(file, line, func, msg),
          error_code_(err_code),
          error_message_(
              std::error_code(err_code, std::generic_category()).message()) {}

    const char *what() const noexcept override {
        if (what_message_.empty()) {
            what_message_ = "System error [" + std::to_string(error_code_) +
                            "]: " + error_message_ + "\n" + Exception::what();
        }
        return what_message_.c_str();
    }

private:
    int error_code_;
    std::string error_message_;
    mutable std::string what_message_;
};

// Nested exception handling
class NestedException : public Exception {
public:
    explicit NestedException(const char *file, int line, const char *func,
                             std::exception_ptr ptr)
        : Exception(file, line, func), exception_ptr_(std::move(ptr)) {}

    const char *what() const noexcept override {
        if (what_message_.empty()) {
            try {
                std::rethrow_exception(exception_ptr_);
            } catch (const std::exception &e) {
                what_message_ = "Nested exception: " + std::string(e.what());
            } catch (...) {
                what_message_ = "Nested unknown exception";
            }
        }
        return what_message_.c_str();
    }

private:
    std::exception_ptr exception_ptr_;
    mutable std::string what_message_;
};

#define THROW_EXCEPTION(...)                                     \
    throw atom::error::Exception(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                 ATOM_FUNC_NAME, __VA_ARGS__)

#define THROW_NESTED_EXCEPTION(...)                                       \
    atom::error::Exception::rethrowNested(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                          ATOM_FUNC_NAME, __VA_ARGS__)

#define THROW_SYSTEM_ERROR(error_code, ...)                                 \
    static_assert(std::is_integral<decltype(error_code)>::value,            \
                  "Error code must be an integral type");                   \
    static_assert(error_code != 0, "Error code must be non-zero");          \
    throw atom::error::SystemErrorException(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                            ATOM_FUNC_NAME, error_code,     \
                                            __VA_ARGS__)

// -------------------------------------------------------------------
// Common
// -------------------------------------------------------------------

class RuntimeError : public Exception {
public:
    using Exception::Exception;
};

namespace internal {
template <typename... Args>
struct are_all_printable;

// Base case: Empty parameter pack is printable
template <>
struct are_all_printable<> {
    static constexpr bool value = true;
};

// Recursive case: Check if the first argument is printable and recursively
// check the rest
template <typename First, typename... Rest>
struct are_all_printable<First, Rest...> {
    // Check if std::ostream can output the type
    static constexpr bool value =
        std::is_convertible<decltype(std::declval<std::ostream &>()
                                     << std::declval<First>()),
                            std::ostream &>::value &&
        are_all_printable<Rest...>::value;
};
}  // namespace internal

#define THROW_RUNTIME_ERROR(...)                                      \
    throw atom::error::RuntimeError(ATOM_FILE_NAME, ATOM_FILE_LINE,   \
                                    ATOM_FUNC_NAME, __VA_ARGS__)

#define THROW_NESTED_RUNTIME_ERROR(...)                                      \
    atom::error::RuntimeError::rethrowNested(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                             ATOM_FUNC_NAME, __VA_ARGS__)

class LogicError : public Exception {
public:
    using Exception::Exception;
};

#define THROW_LOGIC_ERROR(...)                                    \
    throw atom::error::LogicError(ATOM_FILE_NAME, ATOM_FILE_LINE, \
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

class FailToCreateFile : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_CREATE_FILE(...)                                  \
    throw atom::error::FailToCreateFile(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                        ATOM_FUNC_NAME, __VA_ARGS__)

class FailToDeleteFile : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_DELETE_FILE(...)                                  \
    throw atom::error::FailToDeleteFile(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                        ATOM_FUNC_NAME, __VA_ARGS__)

class FailToCopyFile : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_COPY_FILE(...)                                  \
    throw atom::error::FailToCopyFile(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                      ATOM_FUNC_NAME, __VA_ARGS__)

class FailToMoveFile : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_MOVE_FILE(...)                                  \
    throw atom::error::FailToMoveFile(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                      ATOM_FUNC_NAME, __VA_ARGS__)

class FailToReadFile : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_READ_FILE(...)                                  \
    throw atom::error::FailToReadFile(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                      ATOM_FUNC_NAME, __VA_ARGS__)

class FailToWriteFile : public Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_WRITE_FILE(...)                                  \
    throw atom::error::FailToWriteFile(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                       ATOM_FUNC_NAME, __VA_ARGS__)

// -------------------------------------------------------------------
// Dynamic Library
// -------------------------------------------------------------------

class FailToLoadDll : public Exception {
public:
    using Exception::Exception;
};

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

// -------------------------------------------------------------------
// Network Error
// -------------------------------------------------------------------

class CurlInitializationError : public Exception {
public:
    using Exception::Exception;
};

#define THROW_CURL_INITIALIZATION_ERROR(...)                                   \
    throw atom::error::CurlInitializationError(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                               ATOM_FUNC_NAME, __VA_ARGS__)

class CurlRuntimeError : public Exception {
public:
    using Exception::Exception;
};

#define THROW_CURL_RUNTIME_ERROR(...)                                   \
    throw atom::error::CurlRuntimeError(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                        ATOM_FUNC_NAME, __VA_ARGS__)
}  // namespace atom::error

#endif
