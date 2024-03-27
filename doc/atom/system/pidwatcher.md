# PidWatcher Class Documentation

## Description

The `PidWatcher` class provides functionality to monitor and manage processes based on their names.

### Public Members

- `SetExitCallback`: Set a callback function to be executed when the process exits.
- `SetMonitorFunction`: Set a monitor function to run at a specified interval.
- `GetPidByName`: Get the process ID (PID) by process name.
- `Start`: Start monitoring a specific process.
- `Stop`: Stop monitoring the process.
- `Switch`: Switch the target process being monitored.

## Constructor

```cpp
PidWatcher();
```

## Destructor

```cpp
~PidWatcher();
```

## SetExitCallback Function

```cpp
void SetExitCallback(Callback callback);
```

```cpp
// Setting an exit callback function
pidWatcher.SetExitCallback([]() {
    // Custom exit callback function
});
```

## SetMonitorFunction Function

```cpp
void SetMonitorFunction(Callback callback, std::chrono::milliseconds interval);
```

```cpp
// Setting a monitor function with a 1-second interval
pidWatcher.SetMonitorFunction([]() {
    // Custom monitor callback function
}, std::chrono::seconds(1));
```

## GetPidByName Function

```cpp
pid_t GetPidByName(const std::string &name) const;
```

```cpp
// Getting the PID of a process by name
pid_t pid = pidWatcher.GetPidByName("example_process");
// Expected Output: PID of the process with the given name.
```

## Start Function

```cpp
bool Start(const std::string &name);
```

```cpp
// Starting monitoring of a process by name
bool started = pidWatcher.Start("example_process");
// Expected Output: true if monitoring started successfully, false otherwise.
```

## Stop Function

```cpp
void Stop();
```

```cpp
// Stopping the monitoring of the current process
pidWatcher.Stop();
// Note: This will stop monitoring the currently monitored process.
```

## Switch Function

```cpp
bool Switch(const std::string &name);
```

```cpp
// Switching the target process being monitored
bool switched = pidWatcher.Switch("new_process");
// Expected Output: true if the switch was successful, false otherwise.
```

## MonitorThread Function

Private function to run the monitoring thread.

## ExitThread Function

Private function to run the exit callback thread.
