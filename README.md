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
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=ElementAstro_Lithium&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=ElementAstro_Lithium)

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
