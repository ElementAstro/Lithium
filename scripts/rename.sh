#!/bin/bash

# rename.sh
#
# 用于在 Linux 上管理文件和目录的 Shell 脚本
#
# 使用方法：
#   ./rename.sh {rename|list|copy|delete|move} <RootDirectory> <OldElement> <NewElement> [<DestinationDirectory>]
#
# 参数：
#   - Action: 必需。操作类型。可选值为 rename、list、copy、delete、move。
#   - RootDirectory: 必需。操作的根目录。
#   - OldElement: 重命名操作中要被替换的字符串。
#   - NewElement: 重命名操作中替换后的新字符串。
#   - DestinationDirectory: 复制和移动操作的目标目录。
#
# 使用示例：
#   重命名文件：
#     ./rename.sh rename /path/to/directory old new
#
#   列出文件和目录：
#     ./rename.sh list /path/to/directory
#
#   复制文件和目录：
#     ./rename.sh copy /path/to/source old new /path/to/destination
#
#   删除文件和目录：
#     ./rename.sh delete /path/to/directory
#
#   移动文件和目录：
#     ./rename.sh move /path/to/source old new /path/to/destination

# 参数解析
ACTION=$1
ROOT_DIRECTORY=$2
OLD_ELEMENT=$3
NEW_ELEMENT=$4
DESTINATION_DIRECTORY=$5

# 检查必需参数
if [ -z "$ACTION" ] || [ -z "$ROOT_DIRECTORY" ]; then
    echo "Usage: $0 {rename|list|copy|delete|move} <RootDirectory> <OldElement> <NewElement> [<DestinationDirectory>]"
    exit 1
fi

# 重命名文件
function rename_files {
    local path=$1
    for file in $(find "$path" -type f); do
        local dir=$(dirname "$file")
        local filename=$(basename "$file")
        local new_filename=$(echo "$filename" | sed "s/$OLD_ELEMENT/$NEW_ELEMENT/g")
        local new_filepath="$dir/$new_filename"
        mv "$file" "$new_filepath" 2>/dev/null
    done

    for dir in $(find "$path" -type d); do
        rename_files "$dir"
    done
}

# 列出文件和目录
function list_files {
    find "$1" -print
}

# 复制文件和目录
function copy_files {
    cp -R "$1" "$2"
}

# 删除文件和目录
function delete_files {
    rm -rf "$1"
}

# 移动文件和目录
function move_files {
    mv "$1" "$2"
}

# 主程序入口
case $ACTION in
    rename)
        if [ -z "$OLD_ELEMENT" ] || [ -z "$NEW_ELEMENT" ]; then
            echo "OldElement and NewElement are required for rename action"
            exit 1
        fi
        rename_files "$ROOT_DIRECTORY"
        ;;
    list)
        list_files "$ROOT_DIRECTORY"
        ;;
    copy)
        if [ -z "$DESTINATION_DIRECTORY" ]; then
            echo "DestinationDirectory is required for copy action"
            exit 1
        fi
        copy_files "$ROOT_DIRECTORY" "$DESTINATION_DIRECTORY"
        ;;
    delete)
        delete_files "$ROOT_DIRECTORY"
        ;;
    move)
        if [ -z "$DESTINATION_DIRECTORY" ]; then
            echo "DestinationDirectory is required for move action"
            exit 1
        fi
        move_files "$ROOT_DIRECTORY" "$DESTINATION_DIRECTORY"
        ;;
    *)
        echo "Invalid action specified"
        echo "Usage: $0 {rename|list|copy|delete|move} <RootDirectory> <OldElement> <NewElement> [<DestinationDirectory>]"
        exit 1
        ;;
esac
