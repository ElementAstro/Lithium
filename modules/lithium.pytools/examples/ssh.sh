# Example 1: Execute a command on the remote server
# This example executes the specified command on the remote SSH server.
$ python ssh.py --hostname example.com --username user --password pass exec "ls -la"

# Example 2: Upload a file to the remote server
# This example uploads a local file to the specified path on the remote SSH server.
$ python ssh.py --hostname example.com --username user --password pass upload /local/path/to/file /remote/path/to/file

# Example 3: Download a file from the remote server
# This example downloads a file from the specified path on the remote SSH server to the local machine.
$ python ssh.py --hostname example.com --username user --password pass download /remote/path/to/file /local/path/to/file

# Example 4: List contents of a remote directory
# This example lists the contents of the specified directory on the remote SSH server.
$ python ssh.py --hostname example.com --username user --password pass list /remote/path/to/directory

# Example 5: Create a directory on the remote server
# This example creates a directory at the specified path on the remote SSH server.
$ python ssh.py --hostname example.com --username user --password pass mkdir /remote/path/to/directory

# Example 6: Delete a file on the remote server
# This example deletes a file at the specified path on the remote SSH server.
$ python ssh.py --hostname example.com --username user --password pass delete /remote/path/to/file

# Example 7: Execute a command on the remote server using a private key
# This example executes the specified command on the remote SSH server using a private key for authentication.
$ python ssh.py --hostname example.com --username user --key_file /path/to/private/key exec "ls -la"

# Example 8: Upload a file to the remote server using a private key
# This example uploads a local file to the specified path on the remote SSH server using a private key for authentication.
$ python ssh.py --hostname example.com --username user --key_file /path/to/private/key upload /local/path/to/file /remote/path/to/file

# Example 9: Download a file from the remote server using a private key
# This example downloads a file from the specified path on the remote SSH server to the local machine using a private key for authentication.
$ python ssh.py --hostname example.com --username user --key_file /path/to/private/key download /remote/path/to/file /local/path/to/file

# Example 10: List contents of a remote directory using a private key
# This example lists the contents of the specified directory on the remote SSH server using a private key for authentication.
$ python ssh.py --hostname example.com --username user --key_file /path/to/private/key list /remote/path/to/directory

# Example 11: Create a directory on the remote server using a private key
# This example creates a directory at the specified path on the remote SSH server using a private key for authentication.
$ python ssh.py --hostname example.com --username user --key_file /path/to/private/key mkdir /remote/path/to/directory

# Example 12: Delete a file on the remote server using a private key
# This example deletes a file at the specified path on the remote SSH server using a private key for authentication.
$ python ssh.py --hostname example.com --username user --key_file /path/to/private/key delete /remote/path/to/file