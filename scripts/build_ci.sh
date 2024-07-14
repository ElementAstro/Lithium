#!/bin/sh

set -e

echo "Checking system environment..."

# 检查是否以 sudo 运行脚本
if [ "$(id -u)" -ne 0 ]; then
    echo "This script must be run with sudo."
    exit 1
fi

# 检查系统包管理器
if command -v apt-get > /dev/null; then
    PKG_MANAGER="apt-get"
    PKG_INSTALL="$PKG_MANAGER install -y"
    PKG_UPDATE="$PKG_MANAGER update"
    PKG_UPGRADE="$PKG_MANAGER upgrade -y"
    PKG_REMOVE="$PKG_MANAGER autoremove -y"
    PKG_CLEAN="$PKG_MANAGER clean"
    DEPENDENCIES="libcfitsio-dev zlib1g-dev libssl-dev libzip-dev libnova-dev libfmt-dev libopencv-dev build-essential software-properties-common uuid-dev"
elif command -v yum > /dev/null; then
    PKG_MANAGER="yum"
    PKG_INSTALL="$PKG_MANAGER install -y"
    PKG_UPDATE="$PKG_MANAGER update -y"
    PKG_UPGRADE="$PKG_MANAGER upgrade -y"
    PKG_REMOVE="$PKG_MANAGER autoremove -y"
    PKG_CLEAN="$PKG_MANAGER clean all"
    DEPENDENCIES="cfitsio-devel zlib-devel openssl-devel libzip-devel nova-devel fmt-devel opencv-devel make automake gcc gcc-c++ kernel-devel uuid-devel"
else
    echo "This script only supports Debian-based or RedHat-based systems."
    exit 1
fi

# 更新系统包
echo "Updating system packages..."
$PKG_UPDATE
$PKG_UPGRADE

# 安装或更新 gcc 和 g++
if ! command -v gcc > /dev/null || ! command -v g++ > /dev/null; then
    echo "gcc and g++ are not installed. Installing..."
    $PKG_INSTALL gcc g++
else
    echo "gcc and g++ are already installed."
fi

# 检查 g++ 版本是否至少为 10
gpp_version=$(g++ -dumpversion | cut -f1 -d.)
if [ "$gpp_version" -lt "10" ]; then
    if [ "$PKG_MANAGER" = "apt-get" ]; then
        echo "Installing gcc-13 and g++-13..."
        add-apt-repository ppa:ubuntu-toolchain-r/test -y
        $PKG_UPDATE
        $PKG_INSTALL gcc-13 g++-13
        update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100
        update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100
    elif [ "$PKG_MANAGER" = "yum" ]; then
        echo "Installing devtoolset-10..."
        $PKG_INSTALL centos-release-scl
        $PKG_INSTALL devtoolset-10
        source /opt/rh/devtoolset-10/enable
    fi
fi

# 安装或更新 CMake
if ! command -v cmake > /dev/null; then
    echo "CMake is not installed. Installing..."
    $PKG_INSTALL cmake
else
    echo "CMake is already installed."
fi

# 检查 CMake 版本是否至少为 3.20
cmake_version=$(cmake --version | awk -F " " '/version/ {print $3}')
if [ "$(printf '%s\n' "3.20" "$cmake_version" | sort -V | head -n1)" != "3.20" ]; then
    echo "Updating CMake to version 3.28..."
    wget https://cmake.org/files/v3.28/cmake-3.28.0-rc5.tar.gz
    tar -zxvf cmake-3.28.0-rc5.tar.gz
    cd cmake-3.28.0-rc5
    ./bootstrap && make -j$(nproc) && make install
    cd ..
    rm -rf cmake-3.28.0-rc5 cmake-3.28.0-rc5.tar.gz
fi

# 安装依赖项
echo "Installing dependencies..."
$PKG_INSTALL $DEPENDENCIES

# 其他检查和优化选项
echo "Performing additional checks and optimizations..."

# 检查 make 是否安装
if ! command -v make > /dev/null; then
    echo "make is not installed. Installing..."
    $PKG_INSTALL make
fi

# 检查 git 是否安装
if ! command -v git > /dev/null; then
    echo "git is not installed. Installing..."
    $PKG_INSTALL git
fi

# 检查 curl 是否安装
if ! command -v curl > /dev/null; then
    echo "curl is not installed. Installing..."
    $PKG_INSTALL curl
fi

# 检查 wget 是否安装
if ! command -v wget > /dev/null; then
    echo "wget is not installed. Installing..."
    $PKG_INSTALL wget
fi

# 清理不必要的包和缓存
echo "Cleaning up unnecessary packages and cache..."
$PKG_REMOVE
$PKG_CLEAN

echo "Build environment setup completed."
