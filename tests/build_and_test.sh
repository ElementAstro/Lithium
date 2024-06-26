#!/bin/bash

# This script sets up a development environment by installing necessary tools like CMake, Git, and a C++ compiler.
# It supports both Linux (using apt-get) and macOS (using brew) platforms.

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to install a package using apt-get (for Debian-based systems)
install_with_apt() {
    echo "Updating package lists..."
    sudo apt-get update
    echo "Installing $1..."
    sudo apt-get install -y "$1"
}

# Function to install a package using brew (for macOS)
install_with_brew() {
    echo "Installing $1..."
    brew install "$1"
}

# Detect platform and set package manager
detect_package_manager() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "Using apt-get for package management."
        PACKAGE_MANAGER="apt"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "Using brew for package management."
        PACKAGE_MANAGER="brew"
    else
        echo "Unsupported platform: $OSTYPE"
        exit 1
    fi
}

# Main installation routine
main() {
    detect_package_manager

    # Install CMake if not installed
    if ! command_exists cmake; then
        echo "CMake is not installed."
        if [[ "$PACKAGE_MANAGER" == "apt" ]]; then
            install_with_apt cmake
        elif [[ "$PACKAGE_MANAGER" == "brew" ]]; then
            install_with_brew cmake
        fi
    fi

    # Install Git if not installed
    if ! command_exists git; then
        echo "Git is not installed."
        if [[ "$PACKAGE_MANAGER" == "apt" ]]; then
            install_with_apt git
        elif [[ "$PACKAGE_MANAGER" == "brew" ]]; then
            install_with_brew git
        fi
    fi

    # Install C++ compiler if not found
    if ! command_exists g++ && ! command_exists clang++; then
        echo "No C++ compiler found."
        if [[ "$PACKAGE_MANAGER" == "apt" ]]; then
            install_with_apt g++
        elif [[ "$PACKAGE_MANAGER" == "brew" ]]; then
            install_with_brew gcc
        fi
    fi

    # Create and navigate to the build directory
    mkdir -p build
    cd build || exit

    # Clone Google Test if not already present
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
}

# Call the main routine
main