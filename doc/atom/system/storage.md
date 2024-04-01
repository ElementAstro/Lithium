# StorageMonitor Class and Member Functions

## Description

The `StorageMonitor` class monitors storage space changes for all mounted devices and triggers registered callback functions when storage space changes occur.

## Constructor

Default constructor for `StorageMonitor`.

```cpp
StorageMonitor monitor;
```

## registerCallback

Registers a callback function to be triggered when storage space changes occur.

```cpp
monitor.registerCallback([](const std::string &path) {
    std::cout << "Storage space changed at path: " << path << std::endl;
});
```

## startMonitoring

Starts monitoring storage space.

```cpp
bool success = monitor.startMonitoring();
// Expected Output: true or false
```

## stopMonitoring

Stops monitoring storage space.

```cpp
monitor.stopMonitoring();
```

## triggerCallbacks

Triggers all registered callback functions.

```cpp
monitor.triggerCallbacks("/mnt/usb");
```

## isNewMediaInserted

Checks if a new storage device is inserted at the specified path.

```cpp
bool inserted = monitor.isNewMediaInserted("/mnt/usb");
// Expected Output: true or false
```

## listAllStorage

Lists all mounted storage spaces. (Debug feature)

## listFiles

Lists files in the specified path. (Debug feature)

```cpp
monitor.listFiles("/mnt/usb");
```

## monitorUdisk

Monitors USB disks on Linux systems.

### Special Note

This function is specific to Linux systems.

```cpp
#ifdef __linux__
monitorUdisk(monitor);
#endif
```

## Member Variables

- `m_storagePaths`: Contains all mounted storage space paths.
- `m_lastCapacity`: Stores the last recorded storage space capacity.
- `m_lastFree`: Stores the last recorded available storage space.
- `m_mutex`: Mutex for thread-safe protection of data structures.
- `m_callbacks`: List of registered callback functions.
- `m_isRunning`: Flag indicating if monitoring is running.
