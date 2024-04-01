# 定义 Nginx 路径
$NginxPath = "C:\nginx"
$NginxConfPath = "$NginxPath\conf\nginx.conf"
$NginxBinary = "$NginxPath\nginx.exe"

# 函数: 启动 Nginx
function Start-Nginx {
    if (Test-Path $NginxBinary) {
        & $NginxBinary
        Write-Host "Nginx 已启动" -ForegroundColor Green
    } else {
        Write-Host "无法找到 Nginx 二进制文件" -ForegroundColor Red
    }
}

# 函数: 停止 Nginx
function Stop-Nginx {
    if (Test-Path $NginxBinary) {
        $nginxProcess = Get-Process | Where-Object {$_.Path -eq $NginxBinary}
        if ($nginxProcess) {
            $nginxProcess | Stop-Process -Force
            Write-Host "Nginx 已停止" -ForegroundColor Green
        } else {
            Write-Host "Nginx 未运行" -ForegroundColor Red
        }
    } else {
        Write-Host "无法找到 Nginx 二进制文件" -ForegroundColor Red
    }
}

# 函数: 重新加载 Nginx 配置
function Reload-Nginx {
    if (Test-Path $NginxBinary) {
        $nginxProcess = Get-Process | Where-Object {$_.Path -eq $NginxBinary}
        if ($nginxProcess) {
            $nginxProcess.Refresh()
            Write-Host "Nginx 配置已重新加载" -ForegroundColor Green
        } else {
            Write-Host "Nginx 未运行" -ForegroundColor Red
        }
    } else {
        Write-Host "无法找到 Nginx 二进制文件" -ForegroundColor Red
    }
}

# 函数: 检查 Nginx 配置文件语法
function Test-NginxConfig {
    if (Test-Path $NginxConfPath) {
        & $NginxBinary -t -c $NginxConfPath
    } else {
        Write-Host "无法找到 Nginx 配置文件" -ForegroundColor Red
    }
}

# 函数: 显示帮助信息
function Show-Help {
    Write-Host "Usage: .\nginx.ps1 [start|stop|reload|check|help]"
    Write-Host "  start    启动 Nginx"
    Write-Host "  stop     停止 Nginx"
    Write-Host "  reload   重新加载 Nginx 配置"
    Write-Host "  check    检查 Nginx 配置文件语法"
    Write-Host "  help     显示帮助信息"
}

# 主函数
function Main {
    param($Command)

    switch ($Command) {
        "start" { Start-Nginx }
        "stop" { Stop-Nginx }
        "reload" { Reload-Nginx }
        "check" { Test-NginxConfig }
        "help" { Show-Help }
        default { Write-Host "无效的命令" -ForegroundColor Red; Show-Help }
    }
}

# 执行主函数
Main $args[0]