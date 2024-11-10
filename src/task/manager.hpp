/**
 * @file task_interpreter.hpp
 * @brief Task Interpreter for managing and executing scripts.
 *
 * This file defines the `TaskInterpreter` class, which is responsible for
 * loading, managing, and executing tasks represented as JSON scripts. The
 * `TaskInterpreter` class provides functionality to register functions and
 * exception handlers, set and retrieve variables, and control script execution
 * flow (e.g., pause, resume, stop). It supports various script operations such
 * as parsing labels, executing steps, handling exceptions, and evaluating
 * expressions.
 *
 * The class also supports asynchronous operations and event handling, making it
 * suitable for dynamic and complex scripting environments.
 *
 * @date 2023-04-03
 * @author Max Qian <lightapt.com>
 * @copyright Copyright (C) 2023-2024 Max Qian
 */

#ifndef LITHIUM_TASK_INTERPRETER_HPP
#define LITHIUM_TASK_INTERPRETER_HPP

#include <memory>
#include <string>

namespace lithium {
struct Program;
class Interpreter {
public:
    Interpreter();

    void loadScript(const std::string& filename);

    void interpretScript(const std::string& filename);

    void interpret(const std::shared_ptr<Program>& ast);
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
}  // namespace lithium

#endif
