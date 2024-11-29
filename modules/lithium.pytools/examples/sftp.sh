# Example 1: Upload a directory to the server
# This example uploads a local directory to the specified remote directory on the SFTP server.
$ python sftp.py hostname username --password mypassword upload-dir /local/path /remote/path

# Example 2: Download a directory from the server
# This example downloads a remote directory to the specified local directory.
$ python sftp.py hostname username --password mypassword download-dir /remote/path /local/path

# Example 3: Create a directory on the server
# This example creates a directory on the SFTP server at the specified remote path.
$ python sftp.py hostname username --password mypassword mkdir /remote/path

# Example 4: Remove a directory from the server
# This example removes a directory from the SFTP server at the specified remote path.
$ python sftp.py hostname username --password mypassword rmdir /remote/path

# Example 5: Get file or directory info
# This example retrieves information about a file or directory on the SFTP server.
$ python sftp.py hostname username --password mypassword info /remote/path

# Example 6: Resume an interrupted file upload
# This example resumes an interrupted file upload to the SFTP server.
$ python sftp.py hostname username --password mypassword resume-upload /local/file /remote/file

# Example 7: List files in a remote directory
# This example lists the files in the specified remote directory on the SFTP server.
$ python sftp.py hostname username --password mypassword list /remote/path

# Example 8: Move or rename a remote file
# This example moves or renames a file on the SFTP server from the source path to the destination path.
$ python sftp.py hostname username --password mypassword move /remote/src /remote/dest

# Example 9: Delete a remote file
# This example deletes a file from the SFTP server at the specified remote path.
$ python sftp.py hostname username --password mypassword delete /remote/path