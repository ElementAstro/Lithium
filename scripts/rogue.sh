#!/bin/bash

# Function to lock the root account
lock_root() {
    echo "Locking the root account..."
    passwd -l root
    if [[ $? -eq 0 ]]; then
        echo "Root account locked successfully."
    else
        echo "Failed to lock root account."
    fi
}

# Function to unlock the root account
unlock_root() {
    echo "Unlocking the root account..."
    passwd -u root
    if [[ $? -eq 0 ]]; then
        echo "Root account unlocked successfully."
    else
        echo "Failed to unlock root account."
    fi
}

# Function to lock a specific device (e.g., /dev/sda)
lock_device() {
    DEVICE=$1
    if [ -z "$DEVICE" ]; then
        echo "No device specified. Exiting."
        exit 1
    fi

    echo "Locking the device $DEVICE..."
    blockdev --setro $DEVICE
    if [[ $? -eq 0 ]]; then
        echo "Device $DEVICE locked successfully."
    else
        echo "Failed to lock device $DEVICE."
    fi
}

# Function to unlock a specific device
unlock_device() {
    DEVICE=$1
    if [ -z "$DEVICE" ]; then
        echo "No device specified. Exiting."
        exit 1
    fi

    echo "Unlocking the device $DEVICE..."
    blockdev --setrw $DEVICE
    if [[ $? -eq 0 ]]; then
        echo "Device $DEVICE unlocked successfully."
    else
        echo "Failed to unlock device $DEVICE."
    fi
}

# Function to set up a firewall rule (e.g., block all incoming traffic)
setup_firewall() {
    echo "Setting up firewall to block all incoming traffic..."
    iptables -P INPUT DROP
    iptables -P FORWARD DROP
    iptables -P OUTPUT ACCEPT
    echo "Firewall setup complete."
}

# Function to disable USB ports (blocking new USB devices)
disable_usb() {
    echo "Disabling USB ports..."
    echo "1" > /sys/bus/usb/devices/usb*/authorized
    if [[ $? -eq 0 ]]; then
        echo "USB ports disabled successfully."
    else
        echo "Failed to disable USB ports."
    fi
}

# Function to enable USB ports (allowing new USB devices)
enable_usb() {
    echo "Enabling USB ports..."
    echo "0" > /sys/bus/usb/devices/usb*/authorized
    if [[ $? -eq 0 ]]; then
        echo "USB ports enabled successfully."
    else
        echo "Failed to enable USB ports."
    fi
}

# Function to close specific ports
close_ports() {
    PORTS=$1
    if [ -z "$PORTS" ]; then
        echo "No ports specified. Exiting."
        exit 1
    fi

    echo "Closing ports: $PORTS..."
    for PORT in $(echo $PORTS | tr "," "\n"); do
        iptables -A INPUT -p tcp --dport $PORT -j REJECT
        echo "Port $PORT closed."
    done
}

# Function to setup SSH protection
setup_ssh_protection() {
    SSH_CONFIG="/etc/ssh/sshd_config"
    echo "Setting up SSH protection..."

    # Restrict SSH access to specific IPs (edit this list)
    ALLOWED_IPS="192.168.1.100"
    echo "Allowing SSH access only from: $ALLOWED_IPS"
    echo "AllowUsers root@$ALLOWED_IPS" >> $SSH_CONFIG

    # Change SSH port (optional)
    NEW_SSH_PORT=2222
    echo "Changing SSH port to $NEW_SSH_PORT"
    sed -i "s/#Port 22/Port $NEW_SSH_PORT/" $SSH_CONFIG

    # Restart SSH service to apply changes
    systemctl restart sshd
    echo "SSH protection setup complete."
}

# Function to monitor logs for suspicious activity
monitor_logs() {
    LOG_FILE="/var/log/auth.log"
    echo "Monitoring $LOG_FILE for suspicious activity..."
    tail -f $LOG_FILE | grep --line-buffered "Failed password\|error"
}

# Function to check system integrity using AIDE or Tripwire
check_integrity() {
    echo "Checking system integrity..."
    if command -v aide >/dev/null 2>&1; then
        aide --check
    elif command -v tripwire >/dev/null 2>&1; then
        tripwire --check
    else
        echo "No integrity checking tool installed. Please install AIDE or Tripwire."
    fi
}

# Function to set up automatic logout
setup_auto_logout() {
    echo "Setting up automatic logout for idle users..."
    TMOUT=300
    echo "export TMOUT=$TMOUT" >> /etc/profile
    echo "Automatic logout set for $TMOUT seconds of inactivity."
}

# Function to verify system security settings
verify_security() {
    echo "Verifying security settings..."

    # Verify root account lock
    ROOT_LOCKED=$(passwd -S root | awk '{print $2}')
    if [ "$ROOT_LOCKED" == "L" ]; then
        echo "Root account is locked."
    else
        echo "Root account is not locked."
    fi

    # Verify device lock
    DEVICE=$1
    RO_STATUS=$(blockdev --getro $DEVICE)
    if [ "$RO_STATUS" -eq 1 ]; then
        echo "Device $DEVICE is read-only."
    else
        echo "Device $DEVICE is not read-only."
    fi

    # Verify firewall setup
    FW_POLICY=$(iptables -L INPUT -v -n | grep "policy DROP")
    if [ ! -z "$FW_POLICY" ]; then
        echo "Firewall is configured to block incoming traffic."
    else
        echo "Firewall is not blocking incoming traffic."
    fi

    # Verify USB port status
    USB_STATUS=$(cat /sys/bus/usb/devices/usb*/authorized)
    if [ "$USB_STATUS" -eq 1 ]; then
        echo "USB ports are disabled."
    else
        echo "USB ports are not disabled."
    fi

    # Verify SSH protection
    SSH_PORT=$(grep "^Port " /etc/ssh/sshd_config | awk '{print $2}')
    if [ "$SSH_PORT" -eq 2222 ]; then
        echo "SSH is running on custom port $SSH_PORT."
    else
        echo "SSH is running on default port 22."
    fi
}

# Main script
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root."
    exit 1
fi

# Command line options
case $1 in
    lock-root)
        lock_root
        ;;
    unlock-root)
        unlock_root
        ;;
    lock-device)
        lock_device $2
        ;;
    unlock-device)
        unlock_device $2
        ;;
    setup-firewall)
        setup_firewall
        ;;
    close-ports)
        close_ports $2
        ;;
    disable-usb)
        disable_usb
        ;;
    enable-usb)
        enable_usb
        ;;
    setup-ssh-protection)
        setup_ssh_protection
        ;;
    monitor-logs)
        monitor_logs
        ;;
    check-integrity)
        check_integrity
        ;;
    setup-auto-logout)
        setup_auto_logout
        ;;
    verify-security)
        verify_security $2
        ;;
    *)
        echo "Usage: $0 {lock-root|unlock-root|lock-device|unlock-device|setup-firewall|close-ports|disable-usb|enable-usb|setup-ssh-protection|monitor-logs|check-integrity|setup-auto-logout|verify-security} [device|ports]"
        exit 1
        ;;
esac
