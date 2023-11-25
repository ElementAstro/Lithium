# 停止热点
Stop-NetAdapter -Name "vEthernet (WLAN)"

# 验证热点是否已经停止
$hotspot = Get-NetAdapter | Where-Object { $_.Name -eq "vEthernet (WLAN)" }
if ($hotspot.Status -ne "Up") {
    Write-Output "Hotspot has been stopped"
} else {
    Write-Output "Failed to stop hotspot"
}
