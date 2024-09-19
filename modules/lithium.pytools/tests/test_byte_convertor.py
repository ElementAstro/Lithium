import pytest
import os
import zlib
from datetime import datetime
from tools.byte_convertor import convert_to_header


@pytest.fixture
def sample_binary_file(tmp_path):
    file_path = tmp_path / "sample.bin"
    data = bytes(range(256))  # Sample binary data
    with open(file_path, 'wb') as f:
        f.write(data)
    return file_path


@pytest.fixture
def expected_header_content():
    return (
        "/* Generated from sample.bin\n"
        "/* Original size: 256 bytes\n"
        "/* Compressed: No\n"
        f"/* Generated on: {datetime.now().strftime('%Y-%m-%d')}\n"
        "*/\n\n"
        "const unsigned char resource_data[] = { "
        + ', '.join(f'0x{b:02X}' for b in range(256))
        + " };\n"
        "const unsigned int resource_size = sizeof(resource_data);\n"
    )


def test_convert_to_header_basic(sample_binary_file, tmp_path, expected_header_content):
    output_header = tmp_path / "output.h"
    convert_to_header(
        input_file=str(sample_binary_file),
        output_header=str(output_header),
        array_name="resource_data",
        size_name="resource_size",
        array_type="unsigned char",
        comment_style="C",
        compress=False,
        format='hex',
        start=0,
        end=None,
        protect=True,
        cpp_class=False,
        split_size=None
    )
    with open(output_header, 'r', encoding="utf-8") as f:
        content = f.read()
    assert content == expected_header_content


def test_convert_to_header_compressed(sample_binary_file, tmp_path):
    output_header = tmp_path / "output_compressed.h"
    convert_to_header(
        input_file=str(sample_binary_file),
        output_header=str(output_header),
        array_name="resource_data",
        size_name="resource_size",
        array_type="unsigned char",
        comment_style="C",
        compress=True,
        format='hex',
        start=0,
        end=None,
        protect=True,
        cpp_class=False,
        split_size=None
    )
    with open(output_header, 'r', encoding="utf-8") as f:
        content = f.read()
    assert "resource_data_compressed" in content
    assert "Compressed: Yes" in content


def test_convert_to_header_bin_format(sample_binary_file, tmp_path):
    output_header = tmp_path / "output_bin.h"
    convert_to_header(
        input_file=str(sample_binary_file),
        output_header=str(output_header),
        array_name="resource_data",
        size_name="resource_size",
        array_type="unsigned char",
        comment_style="C",
        compress=False,
        format='bin',
        start=0,
        end=None,
        protect=True,
        cpp_class=False,
        split_size=None
    )
    with open(output_header, 'r', encoding="utf-8") as f:
        content = f.read()
    assert "0b" in content


def test_convert_to_header_no_protect(sample_binary_file, tmp_path):
    output_header = tmp_path / "output_no_protect.h"
    convert_to_header(
        input_file=str(sample_binary_file),
        output_header=str(output_header),
        array_name="resource_data",
        size_name="resource_size",
        array_type="unsigned char",
        comment_style="C",
        compress=False,
        format='hex',
        start=0,
        end=None,
        protect=False,
        cpp_class=False,
        split_size=None
    )
    with open(output_header, 'r') as f:
        content = f.read()
    assert "#ifndef" not in content


def test_convert_to_header_cpp_class(sample_binary_file, tmp_path):
    output_header = tmp_path / "output_cpp_class.h"
    convert_to_header(
        input_file=str(sample_binary_file),
        output_header=str(output_header),
        array_name="resource_data",
        size_name="resource_size",
        array_type="unsigned char",
        comment_style="C",
        compress=False,
        format='hex',
        start=0,
        end=None,
        protect=True,
        cpp_class=True,
        split_size=None
    )
    with open(output_header, 'r') as f:
        content = f.read()
    assert "class Resource_dataWrapper" in content


def test_convert_to_header_split(sample_binary_file, tmp_path):
    output_header = tmp_path / "output_split.h"
    convert_to_header(
        input_file=str(sample_binary_file),
        output_header=str(output_header),
        array_name="resource_data",
        size_name="resource_size",
        array_type="unsigned char",
        comment_style="C",
        compress=False,
        format='hex',
        start=0,
        end=None,
        protect=True,
        cpp_class=False,
        split_size=128
    )
    part1 = tmp_path / "output_split_part_0.h"
    part2 = tmp_path / "output_split_part_1.h"
    assert part1.exists()
    assert part2.exists()
    with open(part1, 'r') as f:
        content1 = f.read()
    with open(part2, 'r') as f:
        content2 = f.read()
    assert "resource_data_part_0" in content1
    assert "resource_data_part_1" in content2
