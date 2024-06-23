[CmdletBinding()]
Param(
    [Parameter(Position = 0, Mandatory = $true)]
    [ValidateSet("Start", "Stop", "Status", "List", "Set")]
    [string]$Action,

    [Parameter(Position = 1, Mandatory = $false)]
    [string]$Name = "MyHotspot",

    [Parameter(Position = 2, Mandatory = $false)]
    [SecureString]$Password,

    [Parameter(Mandatory = $false)]
    [ValidateSet("WPA2PSK", "WPA2")]
    [string]$Authentication = "WPA2PSK",

    [Parameter(Mandatory = $false)]
    [ValidateSet("AES", "TKIP")]
    [string]$Encryption = "AES",

    [Parameter(Mandatory = $false)]
    [int]$Channel = 11,

    [Parameter(Mandatory = $false)]
    [int]$MaxClients = 10
)

# 定义热点适配器名称
$hotspotAdapterName = "Wi-Fi"

function Start-Hotspot {
    param (
        [string]$Name,
        [string]$Password,
        [string]$Authentication,
        [string]$Encryption,
        [int]$Channel,
        [int]$MaxClients
    )

    netsh wlan set hostednetwork mode=allow ssid=$Name key=$Password
    netsh wlan start hostednetwork
}

function Stop-Hotspot {
    netsh wlan stop hostednetwork
}

function Get-HotspotStatus {
    $status = netsh wlan show hostednetwork
    $status
}

switch ($Action) {
    "Start" {
        if (-not $Password) {
            Write-Error "Password is required when starting a hotspot"
            exit 1
        }

        # 将SecureString类型的密码转换为明文字符串
        $plainPassword = [System.Runtime.InteropServices.Marshal]::PtrToStringAuto([System.Runtime.InteropServices.Marshal]::SecureStringToBSTR($Password))

        # 开启热点
        Start-Hotspot -Name $Name -Password $plainPassword -Authentication $Authentication -Encryption $Encryption -Channel $Channel -MaxClients $MaxClients

        # 验证热点是否已经开启
        $status = Get-HotspotStatus
        if ($status -match "Status\s*:\s*Started") {
            Write-Output "Hotspot $Name is now running with password $plainPassword"
        } else {
            Write-Error "Failed to start hotspot $Name"
            exit 1
        }
    }

    "Stop" {
        Stop-Hotspot

        $status = Get-HotspotStatus
        if ($status -match "Status\s*:\s*Not started") {
            Write-Output "Hotspot has been stopped"
        } else {
            Write-Error "Failed to stop hotspot"
            exit 1
        }
    }

    "Status" {
        $status = Get-HotspotStatus
        if ($status -match "Status\s*:\s*Started") {
            Write-Output "Hotspot is running"
            Write-Output "Hotspot settings:"
            Write-Output $status
        } else {
            Write-Output "Hotspot is not running"
        }
    }

    "List" {
        Write-Output "Current hosted network configuration:"
        $status = Get-HotspotStatus
        Write-Output $status
    }

    "Set" {
        if (-not $Password) {
            Write-Error "Password is required when setting a hotspot profile"
            exit 1
        }

        $plainPassword = [System.Runtime.InteropServices.Marshal]::PtrToStringAuto([System.Runtime.InteropServices.Marshal]::SecureStringToBSTR($Password))

        netsh wlan set hostednetwork mode=allow ssid=$Name key=$plainPassword
        Write-Output "Hotspot profile '$Name' has been updated"
    }
}