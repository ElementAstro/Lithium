#!/bin/bash

# Shared Memory Management Script
# This script provides a command-line interface for managing shared memory segments in a UNIX-like environment.
# It includes functionalities to show current memory status, create segments, view details, change permissions,
# batch delete segments, check memory usage, and display help instructions.
# Author: [Your Name]
# Date: [Current Date]

# Log function for better logging
# This function takes a message as an argument and appends it to a log file with a timestamp.
log() {
    echo "$(date +'%Y-%m-%d %H:%M:%S') - $1" >> shm_management.log
}

# Show current shared memory status
# This function uses the ipcs command to display all current shared memory segments.
show_shm_status() {
    echo "Current shared memory status:"
    ipcs -m
    log "Displayed current shared memory status."
}

# Create shared memory segment
# This function prompts the user for the size of the shared memory segment,
# then creates it using ipcmk. It logs the creation process.
create_shm_segment() {
    echo -n "Enter the size of the shared memory segment (in bytes): "
    read size
    shm_id=$(ipcmk -M $size)  # Create shared memory segment
    if [ $? -eq 0 ]; then
        echo "Shared memory segment created with ID: $shm_id."
        log "Created shared memory segment with ID: $shm_id and size: $size bytes."
    else
        echo "Failed to create shared memory segment."
        log "Failed to create shared memory segment of size: $size bytes."
    fi
}

# View details of a shared memory segment
# This function allows the user to view detailed information about a specific shared memory segment.
view_shm_details() {
    echo -n "Enter the shared memory segment ID to view details: "
    read shm_id
    ipcs -m -i $shm_id  # Display details of the specified shared memory segment
    log "Viewed details for shared memory segment ID: $shm_id."
}

# Change permissions of a shared memory segment
# This function allows the user to change the permissions of a specific shared memory segment.
change_shm_permissions() {
    echo -n "Enter the shared memory segment ID to change permissions: "
    read shm_id
    echo -n "Enter new permissions (e.g., 600): "
    read permissions
    chmod $permissions /proc/sysvipc/shm/$shm_id  # Change permissions on the segment
    if [ $? -eq 0 ]; then
        echo "Permissions for shared memory segment ID: $shm_id changed to $permissions."
        log "Changed permissions for shared memory segment ID: $shm_id to $permissions."
    else
        echo "Failed to change permissions."
        log "Failed to change permissions for segment ID: $shm_id."
    fi
}

# Batch delete shared memory segments
# This function allows the user to delete multiple shared memory segments specified by their IDs.
delete_multiple_shm_segments() {
    echo -n "Enter the IDs of shared memory segments to delete (space-separated): "
    read -a shm_ids  # Read multiple IDs into an array
    for shm_id in "${shm_ids[@]}"; do
        ipcrm -m $shm_id  # Delete the specified shared memory segment
        if [ $? -eq 0 ]; then
            echo "Shared memory segment ID: $shm_id has been deleted."
            log "Deleted shared memory segment ID: $shm_id."
        else
            echo "Failed to delete shared memory segment ID: $shm_id."
            log "Failed to delete shared memory segment ID: $shm_id."
        fi
    done
}

# Check memory usage
# This function displays the current system memory usage using the free command.
check_memory_usage() {
    echo "Current memory usage:"
    free -h  # Display memory usage in human-readable format
    log "Displayed current memory usage."
}

# Show help
# This function displays usage information for the script.
show_help() {
    echo "Shared Memory Management Script Usage:"
    echo "1. Show current shared memory status - View all current shared memory segments."
    echo "2. Create shared memory segment - Create a new shared memory segment."
    echo "3. View shared memory segment details - View details of a specific shared memory segment."
    echo "4. Change shared memory segment permissions - Change the permissions of a specific segment."
    echo "5. Batch delete shared memory segments - Delete multiple shared memory segments at once."
    echo "6. Check memory usage - Display current system memory usage."
    echo "7. Help - Display this help information."
}

# Main menu
# This loop presents the user with options to perform various shared memory management tasks.
while true; do
    echo "Shared Memory Management Script"
    echo "1. Show current shared memory status"
    echo "2. Create shared memory segment"
    echo "3. View shared memory segment details"
    echo "4. Change shared memory segment permissions"
    echo "5. Batch delete shared memory segments"
    echo "6. Check memory usage"
    echo "7. Help"
    echo "8. Exit"
    echo -n "Please select an option: "
    read choice  # Read user choice

    case $choice in
        1) show_shm_status ;;
        2) create_shm_segment ;;
        3) view_shm_details ;;
        4) change_shm_permissions ;;
        5) delete_multiple_shm_segments ;;
        6) check_memory_usage ;;
        7) show_help ;;
        8) echo "Exiting"; log "Exited the script."; exit 0 ;;
        *) echo "Invalid option, please try again." ;;
    esac
done