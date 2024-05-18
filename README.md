# Lithium

<p align="center">
<img src="https://img.shields.io/badge/dialect-C%2B%2B20-blue">
<img src="https://img.shields.io/badge/license-GPL3-blue">
<img src="https://img.shields.io/badge/platform-Windows-green">
<img src= "https://img.shields.io/badge/platform-Linux%20x86__64--bit-green">
<img src="https://img.shields.io/badge/platform-Linux%20ARM-green">
<img src="https://img.shields.io/badge/platform-Ubuntu-green">
</p>

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/d3fed47a38e642a390d8ee506dc0acb3)](https://app.codacy.com/gh/ElementAstro/Lithium/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=ElementAstro_Lithium&metric=security_rating)](https://sonarcloud.io/summary/new_code?id=ElementAstro_Lithium)
[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?project=ElementAstro_Lithium&metric=ncloc)](https://sonarcloud.io/summary/new_code?id=ElementAstro_Lithium)
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=ElementAstro_Lithium&metric=bugs)](https://sonarcloud.io/summary/new_code?id=ElementAstro_Lithium)

## Introduction

Lithium, a lively and lightweight astrophotography terminal.

Features:

- Can be used as imaging software, device server, and system manager.
- Based on the latest C++20 standard, providing efficient functionality implementation (compatible with some C++17 features).
- Supports open loading, allowing dynamic loading of C++ dynamic libraries for hot updates.
- Built-in optimized Chaiscript parsing engine, providing flexible script support.
- Supports various types of plugins, facilitating feature expansion.
- Cross-platform compatibility, fully supporting Windows and Linux operating systems.
- Lightweight software design, while maintaining excellent performance.
- Provides a rich API, covering all necessary functions for astrophotography. ~~If not? Mods!~~
- Supports complex shooting sequences, enabling a programmable user experience.
- Uses the GPL3 open source license, **where the world belongs to open source**

## About Mod/Plugin

In Lithium, the component function is the most special function, providing a mod mechanism similar to Minecraft. The component function supports dynamic addition and insertion of functions, but due to the limitations of C++, we have imposed certain restrictions on the insertion of components to ensure system stability and security.

### Form of Components

- Injective Components: These components replace the implemented functions in `Lithium`. They inject `shared_ptr` into each Manager (similar to `ConfigManager`). The target of the injected function is the same as that of the Manager that has been injected into `GlobalPtrManager`. Components in this form can flexibly replace existing functions.

- Independent Components: These components use a distributed architecture and run in independent processes to ensure system security. When it is necessary to process sensitive data or perform complex calculations, these independent components can provide additional protection and isolation. To increase the security of components, `Lithium` also provides sandboxing functionality.

It should be noted that except for injective and independent components, other forms of components will be considered illegal and unsupported for loading, and will be directly ignored by the system.

### Component Levels

- Addon: The highest level of component, containing a series of Modules and Components

- Module: A module containing a dynamic library of an indefinite number of Components (depending on the platform)

- Component: A `shared_ptr` of a specific actual function or an executable function

All functions are declared in `package.json` for ease of use.

## How to build

### Install dependencies

Although efforts have been made to minimize the use of libraries, a few dependencies still need to be installed.

#### On Windows

```shell
sed -i "s#https\?://mirror.msys2.org/#https://mirrors.tuna.tsinghua.edu.cn/msys2/#g" /etc/pacman.d/mirrorlist*
pacman -Syu
pacman -S mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-dlfcn
pacman -S mingw-w64-x86_64-cfitsio
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-libzip
pacman -S mingw-w64-x86_64-zlib
pacman -S mingw-w64-x86_64-fmt
pacman -S mingw-w64-x86_64-libnova
pacman -S mingw-w64-x86_64-gsl

# for test
pacman -S mingw-w64-x86_64-gtest
```

#### On Ubuntu or other similar Linux platforms (No INDI needed)

```shell
sudo apt-get update && sudo apt-get upgrade -y
sudo apt install gcc g++ cmake
sudo apt install libcfitsio-dev zlib1g-dev libssl-dev libzip-dev libnova-dev libfmt-dev libudev-dev
```

Alternatively, you can directly run the provided script according to your platform:

```shell
sudo sh scripts/build_ci.sh
sh scripts/build_win.sh
```

#### Update GCC and Cmake

Unfortunately, the newest GCC and CMake are not available on Github Codespace, so we must install them manually.

```shell
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo apt-get update
sudo apt-get install gcc-13 g++-13 # GCC13 is the best choice, clang is alse OK

wget https://cmake.org/files/v3.28/cmake-3.28.0-rc5.tar.gz
tar -zxvf cmake-3.28.0-rc5.tar.gz
cd cmake-3.28.0-rc5
./bootstrap && make && sudo make install

#install newest clang-format
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo nano /etc/apt/sources.list
#deb http://apt.llvm.org/focal/ llvm-toolchain-focal-17 main
#deb-src http://apt.llvm.org/focal/ llvm-toolchain-focal-17 main
sudo apt install -y clang-format-17
```

Build the code:

```shell
mkdir build && cd build
cmake ..
make -j4 or cmake --build . -j4

./lithium_server
```

Everything is very simple. The entire process is straightforward.

Here is a poem adapted from a quote :

```txt
Learning requires not mere imagination,
Nor can it be attained through mediocre dedication.
It is through diligent accumulation,
That we shall grow in our education.

Our efforts may falter and fail,
But we must not surrender and bail.
We shall not halt our stride for fear of stumbling,
For setbacks are the price of pursuing enlightenment.

On this quest for truth, we shall encounter obstacles and doubts,
Yet we shall keep our resolve to seek understanding throughout.
Let us nurture a heart that yearns for wisdom and grace,
And never lose sight of this noble race.
```

## Technical Support

[![SonarCloud](https://sonarcloud.io/images/project_badges/sonarcloud-orange.svg)](https://sonarcloud.io/summary/new_code?id=ElementAstro_Lithium)

## 锂

## 简介

锂，轻量化的天文摄影终端

## 特性

- 可用作成像软件、设备服务器和系统管理器
- 基于最新的 C++20 标准，提供高效的功能实现（兼容部分 C++17 特性）
- 支持开放式加载，允许动态加载 C++动态库以进行热更新
- 内置经过优化的 Chaiscript 解析引擎，提供灵活的脚本支持
- 支持多样化插件类型，方便扩展功能
- 跨平台兼容性，完全支持 Windows 和 Linux 操作系统
- 软件设计轻量化，同时保持出色的性能表现
- 提供丰富的 API，覆盖所有天文摄影所需功能,~~功能没有怎么办？写模组！~~
- 支持复杂的拍摄序列，实现编程化的使用体验
- 采用 GPL3 开源许可协议，**世界属于开源**

## 关于模组/插件

在 Lithium，组件功能是最特殊的功能，提供类似于 Minecraft 的模组机制。组件功能支持动态添加和插入功能，但由于 C++自身的限制，我们对组件的插入进行了一定的限制，以确保系统的稳定性和安全性。

### 组件形式

- 注入式组件：这些组件替换了`Lithium`中已实现的功能。它们通过使用`shared_ptr`注入各个 Manager（类似与`ConfigManager`），目标与已注入`GlobalPtrManager`的管理器相同。这种形式的组件可以灵活替换现有功能。

- 独立式组件：这些组件采用分布式架构，在独立的进程中运行，以确保系统的安全性。当需要处理敏感数据或进行复杂的计算时，这种独立的组件能够提供额外的保护和隔离。为了增加组件的安全性，`Lithium`还提供了沙盒功能.

需要注意的是，除了注入式和独立式组件外，其他形式的组件都将被视为非法形式，不支持加载，并将被系统直接忽略。

### 组件级别

- Addon：最高级的组件，包含一系列的 Module 和 Component

- Module：模块，包含不定数量 Component 的动态库（根据平台而定）

- Component：组件，具体实际功能的`shared_ptr`，或者是可执行的函

所有功能均在`package.json`中声明，以方便使用。

## 如何构建

### 安装依赖项

尽管已经尽最大努力减少了库的使用，但仍需要安装一些依赖项

#### 在 Windows 平台下

```shell
# 添加清华镜像源，下载速度嘎嘎的
sed -i "s#https\?://mirror.msys2.org/#https://mirrors.tuna.tsinghua.edu.cn/msys2/#g" /etc/pacman.d/mirrorlist*
pacman -Syu
pacman -S mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-dlfcn
pacman -S mingw-w64-x86_64-cfitsio
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-libzip
pacman -S mingw-w64-x86_64-zlib
pacman -S mingw-w64-x86_64-fmt
pacman -S mingw-w64-x86_64-libnova
# 如果想用make构建
pacman -S make # 注意添加对应的目录，否则会当场爆炸

pacman -S mingw-w64-x86_64-gsl

# 测试用
pacman -S mingw-w64-x86_64-gtest
```

#### Ubuntu/Debian/Other Linux

```shell
sudo apt-get update && sudo apt-get upgrade -y
sudo apt install gcc g++ cmake
sudo apt install libcfitsio-dev zlib1g-dev libssl-dev libzip-dev libnova-dev libfmt-dev
```

或者您可以直接根据您的平台运行提供的脚本：

```shell
sudo sh scripts/build_ci.sh
sh scripts/build_win.sh
```

#### 构建代码

```shell
mkdir build && cd build
cmake ..
# -jn，n取决于你的电脑性能，一般是cpu核心数+2
make -j4 或 cmake --build . -j4

./lithium_server
```

一切都非常简单整个过程很简单

下面是一首小诗，改编自《三体》中的一句话：

```text
学习不仅仅需要想象，
也不能只凭平庸的奉献
通过勤奋的积累，
我们在教育中成长

我们的努力可能会摇摇欲坠，甚至失败，
但我们不能放弃和退缩
我们不应因为恐惧而停下脚步，
因为挫折是追求智慧的代价

在这探寻真理的旅途上，我们会遇到困难和疑虑，
但我们要始终坚定地追求理解
让我们培养一颗渴望智慧和优雅的心灵，
永远不要忘记这个崇高的竞赛
```
