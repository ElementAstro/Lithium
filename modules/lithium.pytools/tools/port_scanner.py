import asyncio
import socket
import concurrent.futures
import argparse
import json
from typing import List, Tuple


async def scan_port(ip: str, port: int, timeout: float, protocol: str = 'tcp', verbose: bool = False) -> Tuple[int, str, str]:
    """
    Attempt to connect to a specific IP and port, and detect the service type.

    Args:
    ip (str): The target IP address.
    port (int): The target port to scan.
    timeout (float): The connection timeout duration.
    protocol (str): The protocol to use for scanning ('tcp' or 'udp').
    verbose (bool): Whether to print detailed scan information.

    Returns:
    Tuple[int, str, str]: A tuple containing the port, its status ('open' or 'closed'), and the service name.
    """
    try:
        if protocol == 'tcp':
            _, writer = await asyncio.wait_for(
                asyncio.open_connection(ip, port), timeout=timeout)
            writer.close()
            await writer.wait_closed()
        else:  # UDP
            # UDP scan is connectionless, we use a datagram socket
            transport, _ = await asyncio.wait_for(
                asyncio.get_event_loop().create_datagram_endpoint(
                    lambda: asyncio.DatagramProtocol(), remote_addr=(ip, port)), timeout=timeout)
            transport.close()

        # Attempt to get the service name
        try:
            service = socket.getservbyport(port, protocol)
        except OSError:
            service = 'Unknown'

        if verbose:
            print(f"Port {port} is open (Service: {service})")

        return (port, 'open', service)

    except (asyncio.TimeoutError, OSError):
        if verbose:
            print(f"Port {port} is closed")
        return (port, 'closed', '')


async def scan_ports(ip: str, ports: range, timeout: float, protocol: str = 'tcp', verbose: bool = False) -> List[Tuple[int, str, str]]:
    """
    Scan a range of ports on a specified IP address.

    Args:
    ip (str): The target IP address.
    ports (range): The range of ports to scan.
    timeout (float): The connection timeout duration.
    protocol (str): The protocol to use for scanning ('tcp' or 'udp').
    verbose (bool): Whether to print detailed scan information.

    Returns:
    List[Tuple[int, str, str]]: A list of tuples containing the port, its status, and the service name.
    """
    tasks = [scan_port(ip, port, timeout, protocol, verbose) for port in ports]
    results = await asyncio.gather(*tasks)
    return results


def save_results(ip: str, results: List[Tuple[int, str, str]], output_file: str, json_format: bool = False) -> None:
    """
    Save the scan results to a file or print to console.

    Args:
    ip (str): The target IP address.
    results (List[Tuple[int, str, str]]): The list of scan results.
    output_file (str): The output file path.
    json_format (bool): Whether to save the results in JSON format.
    """
    if json_format:
        with open(output_file, "w", encoding="utf-8") as f:
            json.dump({ip: results}, f, indent=4)
        print(f"Results have been saved to {output_file} in JSON format")
    else:
        with open(output_file, "w", encoding="utf-8") as f:
            for port, status, service in results:
                f.write(f"{ip}:{port} {status} {service}\n")
        print(f"Results have been saved to {output_file}")


def parse_arguments() -> argparse.Namespace:
    """
    Parse command line arguments.

    Returns:
    argparse.Namespace: The parsed arguments.
    """
    parser = argparse.ArgumentParser(description='Asynchronous Port Scanner')
    parser.add_argument('ips', metavar='IP', type=str,
                        nargs='+', help='Target IP address(es)')
    parser.add_argument('-p', '--ports', type=str, default='1-1024',
                        help='Port range to scan (e.g., 1-1024)')
    parser.add_argument('-t', '--timeout', type=float,
                        default=1.0, help='Timeout for each port (seconds)')
    parser.add_argument('-o', '--output', type=str,
                        default='scan_results.txt', help='Output file to save results')
    parser.add_argument('--protocol', type=str,
                        choices=['tcp', 'udp'], default='tcp', help='Protocol to use (tcp/udp)')
    parser.add_argument('--quick', action='store_true',
                        help='Quick scan mode (common ports)')
    parser.add_argument('--json', action='store_true',
                        help='Save results in JSON format')
    parser.add_argument('--verbose', action='store_true',
                        help='Print detailed scan information')
    parser.add_argument('--exclude', type=str, default='',
                        help='Comma-separated list of ports to exclude from scanning')
    return parser.parse_args()


def get_ports(ports_str: str, quick: bool, exclude: str) -> range:
    """
    Get a range of ports to scan.

    Args:
    ports_str (str): The port range string (e.g., "1-1024").
    quick (bool): Whether to perform a quick scan (common ports only).
    exclude (str): Comma-separated list of ports to exclude from scanning.

    Returns:
    range: The range of ports to scan.
    """
    if quick:
        ports = range(1, 1025)  # Common ports for quick scan
    else:
        start_port, end_port = map(int, ports_str.split('-'))
        ports = range(start_port, end_port + 1)

    if exclude:
        exclude_ports = set(map(int, exclude.split(',')))
        ports = [port for port in ports if port not in exclude_ports]

    return ports


def main() -> None:
    """
    Main function to coordinate the scanning process.
    """
    args = parse_arguments()
    ports = get_ports(args.ports, args.quick, args.exclude)
    loop = asyncio.get_event_loop()

    with concurrent.futures.ThreadPoolExecutor() as executor:
        loop.set_default_executor(executor)
        for ip in args.ips:
            results = loop.run_until_complete(scan_ports(
                ip.strip(), ports, args.timeout, args.protocol, args.verbose))
            save_results(ip.strip(), results, args.output, args.json)


if __name__ == '__main__':
    main()
