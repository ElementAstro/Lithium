#!/bin/bash

set -e

echo "Checking system environment..."

# Check if pacman is available
if ! command -v pacman &> /dev/null; then
    echo "This script requires an MSYS2 environment with pacman."
    exit 1
fi

echo "Updating MSYS2 mirror list..."
sed -i "s#https\?://mirror.msys2.org/#https://mirrors.tuna.tsinghua.edu.cn/msys2/#g" /etc/pacman.d/mirrorlist*

echo "Updating system packages..."
pacman -Syu --noconfirm

echo "Installing mingw-w64-x86_64-toolchain..."
if ! pacman -Q mingw-w64-x86_64-toolchain &> /dev/null; then
    pacman -S --noconfirm mingw-w64-x86_64-toolchain
else
    echo "mingw-w64-x86_64-toolchain is already installed."
fi

echo "Installing mingw-w64-x86_64-dlfcn..."
if ! pacman -Q mingw-w64-x86_64-dlfcn &> /dev/null; then
    pacman -S --noconfirm mingw-w64-x86_64-dlfcn
else
    echo "mingw-w64-x86_64-dlfcn is already installed."
fi

echo "Installing mingw-w64-x86_64-cfitsio..."
if ! pacman -Q mingw-w64-x86_64-cfitsio &> /dev/null; then
    pacman -S --noconfirm mingw-w64-x86_64-cfitsio
else
    echo "mingw-w64-x86_64-cfitsio is already installed."
fi

echo "Installing mingw-w64-x86_64-cmake..."
if ! pacman -Q mingw-w64-x86_64-cmake &> /dev/null; then
    pacman -S --noconfirm mingw-w64-x86_64-cmake
else
    echo "mingw-w64-x86_64-cmake is already installed."
fi

echo "Installing mingw-w64-x86_64-libzip..."
if ! pacman -Q mingw-w64-x86_64-libzip &> /dev/null; then
    pacman -S --noconfirm mingw-w64-x86_64-libzip
else
    echo "mingw-w64-x86_64-libzip is already installed."
fi

echo "Installing mingw-w64-x86_64-zlib..."
if ! pacman -Q mingw-w64-x86_64-zlib &> /dev/null; then
    pacman -S --noconfirm mingw-w64-x86_64-zlib
else
    echo "mingw-w64-x86_64-zlib is already installed."
fi

echo "Installing mingw-w64-x86_64-fmt..."
if ! pacman -Q mingw-w64-x86_64-fmt &> /dev/null; then
    pacman -S --noconfirm mingw-w64-x86_64-fmt
else
    echo "mingw-w64-x86_64-fmt is already installed."
fi

echo "Installing mingw-w64-x86_64-libnova..."
if ! pacman -Q mingw-w64-x86_64-libnova &> /dev/null; then
    pacman -S --noconfirm mingw-w64-x86_64-libnova
else
    echo "mingw-w64-x86_64-libnova is already installed."
fi

echo "Environment setup completed."