# Command Execution Utilities

## ProcessHandle Structure

```cpp
#ifdef _WIN32
struct ProcessHandle {
    HANDLE handle;
};
#else
struct ProcessHandle {
    pid_t pid;
};
#endif
```

## executeCommand

Executes a command and returns the command output as a string.

```cpp
[[nodiscard]] std::string executeCommand(
    const std::string &command, bool openTerminal,
    std::function<void(const std::string &)> processLine);
```

```cpp
std::string output = executeCommand("ls -l", false, [](const std::string &line) {
    std::cout << "Output line: " << line << std::endl;
});
```

```shell
// Output of "ls -l" command
Output line: <file details>
Output line: <file details>
...
```

## executeCommands

Executes a list of commands.

```cpp
void executeCommands(const std::vector<std::string> &commands);
```

```cpp
executeCommands({"ls -l", "pwd"});
```

## executeCommand (overloaded)

Executes a command and returns the process handle.

```cpp
[[nodiscard]] ProcessHandle executeCommand(const std::string &command);
```

```cpp
ProcessHandle handle = executeCommand("echo 'Hello, World!'");
```

## killProcess

Kills a process using the process handle.

```cpp
void killProcess(const ProcessHandle &handle);
```

### Note

- Ensure the `ProcessHandle` is valid and corresponds to a running process.

## executeCommandWithEnv

Executes a command with environment variables and returns the output as a string.

```cpp
[[nodiscard]] std::string executeCommandWithEnv(
    const std::string &command,
    const std::map<std::string, std::string> &envVars);
```

```cpp
std::string output = executeCommandWithEnv("echo $MY_VAR", {{"MY_VAR", "123"}});
```

## executeCommandWithStatus

Executes a command and returns the output along with the exit status.

```cpp
[[nodiscard]] std::pair<std::string, int> executeCommandWithStatus(
    const std::string &command);
```

```cpp
auto result = executeCommandWithStatus("ls -l");
std::cout << "Output: " << result.first << std::endl;
std::cout << "Exit Status: " << result.second << std::endl;
```

```shell
// Output of "ls -l" command
Output: <file details>
Exit Status: 0
```
