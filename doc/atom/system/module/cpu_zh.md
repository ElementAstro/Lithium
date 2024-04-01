# CPU 信息

## 简介

该 API 提供了用于获取 CPU 各种信息的函数，包括使用率百分比、温度、型号、标识符、频率、物理 CPU 数量和逻辑 CPU 数量。

## 函数

### getCurrentCpuUsage

#### 描述

获取 CPU 使用率百分比。

#### 返回值

- 类型: 浮点数
- 描述: CPU 使用率百分比。

#### 示例

```cpp
float cpuUsage = getCurrentCpuUsage();
std::cout << "当前CPU使用率: " << cpuUsage << "%" << std::endl;
```

### getCurrentCpuTemperature

#### 描述

获取 CPU 温度。

#### 返回值

- 类型: 浮点数
- 描述: CPU 温度。

#### 示例

```cpp
float cpuTemperature = getCurrentCpuTemperature();
std::cout << "当前CPU温度: " << cpuTemperature << "°C" << std::endl;
```

### getCPUModel

#### 描述

获取 CPU 型号。

#### 返回值

- 类型: 字符串
- 描述: CPU 型号。

#### 示例

```cpp
std::string cpuModel = getCPUModel();
std::cout << "CPU型号: " << cpuModel << std::endl;
```

### getProcessorIdentifier

#### 描述

获取 CPU 标识符。

#### 返回值

- 类型: 字符串
- 描述: CPU 标识符。

#### 示例

```cpp
std::string cpuIdentifier = getProcessorIdentifier();
std::cout << "CPU标识符: " << cpuIdentifier << std::endl;
```

### getProcessorFrequency

#### 描述

获取 CPU 频率。

#### 返回值

- 类型: 双精度浮点数
- 描述: CPU 频率。

#### 示例

```cpp
double cpuFrequency = getProcessorFrequency();
std::cout << "CPU频率: " << cpuFrequency << " GHz" << std::endl;
```

### getNumberOfPhysicalPackages

#### 描述

获取物理 CPU 数量。

#### 返回值

- 类型: 整数
- 描述: 物理 CPU 数量。

#### 示例

```cpp
int physicalCPUs = getNumberOfPhysicalPackages();
std::cout << "物理CPU数量: " << physicalCPUs << std::endl;
```

### getNumberOfPhysicalCPUs

#### 描述

获取逻辑 CPU 数量。

#### 返回值

- 类型: 整数
- 描述: 逻辑 CPU 数量。

#### 示例

```cpp
int logicalCPUs = getNumberOfPhysicalCPUs();
std::cout << "逻辑CPU数量: " << logicalCPUs << std::endl;
```
