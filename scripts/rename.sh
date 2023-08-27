#!/bin/bash

root_directory=$1
old_element=$2
new_element=$3

rename_files() {
  local path=$1

  for file in "$path"/*; do
    if [[ -f $file ]]; then
      new_file_name="${file/$old_element/$new_element}"
      new_file_path="$(dirname "$file")/$new_file_name"
      mv -n "$file" "$new_file_path"
    elif [[ -d $file ]]; then
      rename_files "$file"
    fi
  done
}

rename_files "$root_directory"
