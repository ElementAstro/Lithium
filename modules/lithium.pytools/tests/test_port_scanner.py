import pytest
import asyncio
import json
import os
from tools.port_scanner import scan_port, scan_ports, save_results, parse_arguments, get_ports


@pytest.fixture
def ip():
    return "127.0.0.1"


@pytest.fixture
def ports():
    return range(80, 83)


@pytest.fixture
def timeout():
    return 1.0


@pytest.fixture
def output_file(tmp_path):
    return tmp_path / "scan_results.txt"


@pytest.fixture
def json_output_file(tmp_path):
    return tmp_path / "scan_results.json"


@pytest.mark.asyncio
async def test_scan_port_tcp(ip, timeout):
    port = 80
    result = await scan_port(ip, port, timeout, protocol='tcp', verbose=False)
    assert result == (port, 'closed', '')


@pytest.mark.asyncio
async def test_scan_port_udp(ip, timeout):
    port = 80
    result = await scan_port(ip, port, timeout, protocol='udp', verbose=False)
    assert result == (port, 'closed', '')


@pytest.mark.asyncio
async def test_scan_ports(ip, ports, timeout):
    results = await scan_ports(ip, ports, timeout, protocol='tcp', verbose=False)
    assert len(results) == len(ports)
    for port, status, service in results:
        assert status == 'closed'


def test_save_results_text(ip, ports, output_file):
    results = [(port, 'closed', '') for port in ports]
    save_results(ip, results, str(output_file), json_format=False)
    with open(output_file, 'r', encoding="utf-8") as f:
        content = f.read()
    assert content == "127.0.0.1:80 closed \n127.0.0.1:81 closed \n127.0.0.1:82 closed \n"


def test_save_results_json(ip, ports, json_output_file):
    results = [(port, 'closed', '') for port in ports]
    save_results(ip, results, str(json_output_file), json_format=True)
    with open(json_output_file, 'r', encoding="utf-8") as f:
        content = json.load(f)
    assert content == {ip: results}


def test_parse_arguments(monkeypatch):
    monkeypatch.setattr('sys.argv', [
                        'port_scanner.py', '127.0.0.1', '-p', '80-82', '-t', '1.0', '--protocol', 'tcp'])
    args = parse_arguments()
    assert args.ips == ['127.0.0.1']
    assert args.ports == '80-82'
    assert args.timeout == 1.0
    assert args.protocol == 'tcp'


def test_get_ports():
    ports = get_ports("80-82", quick=False, exclude="")
    assert list(ports) == [80, 81, 82]


def test_get_ports_with_exclude():
    ports = get_ports("80-82", quick=False, exclude="81")
    assert list(ports) == [80, 82]
