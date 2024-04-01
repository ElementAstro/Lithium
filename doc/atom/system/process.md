# ProcessManager Class Documentation

## Description

The `ProcessManager` class provides functionality to manage processes, including creating, terminating, and monitoring processes, as well as running scripts and retrieving process information.

### Public Members

- `createProcess`: Create a new process to execute a command.
- `terminateProcess`: Terminate a process by PID or name.
- `hasProcess`: Check if a specific process exists.
- `getRunningProcesses`: Get a list of running processes with detailed information.
- `getProcessOutput`: Get the output of a specific process.
- `waitForCompletion`: Wait for all processes to complete and clear the process list.
- `runScript`: Run a script as a process.

## Constructor

```cpp
ProcessManager();
```

## Constructor with Max Processes

```cpp
ProcessManager(int maxProcess);
```

## createShared Function

```cpp
static std::shared_ptr<ProcessManager> createShared();
static std::shared_ptr<ProcessManager> createShared(int maxProcess);
```

```cpp
// Creating a shared ProcessManager instance
auto processManager = ProcessManager::createShared(10);
```

## createProcess Function

```cpp
bool createProcess(const std::string &command, const std::string &identifier);
```

```cpp
// Creating a new process
bool created = processManager->createProcess("ls -l", "process1");
// Expected Output: true if the process creation was successful, false otherwise.
```

## terminateProcess Function

```cpp
bool terminateProcess(pid_t pid, int signal = SIGTERM);
bool terminateProcessByName(const std::string &name, int signal = SIGTERM);
```

```cpp
// Terminating a process by PID
bool terminated = processManager->terminateProcess(12345, SIGKILL);
// Expected Output: true if the process was successfully terminated, false otherwise.
```

## hasProcess Function

```cpp
bool hasProcess(const std::string &identifier);
```

```cpp
// Checking if a process exists
bool exists = processManager->hasProcess("process1");
// Expected Output: true if the process exists, false otherwise.
```

## getRunningProcesses Function

```cpp
std::vector<Process> getRunningProcesses();
```

```cpp
// Getting a list of running processes
std::vector<Process> runningProcesses = processManager->getRunningProcesses();
// Expected Output: List of running processes with detailed information.
```

## getProcessOutput Function

```cpp
std::vector<std::string> getProcessOutput(const std::string &identifier);
```

```cpp
// Getting output of a specific process
std::vector<std::string> output = processManager->getProcessOutput("process1");
// Expected Output: Output information of the specific process.
```

## waitForCompletion Function

```cpp
void waitForCompletion();
```

```cpp
// Waiting for all processes to complete
processManager->waitForCompletion();
// Note: This will wait for all running processes to finish before clearing the process list.
```

## runScript Function

```cpp
bool runScript(const std::string &script, const std::string &identifier);
```

```cpp
// Running a script as a process
bool scriptRan = processManager->runScript("/path/to/script.sh", "script_process");
// Expected Output: true if the script was executed as a process, false otherwise.
```

### Private Members

- `m_maxProcesses`: Maximum number of processes allowed.
- `cv`: Condition variable used for waiting for process completion.
- `processes`: List of currently running processes with detailed information.
- `mtx`: Mutex used for manipulating the process list.

## GetAllProcesses Function

```cpp
std::vector<std::pair<int, std::string>> GetAllProcesses();
```

````cpp
// Getting information of all processes
std::vector<std::pair<int, std::string>> allProcesses = GetAllProcesses();
// Expected Output: List of pairs containing PID and process names of all processes.

## GetSelfProcessInfo Function
```cpp
Process GetSelfProcessInfo();
````

```cpp
// Getting information of the current process
Process selfProcessInfo = GetSelfProcessInfo();
// Expected Output: Information about the current process.
```
