# 锂 - 高度集成的天文摄影解决方案

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

## 概览

锂项目旨在为天文摄影爱好者及专业人士提供一个高度整合、轻量级的综合平台。它不仅是一个拍摄软件，还兼具设备控制服务器、系统管理以及广泛的可定制扩展能力，适用于多种应用场景和研究领域。

## 核心特性

- **多面手功能**：支持从图像捕获到设备管理，乃至系统级操作的全方位应用。
- **先进语言标准**：采用最新C++20标准编写，并兼容C++23部分特性，确保代码的现代性和高效性。
- **动态模块加载**：通过C++动态库实现的热更新机制，允许用户即时扩展功能，增强灵活性。
- **嵌入式脚本引擎**：集成高性能的pocketpy解析器，支持Python脚本，便于快速开发定制化逻辑。
- **广泛平台兼容**：全面适配Windows与Linux环境（包括x86_64及ARM架构），并具备部分MacOS支持。
- **丰富接口与组件**：提供全面的API接口和功能组件，满足天文摄影的多样化需求，鼓励用户开发缺失功能的模组。
- **开源授权模式**：遵循GPLv3协议，促进社区共享与合作，同时允许开发闭源插件以保护商业敏感代码。

## 构建指南

### 系统准备

#### Windows

推荐使用MSYS2环境，并通过清华大学开源软件镜像站加速下载。以下命令用于安装必要依赖：

```shell
# 添加清华大学镜像源
sed -i 's|https://mirror.msys2.org/|https://mirrors.tuna.tsinghua.edu.cn/msys2/|g' /etc/pacman.d/mirrorlist.mingw64
sed -i 's|https://mirror.msys2.org/|https://mirrors.tuna.tsinghua.edu.cn/msys2/|g' /etc/pacman.d/mirrorlist

# 更新系统包并安装编译工具链等
pacman -Syu
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-dlfcn mingw-w64-x86_64-cfitsio mingw-w64-x86_64-cmake mingw-w64-x86_64-libzip mingw-w64-x86_64-zlib mingw-w64-x86_64-fmt mingw-w64-x86_64-libnova make mingw-w64-x86_64-gtest
```

#### Ubuntu/Debian

```shell
sudo apt-get update && sudo apt-get upgrade
sudo apt-get install build-essential cmake libcfitsio-dev zlib1g-dev libssl-dev libzip-dev libfmt-dev
```

或使用提供的快捷构建脚本简化过程。

### 构建步骤

1. **创建构建目录**：`mkdir build && cd build`
2. **配置项目**：`cmake ..`
3. **编译执行**：使用`make -jN`或`cmake --build . -j N`命令进行并行编译，其中N为线程数。完成后，通过`./lithium_server`启动程序。

### 思维启迪

在深入探索锂项目的旅程中，我们秉承着对未知的好奇心与不懈的求知欲，正如那首改编的诗句所言，每一步尝试虽可能遭遇挑战与失败，却是通往智慧与理解的必经之路。让我们携手共进，在天文摄影的浩瀚星海中，以技术为舟，以创新为帆，不断前行。
