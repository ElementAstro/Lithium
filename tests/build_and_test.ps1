<#
.SYNOPSIS
    This script sets up a development environment by installing necessary tools using Chocolatey,
    cloning and building a project with dependencies.

.DESCRIPTION
    The script checks for the presence of required tools like Chocolatey, CMake, Git, and Visual Studio Build Tools.
    It installs any missing tools and then proceeds to clone and build a specified project using CMake and Git.

.AUTHOR
    Your Name
#>

# Function to check if a specific command exists in the system
function CommandExists {
<#
    .SYNOPSIS
        Checks if a command/utility is available in the system PATH.

    .DESCRIPTION
        Attempts to get the command using Get-Command and suppresses errors to determine if a command is available.

    .PARAMETER Command
        The name of the command to check.

    .EXAMPLE
        if (CommandExists 'git') {
            Write-Host "Git is installed."
        }

    .RETURN
        Returns true if the command exists, otherwise false.
#>
    param (
        [string]$Command
    )
    $null = Get-Command $Command -ErrorAction SilentlyContinue
    return $?
}

# Function to install Chocolatey if it is not already installed
function InstallChocolatey {
<#
    .SYNOPSIS
        Installs Chocolatey on the system if it is not already installed.

    .DESCRIPTION
        Checks for the presence of the Chocolatey command and installs it using an online script if it is absent.
#>
    if (-not (CommandExists 'choco')) {
        Write-Host "Chocolatey is not installed. Installing Chocolatey..."
        Set-ExecutionPolicy Bypass -Scope Process -Force;
        [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072;
        Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
    }
}

# Function to install a package using Chocolatey
function InstallPackage {
<#
    .SYNOPSIS
        Installs a specified package using Chocolatey.

    .PARAMETER Package
        The name of the package to install.

    .EXAMPLE
        InstallPackage 'cmake'
#>
    param (
        [string]$Package
    )
    if (-not (CommandExists $Package)) {
        Write-Host "$Package is not installed. Installing $Package..."
        choco install $Package -y
    }
}

# Main script execution starts here
# Check and install Chocolatey if necessary
InstallChocolatey

# Install essential packages
'cmake', 'git', 'visualstudio2019buildtools' | ForEach-Object { InstallPackage $_ }

# Validate Visual Studio Build Tools installation
if (-not (CommandExists 'cl')) {
    Write-Host "Visual Studio Build Tools are not installed properly."
    exit
}

# Create and navigate to the build directory
$buildDir = "build"
if (-not (Test-Path -Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir
}
Set-Location -Path $buildDir

# Clone Google Test if it's not already present
if (-not (Test-Path -Path "googletest")) {
    Write-Host "Cloning Google Test..."
    git clone https://github.com/google/googletest.git
}

# Configure the project with CMake
Write-Host "Configuring the project with CMake..."
cmake ..

# Build the project
Write-Host "Building the project..."
cmake --build .

# Run the tests
Write-Host "Running the tests..."
ctest --output-on-failure

# Return to the original directory
Set-Location -Path ..

Write-Host "Build and test process completed successfully."