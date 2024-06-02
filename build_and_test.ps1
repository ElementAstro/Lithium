# Function to check if a command exists
function CommandExists {
    param (
        [string]$Command
    )
    $null = Get-Command $Command -ErrorAction SilentlyContinue
    return $?
}

# Function to install Chocolatey if not installed
function InstallChocolatey {
    if (-not (CommandExists choco)) {
        Write-Host "Chocolatey is not installed. Installing Chocolatey..."
        Set-ExecutionPolicy Bypass -Scope Process -Force;
        [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072;
        Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
    }
}

# Function to install a package using Chocolatey
function InstallPackage {
    param (
        [string]$Package
    )
    choco install $Package -y
}

# Check and install Chocolatey if necessary
InstallChocolatey

# Check and install CMake if necessary
if (-not (CommandExists cmake)) {
    Write-Host "CMake is not installed. Installing CMake..."
    InstallPackage cmake
}

# Check and install Git if necessary
if (-not (CommandExists git)) {
    Write-Host "Git is not installed. Installing Git..."
    InstallPackage git
}

# Check and install Visual Studio Build Tools if necessary
if (-not (CommandExists cl)) {
    Write-Host "Visual Studio Build Tools are not installed. Installing Build Tools..."
    InstallPackage visualstudio2019buildtools --package-parameters "--add Microsoft.VisualStudio.Workload.VCTools --includeRecommended --includeOptional"
}

# Create and navigate to the build directory
if (-not (Test-Path -Path "build")) {
    New-Item -ItemType Directory -Path "build"
}
Set-Location -Path "build"

# Check if Google Test is already cloned
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