#!/bin/bash

# Set default values
SSID="LithiumServer"
PASSWORD="lithiumserver"

# Process arguments
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -s|--ssid)
    SSID="$2"
    shift # past argument
    shift # past value
    ;;
    -p|--password)
    PASSWORD="$2"
    shift # past argument
    shift # past value
    ;;
    *)    # unknown option
    echo "Unknown option: $1"
    exit 1
    ;;
esac
done

# Check if hostapd and dnsmasq are installed
if ! dpkg -s hostapd dnsmasq > /dev/null 2>&1; then
    echo "Installing hostapd and dnsmasq..."
    sudo apt-get update
    sudo apt-get install hostapd dnsmasq -y
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
sudo systemctl start dnsmasq
sudo systemctl start hostapd

# Check if the hotspot is started
ip addr show wlan0 | grep inet | awk '{print $2}' | cut -d/ -f1 | if grep -qE '^(192\.168\.4\.[0-9]{1,3})$'; then
    echo "Hotspot $SSID has been started with password $PASSWORD"
else
    echo "Failed to start the hotspot"
fi
