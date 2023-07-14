# Lithium 锂

# Introduction 简介

像锂一样活泼的轻量化天文摄影终端

## Features 特性

等待最后的修改

## How to build 如何构建

### Install dependencies 安装依赖项

诚然，已经尽可能使用了少的库，但是依然需要安装一些依赖库

在Windows平台下

```
sed -i "s#https\?://mirror.msys2.org/#https://mirrors.tuna.tsinghua.edu.cn/msys2/#g" /etc/pacman.d/mirrorlist*
pacman -Syu
pacman -S mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-dlfcn
pacman -S mingw-w64-x86_64-fmt
pacman -S mingw-w64-x86_64-cfitsio
pacman -S mingw-w64-x86_64-cmake
```

在Ubuntu或者其他的类似Linux平台下
```
sudo apt-add-repository ppa:mutlaqja/ppa -y
sudo apt update && sudo apt upgrade -y
sudo apt-get install libindi1 indi-bin libindi-dev
sudo apt install libcfitsio-dev libz-dev libssl-dev
```

或者直接根据平台运行现成的脚本

```
sudo sh scripts/build_ci.sh
sh scripts/build_win.sh
```

### Build the code 构建代码

```
mkdir build && cd build
cmake ..
make -j4 or cmake --build . -j4

./lithium_server
```

Everything is very simple. 整个过程都很简单awa

下面是一首小诗，改编自三体中的一段台词

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