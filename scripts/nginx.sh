#!/bin/bash

# 定义颜色
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

# 定义 Nginx 路径
NGINX_PATH="/etc/nginx"
NGINX_CONF="$NGINX_PATH/nginx.conf"
NGINX_BINARY="/usr/sbin/nginx"

# 函数: 启动 Nginx
start_nginx() {
    if [ -f "$NGINX_BINARY" ]; then
        $NGINX_BINARY
        echo -e "${GREEN}Nginx 已启动${NC}"
    else
        echo -e "${RED}无法找到 Nginx 二进制文件${NC}"
    fi
}

# 函数: 停止 Nginx
stop_nginx() {
    if [ -f "$NGINX_BINARY" ]; then
        $NGINX_BINARY -s stop
        echo -e "${GREEN}Nginx 已停止${NC}"
    else
        echo -e "${RED}无法找到 Nginx 二进制文件${NC}"
    fi
}

# 函数: 重新加载 Nginx 配置
reload_nginx() {
    if [ -f "$NGINX_BINARY" ]; then
        $NGINX_BINARY -s reload
        echo -e "${GREEN}Nginx 配置已重新加载${NC}"
    else
        echo -e "${RED}无法找到 Nginx 二进制文件${NC}"
    fi
}

# 函数: 检查 Nginx 配置文件语法
check_config() {
    if [ -f "$NGINX_CONF" ]; then
        $NGINX_BINARY -t -c "$NGINX_CONF"
    else
        echo -e "${RED}无法找到 Nginx 配置文件${NC}"
    fi
}

# 函数: 显示帮助信息
show_help() {
    echo "Usage: $0 [start|stop|reload|check|help]"
    echo "  start    启动 Nginx"
    echo "  stop     停止 Nginx"
    echo "  reload   重新加载 Nginx 配置"
    echo "  check    检查 Nginx 配置文件语法"
    echo "  help     显示帮助信息"
}

# 主函数
main() {
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
        help)
            show_help
            ;;
        *)
            echo -e "${RED}无效的命令${NC}"
            show_help
            ;;
    esac
}

# 执行主函数
main "$1"