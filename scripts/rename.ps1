<#
详细的使用方法

此脚本用于在指定目录下对文件和目录执行多种操作，包括重命名、列出、复制、删除和移动。以下是详细的使用说明：

参数说明：
- RootDirectory: 指定要操作的根目录路径（必需参数）。
- OldElement: 在重命名操作中，要替换的旧元素字符串（必需参数）。
- NewElement: 在重命名操作中，用于替换旧元素的新字符串（必需参数）。
- Action: 要执行的操作类型，有效值包括 "Rename", "List", "Copy", "Delete", "Move"（必需参数）。
- DestinationDirectory: 目标目录路径，仅在执行Copy和Move操作时必需。

操作说明：

1. 重命名文件（Rename）：
    - 将指定目录及其子目录中的所有文件名中的旧元素字符串替换为新元素字符串。
    示例：
    ```powershell
    ./YourScript.ps1 -RootDirectory "C:\YourDirectory" -OldElement "old" -NewElement "new" -Action "Rename"
    ```

2. 列出文件（List）：
    - 列出指定目录及其子目录中的所有文件和目录。
    示例：
    ```powershell
    ./YourScript.ps1 -RootDirectory "C:\YourDirectory" -Action "List"
    ```

3. 复制文件（Copy）：
    - 递归复制指定目录及其子目录中的所有文件和目录到目标目录。
    示例：
    ```powershell
    ./YourScript.ps1 -RootDirectory "C:\YourDirectory" -Action "Copy" -DestinationDirectory "D:\BackupDirectory"
    ```

4. 删除文件（Delete）：
    - 递归删除指定目录及其子目录中的所有文件和目录。
    示例：
    ```powershell
    ./YourScript.ps1 -RootDirectory "C:\YourDirectory" -Action "Delete"
    ```

5. 移动文件（Move）：
    - 递归移动指定目录及其子目录中的所有文件和目录到目标目录。
    示例：
    ```powershell
    ./YourScript.ps1 -RootDirectory "C:\YourDirectory" -Action "Move" -DestinationDirectory "D:\NewLocation"
    ```

注意事项：
- 在执行Copy和Move操作时，必须提供DestinationDirectory参数。
- 在执行重命名操作时，确保OldElement和NewElement参数正确，以免误操作。

示例命令：
```powershell
# 重命名操作
./YourScript.ps1 -RootDirectory "C:\YourDirectory" -OldElement "old" -NewElement "new" -Action "Rename"

# 列出文件
./YourScript.ps1 -RootDirectory "C:\YourDirectory" -Action "List"

# 复制文件
./YourScript.ps1 -RootDirectory "C:\YourDirectory" -Action "Copy" -DestinationDirectory "D:\BackupDirectory"

# 删除文件
./YourScript.ps1 -RootDirectory "C:\YourDirectory" -Action "Delete"

# 移动文件
./YourScript.ps1 -RootDirectory "C:\YourDirectory" -Action "Move" -DestinationDirectory "D:\NewLocation"
```
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$RootDirectory,               # 根目录，必需参数
    [Parameter(Mandatory=$true)]
    [string]$OldElement,                  # 要替换的旧元素字符串，必需参数
    [Parameter(Mandatory=$true)]
    [string]$NewElement,                  # 新元素字符串，必需参数
    [Parameter(Mandatory=$true)]
    [ValidateSet("Rename", "List", "Copy", "Delete", "Move")]
    [string]$Action,                      # 操作类型（Rename, List, Copy, Delete, Move），必需参数
    [string]$DestinationDirectory         # 目标目录，Copy和Move操作时必需
)

# 函数：重命名文件
function Rename-Files($Path) {
    $files = Get-ChildItem -Path $Path -File

    foreach ($file in $files) {
        $newFileName = $file.Name.Replace($OldElement, $NewElement)
        $newFilePath = Join-Path -Path $file.Directory.FullName -ChildPath $newFileName
        Rename-Item -Path $file.FullName -NewName $newFilePath -ErrorAction SilentlyContinue
    }

    $directories = Get-ChildItem -Path $Path -Directory
    foreach ($directory in $directories) {
        Rename-Files -Path $directory.FullName
    }
}

# 函数：列出文件
function List-Files($Path) {
    Get-ChildItem -Path $Path -Recurse
}

# 函数：复制文件
function Copy-Files($SourcePath, $DestinationPath) {
    Copy-Item -Path $SourcePath -Destination $DestinationPath -Recurse -Force
}

# 函数：删除文件
function Delete-Files($Path) {
    Remove-Item -Path $Path -Recurse -Force
}

# 函数：移动文件
function Move-Files($SourcePath, $DestinationPath) {
    Move-Item -Path $SourcePath -Destination $DestinationPath -Recurse -Force
}

# 主逻辑，根据Action参数执行相应操作
switch ($Action) {
    "Rename" {
        Rename-Files -Path $RootDirectory
    }
    "List" {
        List-Files -Path $RootDirectory
    }
    "Copy" {
        if (-not $DestinationDirectory) {
            Write-Error "DestinationDirectory is required for Copy action"
            exit 1
        }
        Copy-Files -SourcePath $RootDirectory -DestinationPath $DestinationDirectory
    }
    "Delete" {
        Delete-Files -Path $RootDirectory
    }
    "Move" {
        if (-not $DestinationDirectory) {
            Write-Error "DestinationDirectory is required for Move action"
            exit 1
        }
        Move-Files -SourcePath $RootDirectory -DestinationPath $DestinationDirectory
    }
    default {
        Write-Error "Invalid action specified"
        exit 1
    }
}
