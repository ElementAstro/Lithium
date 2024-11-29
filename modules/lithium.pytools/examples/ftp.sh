# Example 1: List files in the remote directory
# This example lists all files and directories in the specified remote path.
$ python ftp.py --host ftp.example.com --username user --password pass ls --path /remote/path

# Example 2: List files recursively in the remote directory
# This example lists all files and directories recursively in the specified remote path.
$ python ftp.py --host ftp.example.com --username user --password pass ls --path /remote/path --recursive

# Example 3: Download a file from the remote server
# This example downloads a file from the specified remote path to the specified local path.
$ python ftp.py --host ftp.example.com --username user --password pass get /remote/path/file.txt /local/path/file.txt

# Example 4: Upload a file to the remote server
# This example uploads a file from the specified local path to the specified remote path.
$ python ftp.py --host ftp.example.com --username user --password pass put /local/path/file.txt /remote/path/file.txt

# Example 5: Delete a file on the remote server
# This example deletes a file at the specified remote path.
$ python ftp.py --host ftp.example.com --username user --password pass rm /remote/path/file.txt

# Example 6: Create a directory on the remote server
# This example creates a directory at the specified remote path.
$ python ftp.py --host ftp.example.com --username user --password pass mkdir /remote/path/new_directory

# Example 7: Remove a directory on the remote server
# This example removes a directory at the specified remote path.
$ python ftp.py --host ftp.example.com --username user --password pass rmdir /remote/path/new_directory

# Example 8: Rename a file or directory on the remote server
# This example renames a file or directory from the old path to the new path.
$ python ftp.py --host ftp.example.com --username user --password pass rename /remote/path/old_name /remote/path/new_name

# Example 9: Show the current directory on the remote server
# This example shows the current working directory on the remote server.
$ python ftp.py --host ftp.example.com --username user --password pass pwd

# Example 10: Change the current directory on the remote server
# This example changes the current working directory to the specified remote path.
$ python ftp.py --host ftp.example.com --username user --password pass cd /remote/path

# Example 11: Perform batch upload/download operations
# This example performs batch upload/download operations specified in a JSON file.
$ python ftp.py --host ftp.example.com --username user --password pass batch /path/to/operations.json