#!/bin/bash

set -e

echo "Checking system environment..."

# Check if the script is run with sudo
if [[ $EUID -ne 0 ]]; then
    echo "This script must be run with sudo."
    exit 1
fi

# Check if the system is Debian-based
if ! command -v apt-get &> /dev/null; then
    echo "This script only supports Debian-based systems."
    exit 1
fi

# Check if gcc and g++ are installed
if ! command -v gcc &> /dev/null || ! command -v g++ &> /dev/null; then
    echo "gcc and g++ are not installed. Installing..."
    apt-get install -y gcc g++
else
    echo "gcc and g++ are already installed."
fi

# Check if g++ version is at least 10
gpp_version=$(g++ --version | grep -oP '(?<=g\+\+ )[0-9]+')
if [ "$gpp_version" -lt "10" ]; then
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
    sudo apt-get install gcc-13 g++-13 -y
fi

# Check if CMake is installed
if ! command -v cmake &> /dev/null; then
    echo "CMake is not installed. Installing..."
    apt-get install -y cmake
else
    echo "CMake is already installed."
fi

# Check if CMake version is at least 3.20
cmake_version=$(cmake --version | grep -oP '(?<=version )([0-9]+\.[0-9]+)')
if [ "$(printf "%s\n" "3.20" "$cmake_version" | sort -V | head -n1)" != "3.20" ]; then
    wget https://cmake.org/files/v3.28/cmake-3.28.0-rc5.tar.gz
    tar -zxvf cmake-3.28.0-rc5.tar.gz
    cd cmake-3.28.0-rc5
    ./bootstrap && make && sudo make install
    cd ..
fi

echo "Updating system packages..."
apt-get update
apt-get upgrade -y

echo "Installing dependencies..."
apt-get install -y libcfitsio-dev zlib1g-dev libssl-dev libzip-dev libnova-dev libfmt-dev

echo "Checking installed dependencies..."
for lib in cfitsio zlib ssl zip nova fmt; do
    if ! ldconfig -p | grep -q "lib$lib"; then
        echo "lib$lib is not properly installed. Please check the installation manually."
    else
        echo "lib$lib is properly installed."
    fi
done

echo "Build environment setup completed."
