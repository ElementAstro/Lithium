#!/bin/bash

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to install a package using apt-get
install_with_apt() {
    sudo apt-get update
    sudo apt-get install -y "$1"
}

# Function to install a package using brew
install_with_brew() {
    brew install "$1"
}

# Detect platform and set package manager
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PACKAGE_MANAGER="apt"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PACKAGE_MANAGER="brew"
else
    echo "Unsupported platform: $OSTYPE"
    exit 1
fi

# Check and install CMake if necessary
if ! command_exists cmake; then
    echo "CMake is not installed. Installing CMake..."
    if [[ "$PACKAGE_MANAGER" == "apt" ]]; then
        install_with_apt cmake
    elif [[ "$PACKAGE_MANAGER" == "brew" ]]; then
        install_with_brew cmake
    fi
fi

# Check and install Git if necessary
if ! command_exists git; then
    echo "Git is not installed. Installing Git..."
    if [[ "$PACKAGE_MANAGER" == "apt" ]]; then
        install_with_apt git
    elif [[ "$PACKAGE_MANAGER" == "brew" ]]; then
        install_with_brew git
    fi
fi

# Check and install a C++ compiler if necessary
if ! command_exists g++ && ! command_exists clang++; then
    echo "No C++ compiler found. Installing g++..."
    if [[ "$PACKAGE_MANAGER" == "apt" ]]; then
        install_with_apt g++
    elif [[ "$PACKAGE_MANAGER" == "brew" ]]; then
        install_with_brew gcc
    fi
fi

# Create and navigate to the build directory
mkdir -p build
cd build

# Check if Google Test is already cloned
if [ ! -d "googletest" ]; then
    echo "Cloning Google Test..."
    git clone https://github.com/google/googletest.git
fi

# Configure the project with CMake
echo "Configuring the project with CMake..."
cmake ..

# Build the project
echo "Building the project..."
cmake --build .

# Run the tests
echo "Running the tests..."
ctest --output-on-failure

# Return to the original directory
cd ..

echo "Build and test process completed successfully."