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

# List of packages to install
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
)

install_package() {
    local package=$1
    echo "Installing $package..."
    if ! pacman -Q $package &> /dev/null; then
        pacman -S --noconfirm $package
    else
        echo "$package is already installed."
    fi
}

for package in "${packages[@]}"; do
    install_package $package
done

echo "Environment setup completed."