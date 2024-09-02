# Function to lock the Administrator account
function Lock-AdministratorAccount {
    Write-Host "Locking the Administrator account..."
    Disable-LocalUser -Name "Administrator"
    if ($?) {
        Write-Host "Administrator account locked successfully."
    } else {
        Write-Host "Failed to lock Administrator account."
    }
}

# Function to unlock the Administrator account
function Unlock-AdministratorAccount {
    Write-Host "Unlocking the Administrator account..."
    Enable-LocalUser -Name "Administrator"
    if ($?) {
        Write-Host "Administrator account unlocked successfully."
    } else {
        Write-Host "Failed to unlock Administrator account."
    }
}

# Function to lock a specific drive (set it to read-only)
function Lock-Drive {
    param (
        [string]$DriveLetter
    )
    Write-Host "Locking the drive $DriveLetter..."
    $disk = Get-WmiObject -Query "Select * from Win32_LogicalDisk Where DeviceID='$DriveLetter:'"
    if ($disk) {
        $disk.VolumeName = "Read-Only"
        $disk.Put()
        Write-Host "Drive $DriveLetter locked successfully."
    } else {
        Write-Host "Failed to lock drive $DriveLetter."
    }
}

# Function to unlock a specific drive (set it to read-write)
function Unlock-Drive {
    param (
        [string]$DriveLetter
    )
    Write-Host "Unlocking the drive $DriveLetter..."
    $disk = Get-WmiObject -Query "Select * from Win32_LogicalDisk Where DeviceID='$DriveLetter:'"
    if ($disk) {
        $disk.VolumeName = "Read-Write"
        $disk.Put()
        Write-Host "Drive $DriveLetter unlocked successfully."
    } else {
        Write-Host "Failed to unlock drive $DriveLetter."
    }
}

# Function to setup firewall rules (e.g., block all incoming traffic)
function Setup-Firewall {
    Write-Host "Setting up firewall to block all incoming traffic..."
    New-NetFirewallRule -DisplayName "Block All Inbound" -Direction Inbound -Action Block
    Write-Host "Firewall setup complete."
}

# Function to disable USB ports (blocking new USB devices)
function Disable-USB {
    Write-Host "Disabling USB ports..."
    Get-PnpDevice | Where-Object { $_.Class -eq "USB" } | Disable-PnpDevice -Confirm:$false
    if ($?) {
        Write-Host "USB ports disabled successfully."
    } else {
        Write-Host "Failed to disable USB ports."
    }
}

# Function to enable USB ports (allowing new USB devices)
function Enable-USB {
    Write-Host "Enabling USB ports..."
    Get-PnpDevice | Where-Object { $_.Class -eq "USB" } | Enable-PnpDevice -Confirm:$false
    if ($?) {
        Write-Host "USB ports enabled successfully."
    } else {
        Write-Host "Failed to enable USB ports."
    }
}

# Function to close specific ports
function Close-Ports {
    param (
        [string]$Ports
    )
    Write-Host "Closing ports: $Ports..."
    $portsArray = $Ports.Split(',')
    foreach ($port in $portsArray) {
        New-NetFirewallRule -DisplayName "Block Port $port" -Direction Inbound -LocalPort $port -Protocol TCP -Action Block
        Write-Host "Port $port closed."
    }
}

# Function to setup RDP protection (similar to SSH protection)
function Setup-RDPProtection {
    Write-Host "Setting up RDP protection..."

    # Allow RDP access only from specific IPs
    $allowedIPs = "192.168.1.100"
    Write-Host "Allowing RDP access only from: $allowedIPs"
    Set-NetFirewallRule -DisplayName "Remote Desktop - User Mode (TCP-In)" -RemoteAddress $allowedIPs

    # Change RDP port (optional)
    $newRDPPort = 2222
    Write-Host "Changing RDP port to $newRDPPort"
    Set-ItemProperty -Path "HKLM:\System\CurrentControlSet\Control\Terminal Server\WinStations\RDP-Tcp" -Name "PortNumber" -Value $newRDPPort

    # Restart RDP service to apply changes
    Restart-Service -Name "TermService"
    Write-Host "RDP protection setup complete."
}

# Function to monitor event logs for suspicious activity
function Monitor-EventLogs {
    Write-Host "Monitoring Security event log for suspicious activity..."
    Get-WinEvent -LogName "Security" -FilterXPath "*[System[(EventID=4625)]]" | Select-Object -First 10 | Format-List
}

# Function to check system integrity using SFC and DISM
function Check-SystemIntegrity {
    Write-Host "Checking system integrity..."
    sfc /scannow
    dism /online /cleanup-image /checkhealth
    dism /online /cleanup-image /scanhealth
    dism /online /cleanup-image /restorehealth
}

# Function to setup automatic logout
function Setup-AutoLogout {
    Write-Host "Setting up automatic logout for idle users..."
    $timeout = 300
    Write-Host "Setting timeout to $timeout seconds..."
    New-ItemProperty -Path 'HKCU:\Control Panel\Desktop' -Name ScreenSaveTimeOut -Value $timeout -PropertyType String -Force
    New-ItemProperty -Path 'HKCU:\Control Panel\Desktop' -Name ScreenSaverIsSecure -Value 1 -PropertyType String -Force
    Write-Host "Automatic logout setup complete."
}

# Function to verify security settings
function Verify-Security {
    Write-Host "Verifying security settings..."

    # Check if Administrator account is locked
    $adminStatus = Get-LocalUser -Name "Administrator" | Select-Object -ExpandProperty Enabled
    if (-not $adminStatus) {
        Write-Host "Administrator account is locked."
    } else {
        Write-Host "Administrator account is not locked."
    }

    # Check if firewall is configured
    $firewallRules = Get-NetFirewallRule -DisplayName "Block All Inbound"
    if ($firewallRules) {
        Write-Host "Firewall is configured to block incoming traffic."
    } else {
        Write-Host "Firewall is not blocking incoming traffic."
    }

    # Check if USB ports are disabled
    $usbStatus = Get-PnpDevice | Where-Object { $_.Class -eq "USB" } | Select-Object -ExpandProperty Status
    if ($usbStatus -contains "Disabled") {
        Write-Host "USB ports are disabled."
    } else {
        Write-Host "USB ports are not disabled."
    }

    # Check if RDP port is changed
    $rdpPort = Get-ItemProperty -Path "HKLM:\System\CurrentControlSet\Control\Terminal Server\WinStations\RDP-Tcp" -Name "PortNumber" | Select-Object -ExpandProperty PortNumber
    if ($rdpPort -eq 2222) {
        Write-Host "RDP is running on custom port $rdpPort."
    } else {
        Write-Host "RDP is running on default port."
    }
}

# Main script
param (
    [string]$Action,
    [string]$Parameter
)

switch ($Action) {
    "lock-admin" { Lock-AdministratorAccount }
    "unlock-admin" { Unlock-AdministratorAccount }
    "lock-drive" { Lock-Drive -DriveLetter $Parameter }
    "unlock-drive" { Unlock-Drive -DriveLetter $Parameter }
    "setup-firewall" { Setup-Firewall }
    "close-ports" { Close-Ports -Ports $Parameter }
    "disable-usb" { Disable-USB }
    "enable-usb" { Enable-USB }
    "setup-rdp-protection" { Setup-RDPProtection }
    "monitor-logs" { Monitor-EventLogs }
    "check-integrity" { Check-SystemIntegrity }
    "setup-auto-logout" { Setup-AutoLogout }
    "verify-security" { Verify-Security }
    default { Write-Host "Usage: script.ps1 -Action {lock-admin|unlock-admin|lock-drive|unlock-drive|setup-firewall|close-ports|disable-usb|enable-usb|setup-rdp-protection|monitor-logs|check-integrity|setup-auto-logout|verify-security} -Parameter {optional}" }
}
