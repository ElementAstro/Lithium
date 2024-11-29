# Example 1: Scan a range of ports on a single IP address
# This example scans ports 1 to 1024 on the specified IP address using TCP protocol.
$ python port_scanner.py 192.168.1.1 -p 1-1024

# Example 2: Scan a range of ports on multiple IP addresses
# This example scans ports 1 to 1024 on multiple IP addresses using TCP protocol.
$ python port_scanner.py 192.168.1.1 192.168.1.2 -p 1-1024

# Example 3: Scan using UDP protocol
# This example scans ports 1 to 1024 on the specified IP address using UDP protocol.
$ python port_scanner.py 192.168.1.1 -p 1-1024 --protocol udp

# Example 4: Perform a quick scan on common ports
# This example performs a quick scan on common ports (1 to 1024) on the specified IP address.
$ python port_scanner.py 192.168.1.1 --quick

# Example 5: Save scan results to a file
# This example scans ports 1 to 1024 on the specified IP address and saves the results to a file.
$ python port_scanner.py 192.168.1.1 -p 1-1024 -o scan_results.txt

# Example 6: Save scan results in JSON format
# This example scans ports 1 to 1024 on the specified IP address and saves the results in JSON format.
$ python port_scanner.py 192.168.1.1 -p 1-1024 -o scan_results.json --json

# Example 7: Scan with a custom timeout
# This example scans ports 1 to 1024 on the specified IP address with a custom timeout of 2 seconds per port.
$ python port_scanner.py 192.168.1.1 -p 1-1024 -t 2.0

# Example 8: Exclude specific ports from scanning
# This example scans ports 1 to 1024 on the specified IP address but excludes ports 80 and 443.
$ python port_scanner.py 192.168.1.1 -p 1-1024 --exclude 80,443

# Example 9: Enable verbose output
# This example scans ports 1 to 1024 on the specified IP address and prints detailed scan information.
$ python port_scanner.py 192.168.1.1 -p 1-1024 --verbose

# Example 10: Scan multiple IP addresses with quick scan and save results in JSON format
# This example performs a quick scan on common ports for multiple IP addresses and saves the results in JSON format.
$ python port_scanner.py 192.168.1.1 192.168.1.2 --quick -o scan_results.json --json