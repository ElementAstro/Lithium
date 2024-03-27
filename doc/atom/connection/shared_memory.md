# SharedMemory Class

## Overview

The `SharedMemory` class provides a way to implement shared memory for inter-process communication. It allows data to be written to and read from shared memory, with support for synchronization and thread safety.

## Constructor

### Signature

```cpp
explicit SharedMemory(const std::string &name, bool create = true);
```

### Example

```cpp
SharedMemory<int> sharedMem("example_shared_mem", true);
```

## Destructor

### Signature

```cpp
~SharedMemory();
```

## Methods

### 1. write

#### Signature

```cpp
void write(const T &data, std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
```

#### Example

```cpp
int dataToWrite = 42;
sharedMem.write(dataToWrite);
```

### 2. read

#### Signature

```cpp
T read(std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) const;
```

#### Example

```cpp
int dataRead = sharedMem.read();
```

### 3. clear

#### Signature

```cpp
void clear();
```

#### Example

```cpp
sharedMem.clear();
```

### 4. isOccupied

#### Signature

```cpp
bool isOccupied() const;
```

#### Example

```cpp
bool occupied = sharedMem.isOccupied();
```

## Usage Notes

- The template type `T` must be trivially copyable and have a standard layout.
- The class provides support for both Windows and Unix-like systems for shared memory management.
- Mutual exclusion is ensured using an atomic flag and mutex to maintain data consistency in concurrent access scenarios.
