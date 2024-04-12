#!/bin/bash

# Set default values
SSID="LithiumServer"
PASSWORD="lithiumserver"
PACKAGE_MANAGER=""

# Function to detect package manager
detect_package_manager() {
    if command -v apt-get &>/dev/null; then
        PACKAGE_MANAGER="apt"
    elif command -v yum &>/dev/null; then
        PACKAGE_MANAGER="yum"
    elif command -v dnf &>/dev/null; then
        PACKAGE_MANAGER="dnf"
    else
        echo "Unsupported package manager. Please install 'hostapd' and 'dnsmasq' manually."
        exit 1
    fi
}

# Function to install packages
install_packages() {
    case $PACKAGE_MANAGER in
        apt)
            sudo apt-get update
            sudo apt-get install hostapd dnsmasq -y
            ;;
        yum|dnf)
            sudo $PACKAGE_MANAGER install hostapd dnsmasq -y
            ;;
        *)
            echo "Unsupported package manager."
            exit 1
            ;;
    esac
}

# Function to start services
start_services() {
    sudo systemctl start dnsmasq
    sudo systemctl start hostapd
}

# Function to check if services are running
check_services() {
    if ! systemctl is-active --quiet dnsmasq || ! systemctl is-active --quiet hostapd; then
        echo "Failed to start the hotspot."
        exit 1
    fi
}

# Main script

# Process arguments
while [[ $# -gt 0 ]]; do
    key="$1"

    case $key in
        -s|--ssid)
            SSID="$2"
            shift 2
            ;;
        -p|--password)
            PASSWORD="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Detect package manager
detect_package_manager

# Check if hostapd and dnsmasq are installed
if ! dpkg -s hostapd dnsmasq > /dev/null 2>&1; then
    echo "Installing hostapd and dnsmasq..."
    install_packages
fi

# Configure dnsmasq
echo "interface=wlan0
dhcp-range=192.168.4.2,192.168.4.20,255.255.255.0,24h" | sudo tee /etc/dnsmasq.conf > /dev/null

# Configure hostapd
echo "interface=wlan0
driver=nl80211
ssid=$SSID
hw_mode=g
channel=6
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=$PASSWORD
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP" | sudo tee /etc/hostapd/hostapd.conf > /dev/null

# Start dnsmasq and hostapd
start_services

# Check if the hotspot is started
check_services

echo "Hotspot $SSID has been started with password $PASSWORD"
