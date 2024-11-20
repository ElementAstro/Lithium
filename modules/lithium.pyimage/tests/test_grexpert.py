import pytest
from pathlib import Path
from argparse import Namespace
from modules.lithium.pyimage.image.api.grexpert import validate_args, SerializationException


def test_validate_args_valid():
    args = Namespace(
        filenames=[Path("test_image.png")],
        operation="background-extraction",
        smoothing=0.5,
        strength=0.5,
        batch_size=16,
        correction="Subtraction"
    )
    validate_args(args)


def test_validate_args_non_existent_file():
    args = Namespace(
        filenames=[Path("non_existent_file.png")],
        operation="background-extraction"
    )
    with pytest.raises(SerializationException, match="Input file 'non_existent_file.png' does not exist."):
        validate_args(args)


def test_validate_args_unsupported_operation():
    args = Namespace(
        filenames=[Path("test_image.png")],
        operation="unsupported-operation"
    )
    with pytest.raises(SerializationException, match="Unsupported operation 'unsupported-operation'. Use 'background-extraction' or 'denoising'."):
        validate_args(args)


def test_validate_args_invalid_smoothing():
    args = Namespace(
        filenames=[Path("test_image.png")],
        operation="background-extraction",
        smoothing=1.5
    )
    with pytest.raises(SerializationException, match="Smoothing value out of range: 1.5"):
        validate_args(args)


def test_validate_args_invalid_strength():
    args = Namespace(
        filenames=[Path("test_image.png")],
        operation="background-extraction",
        strength=1.5
    )
    with pytest.raises(SerializationException, match="Strength value out of range: 1.5"):
        validate_args(args)


def test_validate_args_invalid_batch_size():
    args = Namespace(
        filenames=[Path("test_image.png")],
        operation="background-extraction",
        batch_size=33
    )
    with pytest.raises(SerializationException, match="Batch size out of range: 33"):
        validate_args(args)


def test_validate_args_invalid_correction_method():
    args = Namespace(
        filenames=[Path("test_image.png")],
        operation="background-extraction",
        correction="InvalidMethod"
    )
    with pytest.raises(SerializationException, match="Invalid correction method: InvalidMethod"):
        validate_args(args)


def test_validate_args_missing_strength_for_denoising():
    args = Namespace(
        filenames=[Path("test_image.png")],
        operation="denoising"
    )
    with pytest.raises(SerializationException, match="Missing 'strength' parameter for 'denoising' operation."):
        validate_args(args)
