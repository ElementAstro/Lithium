#!/bin/bash

# Define colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

# Define Nginx paths
NGINX_PATH="/etc/nginx"
NGINX_CONF="$NGINX_PATH/nginx.conf"
NGINX_BINARY="/usr/sbin/nginx"

# Function: Install Nginx if not installed
install_nginx() {
    if ! command -v nginx &>/dev/null; then
        echo "Installing Nginx..."
        # Add installation commands according to the package manager used (apt, yum, etc.)
        # Example for apt:
        sudo apt-get update
        sudo apt-get install nginx -y
    fi
}

# Function: Start Nginx
start_nginx() {
    if [ -f "$NGINX_BINARY" ]; then
        $NGINX_BINARY
        echo -e "${GREEN}Nginx has been started${NC}"
    else
        echo -e "${RED}Nginx binary not found${NC}"
    fi
}

# Function: Stop Nginx
stop_nginx() {
    if [ -f "$NGINX_BINARY" ]; then
        $NGINX_BINARY -s stop
        echo -e "${GREEN}Nginx has been stopped${NC}"
    else
        echo -e "${RED}Nginx binary not found${NC}"
    fi
}

# Function: Reload Nginx configuration
reload_nginx() {
    if [ -f "$NGINX_BINARY" ]; then
        $NGINX_BINARY -s reload
        echo -e "${GREEN}Nginx configuration has been reloaded${NC}"
    else
        echo -e "${RED}Nginx binary not found${NC}"
    fi
}

# Function: Check Nginx configuration syntax
check_config() {
    if [ -f "$NGINX_CONF" ]; then
        $NGINX_BINARY -t -c "$NGINX_CONF"
    else
        echo -e "${RED}Nginx configuration file not found${NC}"
    fi
}

# Function: Show help message
show_help() {
    echo "Usage: $0 [start|stop|reload|check|help]"
    echo "  start    Start Nginx"
    echo "  stop     Stop Nginx"
    echo "  reload   Reload Nginx configuration"
    echo "  check    Check Nginx configuration syntax"
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
        *)
            echo -e "${RED}Invalid command${NC}"
            show_help
            exit 1
            ;;
    esac
}

# Execute main function with the provided argument
main "$1"
