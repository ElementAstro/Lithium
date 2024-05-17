# Meson build file for Lithium Builtin Modules
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Lithium Builtin Modules
# Description: A collection of useful system functions
# Author: Max Qian
# License: GPL3

project('lithium.builtin', ['c', 'cpp'], license : 'GPL3')

# Function to recursively add subdirectories that contain a meson.build file
subdirs = []

# Function to get all subdirectories that contain both meson.build and package.json
def add_subdirectories_recursively(start_dir)
  entries = meson.get_directory_listing(start_dir, true)

  foreach entry : entries
    if entry.is_dir() and
       meson.is_directory(entry.path()) and
       meson.is_file(entry.path(), 'meson.build') and
       meson.is_file(entry.path(), 'package.json')
      message('Adding module subdirectory: ' + entry.path())
      subdirs += entry.path()
    endif
  endforeach
endfunction

# Use the function to find subdirectories
add_subdirectories_recursively(meson.current_source_dir())

# Add subdirectories to the build
foreach subdir : subdirs
  subdir(subdir)
endforeach
