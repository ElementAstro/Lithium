# Lithium - The Lightweight Suite for Astronomical Imaging

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

## Overview

The Lithium project is designed as a comprehensive, lightweight platform for astronomy enthusiasts and professionals alike, offering not just capture software but also serving as a device control server, system manager, and a platform for extensive customization, adaptable to various applications and research domains.

## Key Features

- **Versatile Functionality**: Supports full-spectrum application from image acquisition to device management and system operations.
- **Contemporary Language Standard**: Built using the latest C++20 standard with compatibility for select C++23 features, ensuring modernity and efficiency in code.
- **Dynamic Module Loading**: Enables hot updates through C++ dynamic libraries, facilitating instant expansion of capabilities and enhancing flexibility.
- **Embedded Scripting Engine**: Integrates a high-performance pocketpy interpreter supporting Python scripts for rapid development of tailored logic.
- **Broad Platform Compatibility**: Fully supports Windows and Linux environments (including x86_64 and ARM architectures), with partial support for MacOS.
- **Comprehensive APIs and Components**: Offers a wide range of APIs and functional components that cater to diverse astronomical imaging needs, encouraging users to develop modules for missing functionalities.
- **Open-Source Licensing Model**: Adheres to the GPLv3 license, fostering community sharing and collaboration while allowing for proprietary plugins to protect business-sensitive code.
- **Educational and Inspirational**: Encourages learning and innovation through high-quality code examples and documentation, promoting knowledge dissemination and skill enhancement.

## Building Instructions

### System Preparation

#### Windows

It is recommended to use the MSYS2 environment and leverage the Tsinghua University Open Source Software Mirror for expedited downloads. The following commands install necessary dependencies:

```shell
# Add Tsinghua University mirror source
sed -i 's|https://mirror.msys2.org/|https://mirrors.tuna.tsinghua.edu.cn/msys2/|g' /etc/pacman.d/mirrorlist.mingw64

# Update system packages and install build tools and dependencies
pacman -Syu
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-dlfcn mingw-w64-x86_64-cfitsio mingw-w64-x86_64-cmake mingw-w64-x86_64-libzip mingw-w64-x86_64-zlib mingw-w64-x86_64-fmt mingw-w64-x86_64-libnova make mingw-w64-x86_64-gtest

pacman -S mingw-w64-x86_64-clang-analyzer
pacman -S mingw-w64-x86_64-clang-tools-extra
pacman -S mingw-w64-x86_64-opencv
```

#### Ubuntu/Debian

```shell
sudo apt-get update && sudo apt-get upgrade
sudo apt-get install build-essential cmake libcfitsio-dev zlib1g-dev libssl-dev libzip-dev libfmt-dev
```

Alternatively, utilize the provided quick-build scripts to streamline the process.

### Building Steps

1. **Create Build Directory**: `mkdir build && cd build`
2. **Configure Project**: `cmake ..`
3. **Compile and Execute**: Use `make -jN` or `cmake --build . --parallel N` commands to compile in parallel, where N denotes the number of threads. Afterwards, launch the program via `./lithium_server`.

### Intellectual Inspiration

Embarking on the journey with Lithium, we embrace curiosity and an unwavering pursuit of knowledge, echoing the adapted verse which reminds us that every attempt, though fraught with challenges and setbacks, is a necessary step toward wisdom and understanding. Together, let us navigate the vast cosmos of astronomical imaging, our technology the vessel, innovation our sail, advancing relentlessly forward.

<figure><embed src="https://wakatime.com/share/@018d39f0-57c9-4a13-aedb-90454b61e6cc/e1b2b694-2ecc-4cfd-9c75-2d01b8581e4d.svg"></embed></figure>
<figure><embed src="https://wakatime.com/share/@018d39f0-57c9-4a13-aedb-90454b61e6cc/d6c29fb7-c5b3-4ffc-8e17-16634c7b669a.svg"></embed></figure>
