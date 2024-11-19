import pytest
import shutil
from pathlib import Path
from PIL import Image
from ..combination import batch_process

# FILE: modules/lithium.pyimage/image/channel/test_combination.py


@pytest.fixture
def setup_test_images(tmp_path):
    # Create a temporary directory with test images
    img_dir = tmp_path / "images"
    img_dir.mkdir()
    for i in range(3):
        for ch in ['R', 'G', 'B']:
            img = Image.new('L', (100, 100), color=i*85)
            img.save(img_dir / f"test_{i}_{ch}.png")
    return img_dir


def test_batch_process_valid_images(setup_test_images, tmp_path):
    output_dir = tmp_path / "output"
    batch_process(setup_test_images, "RGB", output_dir, "PNG")
    assert len(list(output_dir.glob("*.png"))) == 3


def test_batch_process_missing_channel_images(setup_test_images, tmp_path):
    # Remove one channel image to simulate missing channel
    missing_image = setup_test_images / "test_1_G.png"
    missing_image.unlink()
    output_dir = tmp_path / "output"
    batch_process(setup_test_images, "RGB", output_dir, "PNG")
    assert len(list(output_dir.glob("*.png"))) == 2


def test_batch_process_different_color_spaces(setup_test_images, tmp_path):
    output_dir = tmp_path / "output"
    batch_process(setup_test_images, "HSV", output_dir, "PNG")
    assert len(list(output_dir.glob("*.png"))) == 3


def test_batch_process_custom_channel_mapping(setup_test_images, tmp_path):
    output_dir = tmp_path / "output"
    batch_process(setup_test_images, "RGB", output_dir,
                  "PNG", mapping=['B', 'G', 'R'])
    assert len(list(output_dir.glob("*.png"))) == 3


def test_batch_process_invalid_directory(tmp_path):
    invalid_dir = tmp_path / "invalid"
    output_dir = tmp_path / "output"
    with pytest.raises(FileNotFoundError):
        batch_process(invalid_dir, "RGB", output_dir, "PNG")
