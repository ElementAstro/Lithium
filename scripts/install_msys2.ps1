# 设置MSYS2的下载URL和安装路径
$msys2_url = "https://github.com/msys2/msys2-installer/releases/download/2022-06-03/msys2-x86_64-20220603.exe"
$msys2_installer = "msys2-x86_64-20220603.exe"
$install_dir = "C:\msys64"

# 下载MSYS2安装程序
Write-Output "Downloading MSYS2 installer..."
Invoke-WebRequest -Uri $msys2_url -OutFile $msys2_installer

# 安装MSYS2
Write-Output "Installing MSYS2..."
Start-Process -FilePath .\$msys2_installer -ArgumentList "/S /D=$install_dir" -Wait

# 配置pacman
Write-Output "Configuring pacman..."
$pacman_conf = "C:\msys64\etc\pacman.conf"
Add-Content -Path $pacman_conf -Value 'Server = https://mirrors.tuna.tsinghua.edu.cn/msys2/mingw/x86_64/'
Add-Content -Path $pacman_conf -Value 'Server = https://mirrors.tuna.tsinghua.edu.cn/msys2/mingw/i686/'
Add-Content -Path $pacman_conf -Value 'Server = https://mirrors.tuna.tsinghua.edu.cn/msys2/msys/$arch/'

# 更新系统包
Write-Output "Updating system packages..."
C:\msys64\usr\bin\bash.exe -lc "pacman -Syu --noconfirm"

# 安装必要的开发工具和库
Write-Output "Installing development tools and libraries..."
C:\msys64\usr\bin\bash.exe -lc "pacman -S --noconfirm mingw-w64-x86_64-toolchain mingw-w64-x86_64-dlfcn mingw-w64-x86_64-cfitsio mingw-w64-x86_64-cmake mingw-w64-x86_64-libzip mingw-w64-x86_64-zlib mingw-w64-x86_64-fmt mingw-w64-x86_64-libnova"

Write-Output "MSYS2 installation and setup completed."