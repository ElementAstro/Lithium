#!/bin/bash

# 定义颜色代码
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 检查是否为root用户
if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}Please run as root.${NC}"
    exit 1
fi

# 安装必要的依赖
echo -e "${BLUE}Installing dependencies...${NC}"
apt-get update
apt-get install -y git cmake libcfitsio-dev libcfitsio8 zlib1g-dev libgsl-dev libnova-dev libusb-1.0-0-dev libcurl4-gnutls-dev libboost-dev libboost-regex-dev

# 克隆INDI库
echo -e "${BLUE}正在克隆INDI库...${NC}"
git clone https://github.com/indilib/indi.git

# 进入INDI目录
cd indi

# 创建构建目录
mkdir -p build
cd build

# 运行CMake配置
echo -e "${BLUE}CMake...${NC}"
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ..

make -j$(nproc)
make install

# 安装成功提示
echo -e "${GREEN}INDI installation completed successfully!${NC}"
