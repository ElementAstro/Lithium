sed -i "s#https\?://mirror.msys2.org/#https://mirrors.tuna.tsinghua.edu.cn/msys2/#g" /etc/pacman.d/mirrorlist*
pacman -Syu
pacman -S mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-dlfcn
pacman -S mingw-w64-x86_64-cfitsio
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-libzip
pacman -S mingw-w64-x86_64-zlib
pacman -S mingw-w64-x86_64-fmt
pacman -S mingw-w64-x86_64-libnova