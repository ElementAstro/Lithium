#!/bin/bash

set -e

echo "Checking system environment..."

# 检查是否在 MSYS2 环境中运行
if ! command -v pacman &> /dev/null; then
    echo "This script requires an MSYS2 environment with pacman."
    exit 1
fi

# 更新 MSYS2 镜像列表
echo "Updating MSYS2 mirror list..."
sed -i "s#https\?://mirror.msys2.org/#https://mirrors.tuna.tsinghua.edu.cn/msys2/#g" /etc/pacman.d/mirrorlist*

# 更新系统包
echo "Updating system packages..."
pacman -Syu --noconfirm

# 要安装的包列表
packages=(
    "mingw-w64-x86_64-toolchain"
    "mingw-w64-x86_64-dlfcn"
    "mingw-w64-x86_64-cfitsio"
    "mingw-w64-x86_64-cmake"
    "mingw-w64-x86_64-libzip"
    "mingw-w64-x86_64-zlib"
    "mingw-w64-x86_64-fmt"
    "mingw-w64-x86_64-libnova"
    "mingw-w64-x86_64-libusb"
    "mingw-w64-x86_64-minizip-ng"
    "mingw-w64-x86_64-make"
    "mingw-w64-x86_64-openssl"
    "mingw-w64-x86_64-python"
)

# 安装包的函数
install_package() {
    local package=$1
    echo "Installing $package..."
    if ! pacman -Q $package &> /dev/null; then
        pacman -S --noconfirm $package
    else
        echo "$package is already installed."
    fi
}

# 安装所有包
for package in "${packages[@]}"; do
    install_package $package
done

# 检查安装的依赖项是否正确
check_installed() {
    local package=$1
    if pacman -Q $package &> /dev/null; then
        echo "$package is properly installed."
    else
        echo "$package is not installed correctly. Please check the installation manually."
    fi
}

# 检查所有包
for package in "${packages[@]}"; do
    check_installed $package
done

# 设置环境变量
echo "Setting up environment variables..."
export PATH="/mingw64/bin:$PATH"
echo 'export PATH="/mingw64/bin:$PATH"' >> ~/.bashrc

# 其他检查和优化选项
echo "Performing additional checks and optimizations..."

# 检查 make 是否安装
if ! command -v make &> /dev/null; then
    echo "make is not installed. Installing..."
    pacman -S --noconfirm mingw-w64-x86_64-make
fi

# 检查 git 是否安装
if ! command -v git &> /dev/null; then
    echo "git is not installed. Installing..."
    pacman -S --noconfirm mingw-w64-x86_64-git
fi

# 检查 curl 是否安装
if ! command -v curl &> /dev/null; then
    echo "curl is not installed. Installing..."
    pacman -S --noconfirm mingw-w64-x86_64-curl
fi

# 检查 wget 是否安装
if ! command -v wget &> /dev/null; then
    echo "wget is not installed. Installing..."
    pacman -S --noconfirm mingw-w64-x86_64-wget
fi

echo "Cleaning up unnecessary packages and cache..."
pacman -Sc --noconfirm

echo "Environment setup completed."
