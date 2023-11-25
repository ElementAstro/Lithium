[CmdletBinding()]
Param(
    [Parameter(Position = 0, Mandatory = $true)]
    [string]$Name,

    [Parameter(Position = 1, Mandatory = $true)]
    [string]$Password
)

# 安装NetworkManager模块（如果未安装）
if (-not (Get-Module -ListAvailable NetworkManager)) {
    Install-Module -Name NetworkManager -Scope CurrentUser -Force
}

# 开启热点
$hotspotSetting = @{
    Name = $Name
    Passphrase = $Password
    InternetSharingEnabled = $true
}
New-NetHotspot @hotspotSetting

# 验证热点是否已经开启
$hotspot = Get-NetAdapter | Where-Object { $_.Name -eq "vEthernet (WLAN)" }
if ($hotspot.Status -eq "Up") {
    Write-Output "Hotspot $Name is now running with password $Password"
} else {
    Write-Output "Failed to start hotspot $Name"
}
