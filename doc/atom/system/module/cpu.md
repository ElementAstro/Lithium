# CPU Information API Documentation

## Introduction

This API provides functions to retrieve various information about the CPU, including usage percentage, temperature, model, identifier, frequency, number of physical CPUs, and number of logical CPUs.

## Functions

### getCurrentCpuUsage

#### Description

Get the CPU usage percentage.

#### Returns

- Type: float
- Description: The CPU usage percentage.

#### Example

```cpp
float cpuUsage = getCurrentCpuUsage();
std::cout << "Current CPU Usage: " << cpuUsage << "%" << std::endl;
```

### getCurrentCpuTemperature

#### Description

Get the CPU temperature.

#### Returns

- Type: float
- Description: The CPU temperature.

#### Example

```cpp
float cpuTemperature = getCurrentCpuTemperature();
std::cout << "Current CPU Temperature: " << cpuTemperature << "°C" << std::endl;
```

### getCPUModel

#### Description

Get the CPU model.

#### Returns

- Type: string
- Description: The CPU model.

#### Example

```cpp
std::string cpuModel = getCPUModel();
std::cout << "CPU Model: " << cpuModel << std::endl;
```

### getProcessorIdentifier

#### Description

Get the CPU identifier.

#### Returns

- Type: string
- Description: The CPU identifier.

#### Example

```cpp
std::string cpuIdentifier = getProcessorIdentifier();
std::cout << "CPU Identifier: " << cpuIdentifier << std::endl;
```

### getProcessorFrequency

#### Description

Get the CPU frequency.

#### Returns

- Type: double
- Description: The CPU frequency.

#### Example

```cpp
double cpuFrequency = getProcessorFrequency();
std::cout << "CPU Frequency: " << cpuFrequency << " GHz" << std::endl;
```

### getNumberOfPhysicalPackages

#### Description

Get the number of physical CPUs.

#### Returns

- Type: int
- Description: The number of physical CPUs.

#### Example

```cpp
int physicalCPUs = getNumberOfPhysicalPackages();
std::cout << "Number of Physical CPUs: " << physicalCPUs << std::endl;
```

### getNumberOfPhysicalCPUs

#### Description

Get the number of logical CPUs.

#### Returns

- Type: int
- Description: The number of logical CPUs.

#### Example

```cpp
int logicalCPUs = getNumberOfPhysicalCPUs();
std::cout << "Number of Logical CPUs: " << logicalCPUs << std::endl;
```
