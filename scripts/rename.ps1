param(
    [Parameter(Mandatory=$true)]
    [string]$RootDirectory,
    [Parameter(Mandatory=$true)]
    [string]$OldElement,
    [Parameter(Mandatory=$true)]
    [string]$NewElement
)

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

Rename-Files -Path $RootDirectory
