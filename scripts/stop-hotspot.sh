#!/bin/bash

# Stop dnsmasq and hostapd
sudo systemctl stop dnsmasq
sudo systemctl stop hostapd

# Check if the hotspot is stopped
ip addr show wlan0 | grep inet | awk '{print $2}' | cut -d/ -f1 | if grep -qE '^(192\.168\.4\.[0-9]{1,3})$'; then
    echo "Failed to stop the hotspot"
else
    echo "Hotspot has been stopped"
fi
