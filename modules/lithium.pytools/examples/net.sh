# Example 1: List all installed .NET Framework versions
# This example lists all installed .NET Framework versions by querying the Windows Registry.
$ python net.py --list

# Example 2: Check if a specific .NET Framework version is installed
# This example checks if the specified .NET Framework version is installed.
$ python net.py --check v4.0.30319

# Example 3: Check if a specific .NET Framework version is installed and download the installer if not
# This example checks if the specified .NET Framework version is installed, and if not, downloads the installer from the specified URL.
$ python net.py --check v4.0.30319 --download https://example.com/installer.exe --install ./installer.exe --threads 4 --checksum abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890

# Example 4: Uninstall a specific .NET Framework version
# This example uninstalls the specified .NET Framework version.
$ python net.py --uninstall v4.0.30319