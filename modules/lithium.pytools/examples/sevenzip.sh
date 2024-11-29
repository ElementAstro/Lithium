# Example 1: Compress files into an archive
# This example compresses the specified files into an archive with a compression level of 5.
$ python sevenzip.py compress -f file1.txt file2.txt -a archive.7z -l 5

# Example 2: Compress files into a password-protected archive
# This example compresses the specified files into a password-protected archive.
$ python sevenzip.py compress -f file1.txt file2.txt -a archive.7z -p mypassword

# Example 3: Extract files from an archive
# This example extracts the contents of the specified archive to the given destination directory.
$ python sevenzip.py extract -a archive.7z -d /path/to/destination

# Example 4: Extract files from a password-protected archive
# This example extracts the contents of a password-protected archive to the given destination directory.
$ python sevenzip.py extract -a archive.7z -d /path/to/destination -p mypassword

# Example 5: List contents of an archive
# This example lists the contents of the specified archive.
$ python sevenzip.py list -a archive.7z

# Example 6: List contents of a password-protected archive
# This example lists the contents of a password-protected archive.
$ python sevenzip.py list -a archive.7z -p mypassword

# Example 7: Test the integrity of an archive
# This example tests the integrity of the specified archive.
$ python sevenzip.py test -a archive.7z

# Example 8: Test the integrity of a password-protected archive
# This example tests the integrity of a password-protected archive.
$ python sevenzip.py test -a archive.7z -p mypassword

# Example 9: Delete an archive
# This example deletes the specified archive.
$ python sevenzip.py delete -a archive.7z

# Example 10: Add files to an existing archive
# This example adds the specified files to an existing archive.
$ python sevenzip.py update -a archive.7z -f file3.txt file4.txt --add

# Example 11: Delete files from an existing archive
# This example deletes the specified files from an existing archive.
$ python sevenzip.py update -a archive.7z -f file1.txt file2.txt --delete

# Example 12: Get the version of the 7z executable
# This example retrieves the version of the 7z executable.
$ python sevenzip.py version

# Example 13: List supported archive formats
# This example lists the supported archive formats by the 7z executable.
$ python sevenzip.py formats