# 锂

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
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=ElementAstro_Lithium&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=ElementAstro_Lithium)

## 简介

锂，轻量化的开放加载框架

## 特性

- 可用作成像软件、设备服务器和系统管理器，或者是其他轻量级应用
- 基于最新的 C++20 标准，提供高效实现（兼容部分 **C++23** 特性）
- 支持开放式加载，允许动态加载 C++动态库热更新，基于Atom架构
- 内置经过优化的 Pocketpy 解析引擎，提供灵活的Python脚本支持
- 跨平台兼容性，完全支持 Windows 和 Linux 操作系统
- 提供丰富的 API，覆盖所有天文摄影所需功能,~~功能没有怎么办？写模组！~~
- 采用 GPL3 开源许可协议，**世界属于开源**

## 模组/插件

在 Lithium，组件功能是最特殊的功能，提供类似于 Minecraft 的模组机制。组件功能支持动态添加和插入功能，但由于 C++语言限制，组件功能没有办法做到像python等动态语言那样自由，具有一定的限制和标准：

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
