#!/bin/bash

# Nginx 管理脚本
# 用途: 安装、启动、停止、重载、检查Nginx配置，并显示Nginx状态
#
# 使用方法:
# ./nginx_manager.sh [start|stop|reload|check|status|help]
#
# 参数说明:
#   start    - 启动Nginx
#   stop     - 停止Nginx
#   reload   - 重载Nginx配置
#   check    - 检查Nginx配置语法
#   status   - 显示Nginx运行状态
#   help     - 显示此帮助信息
#
# 脚本功能:
# 1. 自动检测并安装Nginx（支持Debian和Red Hat系统）
# 2. 启动、停止和重载Nginx服务
# 3. 检查Nginx配置文件的语法
# 4. 显示Nginx的当前运行状态
#
# 示例:
# ./nginx_manager.sh start
# ./nginx_manager.sh stop
# ./nginx_manager.sh reload
# ./nginx_manager.sh check
# ./nginx_manager.sh status
# ./nginx_manager.sh help

# Define colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'  # No Color

# Define Nginx paths
NGINX_PATH="/etc/nginx"
NGINX_CONF="$NGINX_PATH/nginx.conf"
NGINX_BINARY="/usr/sbin/nginx"

# Function: Install Nginx if not installed
install_nginx() {
    if ! command -v nginx &>/dev/null; then
        echo "Installing Nginx..."
        if [ -f /etc/debian_version ]; then
            # Debian-based system (e.g., Ubuntu)
            sudo apt-get update
            sudo apt-get install nginx -y
        elif [ -f /etc/redhat-release ]; then
            # Red Hat-based system (e.g., CentOS)
            sudo yum update
            sudo yum install nginx -y
        else
            echo -e "${RED}Unsupported platform. Please install Nginx manually.${NC}"
            exit 1
        fi
    fi
}

# Function: Start Nginx
start_nginx() {
    if [ -f "$NGINX_BINARY" ]; then
        sudo $NGINX_BINARY
        echo -e "${GREEN}Nginx has been started${NC}"
    else
        echo -e "${RED}Nginx binary not found${NC}"
    fi
}

# Function: Stop Nginx
stop_nginx() {
    if [ -f "$NGINX_BINARY" ]; then
        sudo $NGINX_BINARY -s stop
        echo -e "${GREEN}Nginx has been stopped${NC}"
    else
        echo -e "${RED}Nginx binary not found${NC}"
    fi
}

# Function: Reload Nginx configuration
reload_nginx() {
    if [ -f "$NGINX_BINARY" ]; then
        sudo $NGINX_BINARY -s reload
        echo -e "${GREEN}Nginx configuration has been reloaded${NC}"
    else
        echo -e "${RED}Nginx binary not found${NC}"
    fi
}

# Function: Check Nginx configuration syntax
check_config() {
    if [ -f "$NGINX_CONF" ]; then
        sudo $NGINX_BINARY -t -c "$NGINX_CONF"
    else
        echo -e "${RED}Nginx configuration file not found${NC}"
    fi
}

# Function: Show Nginx status
status_nginx() {
    if pgrep nginx &>/dev/null; then
        echo -e "${GREEN}Nginx is running${NC}"
    else
        echo -e "${RED}Nginx is not running${NC}"
    fi
}

# Function: Show help message
show_help() {
    echo "Usage: $0 [start|stop|reload|check|status|help]"
    echo "  start    Start Nginx"
    echo "  stop     Stop Nginx"
    echo "  reload   Reload Nginx configuration"
    echo "  check    Check Nginx configuration syntax"
    echo "  status   Show Nginx status"
    echo "  help     Show help message"
}

# Main function
main() {
    if [ "$1" == "help" ]; then
        show_help
        exit 0
    fi

    # Check if Nginx is installed
    install_nginx

    # Execute the command
    case "$1" in
        start)
            start_nginx
            ;;
        stop)
            stop_nginx
            ;;
        reload)
            reload_nginx
            ;;
        check)
            check_config
            ;;
        status)
            status_nginx
            ;;
        *)
            echo -e "${RED}Invalid command${NC}"
            show_help
            exit 1
            ;;
    esac
}

# Execute main function with the provided argument
main "$1"
