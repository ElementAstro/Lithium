param(
    [Parameter(Mandatory=$true)]
    [string]$RootDirectory,
    [Parameter(Mandatory=$true)]
    [string]$OldElement,
    [Parameter(Mandatory=$true)]
    [string]$NewElement,
    [Parameter(Mandatory=$true)]
    [ValidateSet("Rename", "List", "Copy", "Delete", "Move")]
    [string]$Action,
    [string]$DestinationDirectory
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

function List-Files($Path) {
    Get-ChildItem -Path $Path -Recurse
}

function Copy-Files($SourcePath, $DestinationPath) {
    Copy-Item -Path $SourcePath -Destination $DestinationPath -Recurse -Force
}

function Delete-Files($Path) {
    Remove-Item -Path $Path -Recurse -Force
}

function Move-Files($SourcePath, $DestinationPath) {
    Move-Item -Path $SourcePath -Destination $DestinationPath -Recurse -Force
}

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