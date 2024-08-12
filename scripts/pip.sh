#!/bin/bash

# Function to update Python packages
update_python_packages() {
    local excluded_packages=("$@")
    local updated_packages=()
    local failed_packages=()

    if [ "$UPDATE_ALL" = true ]; then
        packages=$(pip list --format=json | jq -r '.[].name')
    else
        packages=$(pip list --outdated --format=json | jq -r '.[].name')
    fi

    for package in $packages; do
        if [[ " ${excluded_packages[@]} " =~ " ${package} " ]]; then
            echo "Skipping excluded package: $package"
            continue
        fi

        echo "Updating package: $package"
        if pip install --upgrade "$package" >/dev/null 2>&1; then
            updated_packages+=("$package")
            echo "Package updated successfully: $package"
        else
            failed_packages+=("$package")
            echo "Failed to update package: $package"
        fi
    done

    echo "Updated packages: ${updated_packages[*]}"
    echo "Failed packages: ${failed_packages[*]}"
}

# Function to generate update report
generate_report() {
    local excluded_packages=("$1")
    local updated_packages=("$2")
    local failed_packages=("$3")

    report_file="PythonPackageUpdateReport_$(date +%Y%m%d_%H%M%S).txt"
    {
        echo "Python Package Update Report"
        echo "============================"
        echo
        echo "Date: $(date)"
        echo
        echo "Excluded Packages:"
        printf '%s\n' "${excluded_packages[@]}"
        echo
        echo "Updated Packages:"
        printf '%s\n' "${updated_packages[@]}"
        echo
        echo "Failed Packages:"
        printf '%s\n' "${failed_packages[@]}"
        echo
        echo "Summary:"
        echo "--------"
        echo "Total packages processed: $((${#updated_packages[@]} + ${#failed_packages[@]}))"
        echo "Successfully updated: ${#updated_packages[@]}"
        echo "Failed to update: ${#failed_packages[@]}"
    } > "$report_file"

    echo "Update report saved to: $report_file"
}

# Main execution
EXCLUDED_PACKAGES=()
UPDATE_ALL=false
GENERATE_REPORT=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --exclude)
            shift
            IFS=',' read -ra EXCLUDED_PACKAGES <<< "$1"
            ;;
        --update-all)
            UPDATE_ALL=true
            ;;
        --generate-report)
            GENERATE_REPORT=true
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
    shift
done

# Run the update function
update_result=$(update_python_packages "${EXCLUDED_PACKAGES[@]}")

# Extract updated and failed packages from the result
IFS=$'\n' read -rd '' -a lines <<< "$update_result"
updated_packages=($(echo "${lines[0]}" | cut -d ':' -f2))
failed_packages=($(echo "${lines[1]}" | cut -d ':' -f2))

echo "Update completed."
echo "Total packages updated: ${#updated_packages[@]}"

if [ ${#failed_packages[@]} -gt 0 ]; then
    echo "Failed to update the following packages:"
    printf '- %s\n' "${failed_packages[@]}"
fi

if [ "$GENERATE_REPORT" = true ]; then
    generate_report "${EXCLUDED_PACKAGES[*]}" "${updated_packages[*]}" "${failed_packages[*]}"
fi
