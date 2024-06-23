#!/bin/bash

set -e

echo "Checking system environment..."

# 检查是否以 sudo 运行脚本
if [[ $EUID -ne 0 ]]; then
    echo "This script must be run with sudo."
    exit 1
fi

# 检查是否为基于 Debian 的系统
if ! command -v apt-get &> /dev/null; then
    echo "This script only supports Debian-based systems."
    exit 1
fi

# 安装或更新 gcc 和 g++
if ! command -v gcc &> /dev/null || ! command -v g++ &> /dev/null; then
    echo "gcc and g++ are not installed. Installing..."
    apt-get install -y gcc g++
else
    echo "gcc and g++ are already installed."
fi

# 检查 g++ 版本是否至少为 10
gpp_version=$(g++ -dumpversion | cut -f1 -d.)
if [ "$gpp_version" -lt "10" ]; then
    add-apt-repository ppa:ubuntu-toolchain-r/test -y
    apt-get update
    apt-get install -y gcc-13 g++-13
fi

# 安装或更新 CMake
if ! command -v cmake &> /dev/null; then
    echo "CMake is not installed. Installing..."
    apt-get install -y cmake
else
    echo "CMake is already installed."
fi

# 检查 CMake 版本是否至少为 3.20
cmake_version=$(cmake --version | awk -F " " '/version/ {print $3}')
if dpkg --compare-versions "$cmake_version" lt "3.20"; then
    wget https://cmake.org/files/v3.28/cmake-3.28.0-rc5.tar.gz
    tar -zxvf cmake-3.28.0-rc5.tar.gz
    cd cmake-3.28.0-rc5
    ./bootstrap && make && make install
    cd ..
    rm -rf cmake-3.28.0-rc5 cmake-3.28.0-rc5.tar.gz
fi

echo "Updating system packages..."
apt-get update
apt-get upgrade -y

echo "Installing dependencies..."
dependencies=(
    libcfitsio-dev
    zlib1g-dev
    libssl-dev
    libzip-dev
    libnova-dev
    libfmt-dev
)

apt-get install -y "${dependencies[@]}"

echo "Checking installed dependencies..."
for lib in "${dependencies[@]}"; do
    lib_name=$(echo $lib | sed 's/-dev//')
    if ! ldconfig -p | grep -q "lib${lib_name}"; then
        echo "${lib_name} is not properly installed. Please check the installation manually."
    else
        echo "${lib_name} is properly installed."
    fi
done

echo "Build environment setup completed."