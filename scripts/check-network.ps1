[CmdletBinding()]
Param(
    [Parameter(Position = 0, Mandatory = $true)]
    [string]$ComputerName
)

# 获取网络连接状态
$networkStatus = Test-NetConnection -ComputerName $ComputerName

# 检查网络连接结果
if ($networkStatus.PingSucceeded -eq $true) {
    Write-Output "Internet Connection Success"
} else {
    Write-Output "Failed to Connect to the Internet"
}
