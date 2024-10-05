# Atom System Command Library Documentation

This document provides a detailed explanation of the functions available in the `atom::system` namespace for executing system commands and managing processes.

## Table of Contents

1. [executeCommand](#executecommand)
2. [executeCommandWithInput](#executecommandwithinput)
3. [executeCommandStream](#executecommandstream)
4. [executeCommands](#executecommands)
5. [killProcessByName](#killprocessbyname)
6. [killProcessByPID](#killprocessbypid)
7. [executeCommandWithEnv](#executecommandwithenv)
8. [executeCommandWithStatus](#executecommandwithstatus)
9. [executeCommandSimple](#executecommandsimple)

## executeCommand

```cpp
ATOM_NODISCARD auto executeCommand(
    const std::string &command,
    bool openTerminal = false,
    const std::function<void(const std::string &)> &processLine = [](const std::string &) {}
) -> std::string;
```

This function executes a command and returns the command output as a string.

### Parameters

- `command`: The command to execute (string).
- `openTerminal`: Whether to open a terminal window for the command (boolean, default: false).
- `processLine`: A callback function to process each line of output (optional).

### Return Value

Returns the output of the command as a string.

### Exceptions

Throws a `std::runtime_error` if the command fails to execute.

### Example Usage

```cpp
try {
    std::string output = atom::system::executeCommand("ls -l", false, [](const std::string& line) {
        std::cout << "Processing: " << line << std::endl;
    });
    std::cout << "Command output: " << output << std::endl;
} catch (const std::runtime_error& e) {
    std::cerr << "Error executing command: " << e.what() << std::endl;
}
```

## executeCommandWithInput

```cpp
ATOM_NODISCARD auto executeCommandWithInput(
    const std::string &command,
    const std::string &input,
    const std::function<void(const std::string &)> &processLine = nullptr
) -> std::string;
```

This function executes a command with input and returns the command output as a string.

### Parameters

- `command`: The command to execute (string).
- `input`: The input to provide to the command (string).
- `processLine`: A callback function to process each line of output (optional).

### Return Value

Returns the output of the command as a string.

### Exceptions

Throws a `std::runtime_error` if the command fails to execute.

### Example Usage

```cpp
try {
    std::string input = "Hello, World!";
    std::string output = atom::system::executeCommandWithInput("cat", input);
    std::cout << "Command output: " << output << std::endl;
} catch (const std::runtime_error& e) {
    std::cerr << "Error executing command: " << e.what() << std::endl;
}
```

## executeCommandStream

```cpp
auto executeCommandStream(
    const std::string &command,
    bool openTerminal,
    const std::function<void(const std::string &)> &processLine,
    int &status,
    const std::function<bool()> &terminateCondition = [] { return false; }
) -> std::string;
```

This function executes a command and returns the command output as a string, with additional control over the execution process.

### Parameters

- `command`: The command to execute (string).
- `openTerminal`: Whether to open a terminal window for the command (boolean).
- `processLine`: A callback function to process each line of output.
- `status`: Reference to an integer to store the exit status of the command.
- `terminateCondition`: A callback function to determine whether to terminate the command execution (optional).

### Return Value

Returns the output of the command as a string.

### Exceptions

Throws a `std::runtime_error` if the command fails to execute.

### Example Usage

```cpp
try {
    int status;
    std::string output = atom::system::executeCommandStream(
        "ping -c 5 google.com",
        false,
        [](const std::string& line) { std::cout << line << std::endl; },
        status,
        []() { return false; }  // Never terminate early
    );
    std::cout << "Command exit status: " << status << std::endl;
    std::cout << "Full output: " << output << std::endl;
} catch (const std::runtime_error& e) {
    std::cerr << "Error executing command: " << e.what() << std::endl;
}
```

## executeCommands

```cpp
void executeCommands(const std::vector<std::string> &commands);
```

This function executes a list of commands.

### Parameters

- `commands`: The list of commands to execute (vector of strings).

### Exceptions

Throws a `std::runtime_error` if any of the commands fail to execute.

### Example Usage

```cpp
try {
    std::vector<std::string> commands = {
        "echo 'Hello'",
        "ls -l",
        "pwd"
    };
    atom::system::executeCommands(commands);
} catch (const std::runtime_error& e) {
    std::cerr << "Error executing commands: " << e.what() << std::endl;
}
```

## killProcessByName

```cpp
void killProcessByName(const std::string &processName, int signal);
```

This function kills a process by its name.

### Parameters

- `processName`: The name of the process to kill (string).
- `signal`: The signal to send to the process (integer).

### Example Usage

```cpp
atom::system::killProcessByName("firefox", SIGTERM);
```

## killProcessByPID

```cpp
void killProcessByPID(int pid, int signal);
```

This function kills a process by its PID.

### Parameters

- `pid`: The PID of the process to kill (integer).
- `signal`: The signal to send to the process (integer).

### Example Usage

```cpp
atom::system::killProcessByPID(1234, SIGKILL);
```

## executeCommandWithEnv

```cpp
ATOM_NODISCARD auto executeCommandWithEnv(
    const std::string &command,
    const std::unordered_map<std::string, std::string> &envVars
) -> std::string;
```

This function executes a command with environment variables and returns the command output as a string.

### Parameters

- `command`: The command to execute (string).
- `envVars`: The environment variables as a map of variable name to value.

### Return Value

Returns the output of the command as a string.

### Exceptions

Throws a `std::runtime_error` if the command fails to execute.

### Example Usage

```cpp
try {
    std::unordered_map<std::string, std::string> env = {
        {"MY_VAR", "Hello"},
        {"ANOTHER_VAR", "World"}
    };
    std::string output = atom::system::executeCommandWithEnv("echo $MY_VAR $ANOTHER_VAR", env);
    std::cout << "Command output: " << output << std::endl;
} catch (const std::runtime_error& e) {
    std::cerr << "Error executing command: " << e.what() << std::endl;
}
```

## executeCommandWithStatus

```cpp
ATOM_NODISCARD auto executeCommandWithStatus(const std::string &command)
    -> std::pair<std::string, int>;
```

This function executes a command and returns the command output along with the exit status.

### Parameters

- `command`: The command to execute (string).

### Return Value

Returns a pair containing:

- First: The output of the command as a string.
- Second: The exit status as an integer.

### Exceptions

Throws a `std::runtime_error` if the command fails to execute.

### Example Usage

```cpp
try {
    auto [output, status] = atom::system::executeCommandWithStatus("ls -l /nonexistent");
    std::cout << "Command output: " << output << std::endl;
    std::cout << "Exit status: " << status << std::endl;
} catch (const std::runtime_error& e) {
    std::cerr << "Error executing command: " << e.what() << std::endl;
}
```

## executeCommandSimple

```cpp
ATOM_NODISCARD auto executeCommandSimple(const std::string &command) -> bool;
```

This function executes a command and returns a boolean indicating whether the command was successful.

### Parameters

- `command`: The command to execute (string).

### Return Value

Returns a boolean:

- `true` if the command was executed successfully (exit status 0).
- `false` if the command failed (non-zero exit status).

### Exceptions

Throws a `std::runtime_error` if the command fails to execute.

### Example Usage

```cpp
try {
    bool success = atom::system::executeCommandSimple("test -f /etc/hosts");
    if (success) {
        std::cout << "The file /etc/hosts exists." << std::endl;
    } else {
        std::cout << "The file /etc/hosts does not exist." << std::endl;
    }
} catch (const std::runtime_error& e) {
    std::cerr << "Error executing command: " << e.what() << std::endl;
}
```

## Best Practices and Tips

1. **Error Handling**: Always wrap command execution in try-catch blocks to handle potential runtime errors.

2. **Security Considerations**: Be cautious when executing commands with user-provided input. Always sanitize and validate input to prevent command injection attacks.

3. **Resource Management**: For long-running commands or when executing multiple commands in succession, consider using `executeCommandStream` to process output in real-time and potentially terminate the execution if needed.

4. **Environment Variables**: Use `executeCommandWithEnv` when you need to set specific environment variables for a command without affecting the global environment.

5. **Parsing Output**: When executing commands that produce structured output, consider using the `processLine` callback to parse the output line-by-line instead of processing the entire output at once.

6. **Exit Status**: Use `executeCommandWithStatus` or `executeCommandSimple` when you need to know the exit status of the command in addition to its output.

7. **Input Handling**: For commands that require input, use `executeCommandWithInput` to provide the input data programmatically.

8. **Process Management**: Use `killProcessByName` and `killProcessByPID` judiciously, ensuring you have the necessary permissions and that terminating the process won't lead to data corruption or system instability.
