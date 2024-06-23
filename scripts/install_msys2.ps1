param(
    [Parameter(Mandatory=$false)]
    [string]$InstallPath = "C:\msys64",
    
    [Parameter(Mandatory=$false)]
    [string]$Msys2Url = "https://github.com/msys2/msys2-installer/releases/download/2022-06-03/msys2-x86_64-20220603.exe",
    
    [Parameter(Mandatory=$false)]
    [string]$Mirror1 = "https://mirrors.tuna.tsinghua.edu.cn/msys2/mingw/x86_64/",
    
    [Parameter(Mandatory=$false)]
    [string]$Mirror2 = "https://mirrors.tuna.tsinghua.edu.cn/msys2/mingw/i686/",
    
    [Parameter(Mandatory=$false)]
    [string]$Mirror3 = "https://mirrors.tuna.tsinghua.edu.cn/msys2/msys/\$arch/",
    
    [Parameter(Mandatory=$false)]
    [string[]]$Packages = @("mingw-w64-x86_64-toolchain", "mingw-w64-x86_64-dlfcn", "mingw-w64-x86_64-cfitsio", "mingw-w64-x86_64-cmake", "mingw-w64-x86_64-libzip", "mingw-w64-x86_64-zlib", "mingw-w64-x86_64-fmt", "mingw-w64-x86_64-libnova")
)

# 设置MSYS2的安装路径和下载URL
$msys2_installer = [System.IO.Path]::GetFileName($Msys2Url)

# 下载MSYS2安装程序
Write-Output "Downloading MSYS2 installer from $Msys2Url..."
Invoke-WebRequest -Uri $Msys2Url -OutFile $msys2_installer

# 安装MSYS2
Write-Output "Installing MSYS2 to $InstallPath..."
Start-Process -FilePath .\$msys2_installer -ArgumentList "/S /D=$InstallPath" -Wait

# 配置pacman
Write-Output "Configuring pacman with custom mirrors..."
$pacman_conf = Join-Path -Path $InstallPath -ChildPath "etc\pacman.conf"
Add-Content -Path $pacman_conf -Value "Server = $Mirror1"
Add-Content -Path $pacman_conf -Value "Server = $Mirror2"
Add-Content -Path $pacman_conf -Value "Server = $Mirror3"

# 更新系统包
Write-Output "Updating system packages..."
& "$InstallPath\usr\bin\bash.exe" -lc "pacman -Syu --noconfirm"

# 安装必要的开发工具和库
Write-Output "Installing development tools and libraries..."
$packageList = [string]::Join(" ", $Packages)
& "$InstallPath\usr\bin\bash.exe" -lc "pacman -S --noconfirm $packageList"

# 添加 MSYS2 路径到系统环境变量
Write-Output "Adding MSYS2 to system PATH..."
$envPath = [System.Environment]::GetEnvironmentVariable("Path", [System.EnvironmentVariableTarget]::Machine)
if (-not $envPath.Contains($InstallPath)) {
    $newEnvPath = "$envPath;$InstallPath\usr\bin;$InstallPath\mingw64\bin;$InstallPath\mingw32\bin"
    [System.Environment]::SetEnvironmentVariable("Path", $newEnvPath, [System.EnvironmentVariableTarget]::Machine)
    Write-Output "MSYS2 paths added to system PATH."
} else {
    Write-Output "MSYS2 paths already exist in system PATH."
}

# 重新启动 MSYS2
Write-Output "Restarting MSYS2..."
& "$InstallPath\usr\bin\bash.exe" -lc "exit"

Write-Output "MSYS2 installation and setup completed."