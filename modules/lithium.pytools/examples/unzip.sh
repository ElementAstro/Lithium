# Example 1: Extract files from an archive
# This example extracts the contents of the specified archive to the given destination directory.
$ python unzip.py extract -a archive.zip -d /path/to/destination

# Example 2: Extract files from a password-protected archive
# This example extracts the contents of a password-protected archive to the given destination directory.
$ python unzip.py extract -a archive.zip -d /path/to/destination -p mypassword

# Example 3: List contents of an archive
# This example lists the contents of the specified archive.
$ python unzip.py list -a archive.zip

# Example 4: List contents of a password-protected archive
# This example lists the contents of a password-protected archive.
$ python unzip.py list -a archive.zip -p mypassword

# Example 5: Test the integrity of an archive
# This example tests the integrity of the specified archive.
$ python unzip.py test -a archive.zip

# Example 6: Test the integrity of a password-protected archive
# This example tests the integrity of a password-protected archive.
$ python unzip.py test -a archive.zip -p mypassword

# Example 7: Delete an archive
# This example deletes the specified archive.
$ python unzip.py delete -a archive.zip

# Example 8: Add files to an archive (not supported)
# This example attempts to add files to an existing archive, but the operation is not supported by unzip.
$ python unzip.py update -a archive.zip -f file1.txt file2.txt --action add

# Example 9: Delete files from an archive (not supported)
# This example attempts to delete files from an existing archive, but the operation is not supported by unzip.
$ python unzip.py update -a archive.zip -f file1.txt file2.txt --action delete