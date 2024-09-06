#!/bin/bash

# 脚本：CMake MinGW项目打包脚本
# Script: CMake MinGW Project Packaging Script
# 描述：这个脚本用于构建、安装和打包基于CMake的MinGW项目。
# Description: This script is used to build, install, and package CMake-based MinGW projects.
# 作者：Max Qian
# Author: Max Qian
# 版本：1.2
# Version: 1.2
# 使用方法：./package.sh [clean|build|package]
# Usage: ./package.sh [clean|build|package]

# ===== 变量设置 / Variable Settings =====
# 项目名称 / Project name
PROJECT_NAME="MyProject"
# 构建目录 / Build directory
BUILD_DIR="build"
# 安装目录 / Install directory
INSTALL_DIR="install"
# 打包目录 / Package directory
PACKAGE_DIR="package"
# 版本号（从git获取，如果失败则使用默认值）
# Version number (obtained from git, use default if failed)
VERSION=$(git describe --tags --always --dirty 2>/dev/null || echo "1.0.0")
# 最终生成的压缩包名称 / Name of the final generated archive
ARCHIVE_NAME="${PROJECT_NAME}-${VERSION}-win64.zip"

# ===== 颜色代码 / Color Codes =====
# 用于美化控制台输出 / Used to beautify console output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color / 无颜色

# ===== 辅助函数 / Helper Functions =====
# 输出信息日志 / Output information log
log_info() {
    echo -e "${GREEN}[INFO] $1${NC}"
}

# 输出警告日志 / Output warning log
log_warn() {
    echo -e "${YELLOW}[WARN] $1${NC}"
}

# 输出错误日志 / Output error log
log_error() {
    echo -e "${RED}[ERROR] $1${NC}"
}

# 检查命令是否存在 / Check if a command exists
check_command() {
    if ! command -v $1 &> /dev/null; then
        log_error "$1 could not be found. Please install it and try again."
        log_error "$1 未找到。请安装后重试。"
        exit 1
    fi
}

# ===== 主要功能函数 / Main Function =====
# 清理函数：删除之前的构建产物
# Clean function: Remove previous build artifacts
clean() {
    log_info "Cleaning previous build artifacts..."
    log_info "清理之前的构建产物..."
    rm -rf $BUILD_DIR $INSTALL_DIR $PACKAGE_DIR
}

# 构建函数：配置CMake，编译项目，并安装
# Build function: Configure CMake, compile the project, and install
build() {
    log_info "Configuring project with CMake..."
    log_info "使用CMake配置项目..."
    mkdir -p $BUILD_DIR
    cd $BUILD_DIR
    # 使用MinGW Makefiles生成器，设置安装前缀和构建类型
    # Use MinGW Makefiles generator, set install prefix and build type
    cmake -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=../$INSTALL_DIR -DCMAKE_BUILD_TYPE=Release ..
    if [ $? -ne 0 ]; then
        log_error "CMake configuration failed."
        log_error "CMake配置失败。"
        exit 1
    fi

    log_info "Building project..."
    log_info "构建项目..."
    # 使用CMake构建项目 / Build the project using CMake
    cmake --build . --config Release
    if [ $? -ne 0 ]; then
        log_error "Build failed."
        log_error "构建失败。"
        exit 1
    fi

    log_info "Installing project..."
    log_info "安装项目..."
    # 安装项目到指定目录 / Install the project to the specified directory
    cmake --install .
    if [ $? -ne 0 ]; then
        log_error "Installation failed."
        log_error "安装失败。"
        exit 1
    fi

    cd ..
}

# 打包函数：复制文件，处理依赖，创建压缩包
# Package function: Copy files, handle dependencies, create archive
package() {
    log_info "Packaging project..."
    log_info "打包项目..."
    mkdir -p $PACKAGE_DIR
    # 复制安装的文件到打包目录 / Copy installed files to the package directory
    cp -R $INSTALL_DIR/* $PACKAGE_DIR/

    log_info "Copying dependencies..."
    log_info "复制依赖项..."
    # 复制所有可执行文件的MinGW依赖 / Copy MinGW dependencies for all executables
    for file in $PACKAGE_DIR/bin/*.exe; do
        ldd "$file" | grep mingw | awk '{print $3}' | xargs -I '{}' cp '{}' $PACKAGE_DIR/bin/
    done

    log_info "Creating archive..."
    log_info "创建压缩包..."
    # 使用7-Zip创建ZIP压缩包 / Create ZIP archive using 7-Zip
    7z a -tzip $ARCHIVE_NAME $PACKAGE_DIR/*

    log_info "Packaging complete. Archive created: $ARCHIVE_NAME"
    log_info "打包完成。创建的压缩包：$ARCHIVE_NAME"
}

# 主函数：执行完整的构建和打包流程
# Main function: Execute the complete build and package process
main() {
    clean
    build
    package
}

# ===== 脚本入口 / Script Entry =====
# 检查必要的命令是否存在 / Check if necessary commands exist
check_command cmake
check_command mingw32-make
check_command git
check_command 7z

# 根据命令行参数执行相应的功能
# Execute corresponding functionality based on command line arguments
case "$1" in
    clean)
        clean
        ;;
    build)
        build
        ;;
    package)
        package
        ;;
    *)
        main
        ;;
esac