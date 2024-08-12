[CmdletBinding()]
param (
    [string[]]$ExcludedPackages = @(),
    [switch]$UpdateAll,
    [switch]$GenerateReport
)

function Update-PythonPackages {
    [CmdletBinding()]
    param (
        [string[]]$ExcludedPackages,
        [switch]$UpdateAll
    )

    $outdatedPackages = if ($UpdateAll) {
        pip list --format=json | ConvertFrom-Json | Select-Object -ExpandProperty name
    } else {
        pip list --outdated --format=json | ConvertFrom-Json | Select-Object -ExpandProperty name
    }

    $updatedPackages = @()
    $failedPackages = @()

    foreach ($package in $outdatedPackages) {
        if ($ExcludedPackages -contains $package) {
            Write-Verbose "Skipping excluded package: $package"
            continue
        }

        Write-Verbose "Updating package: $package"
        try {
            $output = pip install --upgrade $package 2>&1
            $updatedPackages += $package
            Write-Verbose "Package updated successfully: $package"
        }
        catch {
            Write-Warning "Failed to update package: $package"
            $failedPackages += $package
        }
    }

    return @{
        Updated = $updatedPackages
        Failed = $failedPackages
    }
}

function New-UpdateReport {
    [CmdletBinding()]
    param (
        [string[]]$ExcludedPackages,
        [string[]]$UpdatedPackages,
        [string[]]$FailedPackages
    )

    $reportContent = @"
Python Package Update Report
============================

Date: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

Excluded Packages:
$($ExcludedPackages -join "`n")

Updated Packages:
$($UpdatedPackages -join "`n")

Failed Packages:
$($FailedPackages -join "`n")

Summary:
--------
Total packages processed: $($UpdatedPackages.Count + $FailedPackages.Count)
Successfully updated: $($UpdatedPackages.Count)
Failed to update: $($FailedPackages.Count)
"@

    $reportFile = "PythonPackageUpdateReport_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"
    $reportContent | Out-File -FilePath $reportFile -Encoding UTF8
    Write-Host "Update report saved to: $reportFile"
}

# Main execution
$updateResult = Update-PythonPackages -ExcludedPackages $ExcludedPackages -UpdateAll:$UpdateAll

Write-Host "Update completed."
Write-Host "Total packages updated: $($updateResult.Updated.Count)"

if ($updateResult.Failed.Count -gt 0) {
    Write-Host "Failed to update the following packages:"
    $updateResult.Failed | ForEach-Object { Write-Host "- $_" }
}

if ($GenerateReport) {
    New-UpdateReport -ExcludedPackages $ExcludedPackages -UpdatedPackages $updateResult.Updated -FailedPackages $updateResult.Failed
}
