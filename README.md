Lithium
=======

<p align="center">
    <a href="https://isocpp.org/">
        <img src="https://img.shields.io/badge/language-C%2B%2B20-blue.svg">
    </a>
    <a href="https://www.gnu.org/licenses/gpl-3.0.en.html" >
        <img src="https://img.shields.io/github/license/ElementAstro/Lithium">
    </a>
</p>

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
- Uses the GPL3 open source license, __where the world belongs to open source__

## How to build:

### Install dependencies:

Although efforts have been made to minimize the use of libraries, a few dependencies still need to be installed.

On Windows platform:

```shell
sed -i "s#https\?://mirror.msys2.org/#https://mirrors.tuna.tsinghua.edu.cn/msys2/#g" /etc/pacman.d/mirrorlist*
pacman -Syu
pacman -S mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-dlfcn
pacman -S mingw-w64-x86_64-cfitsio
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-libzip
pacman -S mingw-w64-x86_64-zlib
pacman -S mingw-w64-x86_64-fftw
pacman -S mingw-w64-x86_64-fmt
pacman -S mingw-w64-x86_64-libnova
```

On Ubuntu or other similar Linux platforms:

```shell
sudo apt-add-repository ppa:mutlaqja/ppa -y
sudo apt update && sudo apt upgrade -y
sudo apt install gcc g++ cmake
sudo apt-get install libindi1 indi-bin libindi-dev
sudo apt install libcfitsio-dev zlib1g-dev libssl-dev libzip-dev
```

Alternatively, you can directly run the provided script according to your platform:

```shell
sudo sh scripts/build_ci.sh
sh scripts/build_win.sh
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
```
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

锂
=

## 简介

锂，轻量化的天文摄影终端

## 特性
- 可用作成像软件、设备服务器和系统管理器
- 基于最新的C++20标准，提供高效的功能实现（兼容部分C++17特性）
- 支持开放式加载，允许动态加载C++动态库以进行热更新
- 内置经过优化的Chaiscript解析引擎，提供灵活的脚本支持
- 支持多样化插件类型，方便扩展功能
- 跨平台兼容性，完全支持Windows和Linux操作系统
- 软件设计轻量化，同时保持出色的性能表现
- 提供丰富的API，覆盖所有天文摄影所需功能,~~功能没有怎么办？写模组！~~
- 支持复杂的拍摄序列，实现编程化的使用体验
- 采用GPL3开源许可协议，__世界属于开源__

## 如何构建

### 安装依赖项

尽管已经尽最大努力减少了库的使用，但仍需要安装一些依赖项

在Windows平台下：

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
# 如果想用make构建
pacman -S make
```

在Ubuntu或其他类似的Linux平台下：

```shell
# 安装INDI及其附属库
sudo apt-add-repository ppa:mutlaqja/ppa -y
sudo apt update && sudo apt upgrade -y
sudo apt-get install libindi1 indi-bin libindi-dev
sudo apt install gcc g++ cmake
sudo apt install libcfitsio-dev zlib1g-dev libssl-dev libzip-dev
```

或者您可以直接根据您的平台运行提供的脚本：

```shell
sudo sh scripts/build_ci.sh
sh scripts/build_win.sh
```

构建代码：

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
