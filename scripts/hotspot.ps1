<# 
使用说明

此脚本用于管理Windows上的Wi-Fi热点，包括启动、停止、查看状态和设置配置文件。以下是详细的使用说明：

参数说明
- Action: 指定要执行的操作。有效值包括 "Start", "Stop", "Status", "List", "Set"。这是一个必选参数。
- Name: 热点的名称，默认为 "MyHotspot"。这是一个可选参数。
- Password: 热点的密码，必须是一个SecureString。当启动或设置热点时，这是一个必需参数。
- Authentication: 认证类型，有效值为 "WPA2PSK" 和 "WPA2"，默认为 "WPA2PSK"。这是一个可选参数。
- Encryption: 加密类型，有效值为 "AES" 和 "TKIP"，默认为 "AES"。这是一个可选参数。
- Channel: 无线信道，默认为 11。有效值范围为 1 到 11。 这是一个可选参数。
- MaxClients: 最大连接客户端数量，默认为 10。 这是一个可选参数。

操作说明

启动热点
启动热点并配置相关参数。
powershell.exe -File "HotspotScript.ps1" -Action Start -Password (ConvertTo-SecureString -String 'YourSecurePasswordHere' -AsPlainText -Force)
注意：必须提供热点的密码。

停止热点
停止当前运行的热点。
powershell.exe -File "HotspotScript.ps1" -Action Stop

查看热点状态
查看当前热点的运行状态。
powershell.exe -File "HotspotScript.ps1" -Action Status

列出当前配置
列出当前热点的所有配置信息。
powershell.exe -File "HotspotScript.ps1" -Action List

设置热点配置
配置热点的详细信息。
powershell.exe -File "HotspotScript.ps1" -Action Set -Password (ConvertTo-SecureString -String 'YourSecurePasswordHere' -AsPlainText -Force)
注意：必须提供热点的密码。

热点自动刷新
为了确保热点的持续可用性，脚本会自动创建一个批处理文件并在后台运行。

创建批处理文件
脚本会自动在桌面上创建一个名为 StartMobileHotspot.bat 的批处理文件。该文件包含启动热点的命令并自动刷新。

#>

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

# Define hotspot adapter name
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

    netsh wlan set hostednetwork mode=allow ssid=$Name key=$Password keyUsage=persistent auth=$Authentication cipher=$Encryption channel=$Channel
    netsh wlan start hostednetwork
}

function Stop-Hotspot {
    netsh wlan stop hostednetwork
}

function Get-HotspotStatus {
    netsh wlan show hostednetwork
}

switch ($Action) {
    "Start" {
        if (-not $Password) {
            Write-Error "Password is required when starting a hotspot"
            exit 1
        }

        $plainPassword = [System.Runtime.InteropServices.Marshal]::PtrToStringAuto([System.Runtime.InteropServices.Marshal]::SecureStringToBSTR($Password))

        Start-Hotspot -Name $Name -Password $plainPassword -Authentication $Authentication -Encryption $Encryption -Channel $Channel -MaxClients $MaxClients

        Start-Sleep -Seconds 2
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

        Start-Sleep -Seconds 2
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

        netsh wlan set hostednetwork mode=allow ssid=$Name key=$plainPassword keyUsage=persistent auth=$Authentication cipher=$Encryption channel=$Channel
        Write-Output "Hotspot profile '$Name' has been updated"
    }
}

# Script to auto-refresh and ensure hotspot availability
if ($MyInvocation.InvocationName -eq $PSCommandPath) {
    start mshta vbscript:createobject("wscript.shell").run("""$PSCommandPath"" hide",0)(window.close)&&exit
    goto :CmdBegin
}

:CmdBegin

$CurrentUserPath = [System.Environment]::GetFolderPath('Desktop')
$BatchFilePath = Join-Path $CurrentUserPath "StartMobileHotspot.bat"

# Create or update the batch file
$batchContent = @"
@echo off
powershell.exe -File `"$PSCommandPath`" Start -Password (ConvertTo-SecureString -String 'YourSecurePasswordHere' -AsPlainText -Force)
choice /t 10 /d y /n >nul
goto autoRefreshWeb
:autoRefreshWeb
start "" `"$BatchFilePath`"
choice /t 10 /d y /n >nul
goto autoRefreshWeb
"@

Set-Content -Path $BatchFilePath -Value $batchContent

# Start the batch file for auto-refresh
Start-Process -FilePath $BatchFilePath
