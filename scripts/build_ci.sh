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

# Check if CMake is installed
if ! command -v cmake &> /dev/null; then
    echo "CMake is not installed. Installing..."
    apt-get install -y cmake
else
    echo "CMake is already installed."
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