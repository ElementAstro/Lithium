import argparse
import asyncio
import json
import socket
import sys
from typing import List, Tuple

from loguru import logger


"""
Asynchronous Port Scanner tool.
Scans specified ports on given IP addresses using TCP or UDP protocols.
Provides functionalities to scan ports, save results in various formats, and handle exceptions.
"""

# Configure loguru for logging
logger.remove()  # Remove default logger to customize logging settings

logger.add(
    "port_scanner.log",
    rotation="10 MB",
    retention="7 days",
    compression="zip",
    enqueue=True,
    encoding="utf-8",
    format=(
        "<green>{time:YYYY-MM-DD HH:mm:ss}</green> | "
        "<level>{level}</level> | <level>{message}</level>"
    ),
)

logger.add(
    sys.stdout,
    level="INFO",
    format="<level>{message}</level>",
)


class PortScannerError(Exception):
    """Custom exception class for Port Scanner errors."""
    pass


async def scan_port(
    ip: str,
    port: int,
    timeout: float,
    protocol: str = 'tcp',
    verbose: bool = False
) -> Tuple[int, str, str]:
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
            reader, writer = await asyncio.wait_for(
                asyncio.open_connection(ip, port),
                timeout=timeout
            )
            writer.close()
            await writer.wait_closed()
        else:  # UDP
            transport, _ = await asyncio.wait_for(
                asyncio.get_event_loop().create_datagram_endpoint(
                    lambda: asyncio.DatagramProtocol(),
                    remote_addr=(ip, port)
                ),
                timeout=timeout
            )
            transport.close()

        # Attempt to get the service name
        try:
            service = socket.getservbyport(port, protocol)
        except OSError:
            service = 'Unknown'

        if verbose:
            logger.info(f"Port {port} is open (Service: {service})")

        return (port, 'open', service)

    except (asyncio.TimeoutError, ConnectionRefusedError, OSError):
        if verbose:
            logger.info(f"Port {port} is closed")
        return (port, 'closed', '')


async def scan_ports(
    ip: str,
    ports: List[int],
    timeout: float,
    protocol: str = 'tcp',
    verbose: bool = False
) -> List[Tuple[int, str, str]]:
    """
    Scan a list of ports on a specified IP address.

    Args:
        ip (str): The target IP address.
        ports (List[int]): The list of ports to scan.
        timeout (float): The connection timeout duration.
        protocol (str): The protocol to use for scanning ('tcp' or 'udp').
        verbose (bool): Whether to print detailed scan information.

    Returns:
        List[Tuple[int, str, str]]: A list of tuples containing the port, its status, and the service name.
    """
    tasks = [
        scan_port(ip, port, timeout, protocol, verbose)
        for port in ports
    ]
    results = await asyncio.gather(*tasks, return_exceptions=True)

    # Handle exceptions in results
    processed_results = []
    for result in results:
        if isinstance(result, Exception):
            logger.error(f"Error scanning port: {result}")
        else:
            processed_results.append(result)
    return processed_results


def save_results(
    ip: str,
    results: List[Tuple[int, str, str]],
    output_file: str,
    json_format: bool = False
) -> None:
    """
    Save the scan results to a file or print to console.

    Args:
        ip (str): The target IP address.
        results (List[Tuple[int, str, str]]): The list of scan results.
        output_file (str): The output file path.
        json_format (bool): Whether to save the results in JSON format.
    """
    try:
        if json_format:
            data = {
                ip: [
                    {"port": port, "status": status, "service": service}
                    for port, status, service in results
                ]
            }
            with open(output_file, "w", encoding="utf-8") as f:
                json.dump(data, f, indent=4)
            logger.info(
                f"Results have been saved to {output_file} in JSON format"
            )
        else:
            with open(output_file, "w", encoding="utf-8") as f:
                for port, status, service in results:
                    f.write(f"{ip}:{port} {status} {service}\n")
            logger.info(f"Results have been saved to {output_file}")
    except IOError as e:
        logger.error(f"Failed to save results to {output_file}: {e}")
        raise PortScannerError(
            f"Failed to save results to {output_file}: {e}"
        ) from e


def parse_arguments() -> argparse.Namespace:
    """
    Parse command line arguments.

    Returns:
        argparse.Namespace: The parsed arguments.
    """
    parser = argparse.ArgumentParser(
        description='Asynchronous Port Scanner'
    )
    parser.add_argument(
        'ips',
        metavar='IP',
        type=str,
        nargs='+',
        help='Target IP address(es)'
    )
    parser.add_argument(
        '-p', '--ports',
        type=str,
        default='1-1024',
        help='Port range to scan (e.g., 1-1024)'
    )
    parser.add_argument(
        '-t', '--timeout',
        type=float,
        default=1.0,
        help='Timeout for each port (seconds)'
    )
    parser.add_argument(
        '-o', '--output',
        type=str,
        default='scan_results.txt',
        help='Output file to save results'
    )
    parser.add_argument(
        '--protocol',
        type=str,
        choices=['tcp', 'udp'],
        default='tcp',
        help='Protocol to use (tcp/udp)'
    )
    parser.add_argument(
        '--quick',
        action='store_true',
        help='Quick scan mode (common ports)'
    )
    parser.add_argument(
        '--json',
        action='store_true',
        help='Save results in JSON format'
    )
    parser.add_argument(
        '--verbose',
        action='store_true',
        help='Print detailed scan information'
    )
    parser.add_argument(
        '--exclude',
        type=str,
        default='',
        help='Comma-separated list of ports to exclude from scanning'
    )
    return parser.parse_args()


def get_ports(
    ports_str: str,
    quick: bool,
    exclude: str
) -> List[int]:
    """
    Get a list of ports to scan.

    Args:
        ports_str (str): The port range string (e.g., "1-1024").
        quick (bool): Whether to perform a quick scan (common ports only).
        exclude (str): Comma-separated list of ports to exclude from scanning.

    Returns:
        List[int]: The list of ports to scan.
    """
    if quick:
        ports = list(range(1, 1025))  # Common ports for quick scan
    else:
        try:
            start_port, end_port = map(int, ports_str.split('-'))
            ports = list(range(start_port, end_port + 1))
        except ValueError as e:
            logger.error(
                "Invalid port range format. Use start-end (e.g., 1-1024)."
            )
            raise PortScannerError(
                "Invalid port range format. Use start-end (e.g., 1-1024)."
            ) from e

    if exclude:
        try:
            exclude_ports = set(map(int, exclude.split(',')))
            ports = [port for port in ports if port not in exclude_ports]
        except ValueError as e:
            logger.error(
                "Invalid exclude ports format. Use comma-separated integers (e.g., 80,443)."
            )
            raise PortScannerError(
                "Invalid exclude ports format. Use comma-separated integers (e.g., 80,443)."
            ) from e

    return ports


def main() -> None:
    """
    Main function to coordinate the scanning process.
    """
    args = parse_arguments()
    try:
        ports = get_ports(args.ports, args.quick, args.exclude)
    except PortScannerError as e:
        logger.error(e)
        sys.exit(1)

    loop = asyncio.get_event_loop()

    tasks = []
    for ip in args.ips:
        ip = ip.strip()
        logger.info(f"Starting scan on {ip}")
        tasks.append(scan_ports(ip, ports, args.timeout,
                     args.protocol, args.verbose))

    try:
        results = loop.run_until_complete(
            asyncio.gather(*tasks, return_exceptions=True)
        )
    except Exception as e:
        logger.exception(f"An unexpected error occurred during scanning: {e}")
        sys.exit(1)

    for ip, scan_result in zip(args.ips, results):
        if isinstance(scan_result, Exception):
            logger.error(f"Error scanning {ip}: {scan_result}")
            continue
        save_results(ip, scan_result, args.output, args.json)


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        logger.warning("Scan interrupted by user.")
        sys.exit(0)
    except PortScannerError as e:
        logger.error(f"Port scanner error: {e}")
        sys.exit(1)
    except Exception as e:
        logger.exception(f"An unexpected error occurred: {e}")
        sys.exit(1)
