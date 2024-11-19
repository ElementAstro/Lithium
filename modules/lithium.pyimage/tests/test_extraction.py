import pytest
import numpy as np
import cv2
from pathlib import Path
from ..extraction import extract_channels, merge_channels, process_directory

# FILE: modules/lithium.pyimage/image/channel/test_extraction.py


@pytest.fixture
def setup_test_image(tmp_path):
    # Create a temporary directory with a test image
    img_dir = tmp_path / "images"
    img_dir.mkdir()
    image = np.ones((100, 100, 3), dtype=np.uint8) * 128
    img_path = img_dir / "test_image.png"
    cv2.imwrite(str(img_path), image)
    return img_path


def test_extract_channels_rgb(setup_test_image):
    image = cv2.imread(str(setup_test_image))
    channels = extract_channels(image, color_space='RGB')
    assert 'R' in channels
    assert 'G' in channels
    assert 'B' in channels
    assert channels['R'].shape == (100, 100)
    assert channels['G'].shape == (100, 100)
    assert channels['B'].shape == (100, 100)


def test_extract_channels_hsv(setup_test_image):
    image = cv2.imread(str(setup_test_image))
    channels = extract_channels(image, color_space='HSV')
    assert 'H' in channels
    assert 'S' in channels
    assert 'V' in channels
    assert channels['H'].shape == (100, 100)
    assert channels['S'].shape == (100, 100)
    assert channels['V'].shape == (100, 100)


def test_merge_channels_rgb(setup_test_image):
    image = cv2.imread(str(setup_test_image))
    channels = extract_channels(image, color_space='RGB')
    merged_image = merge_channels(channels)
    assert merged_image is not None
    assert merged_image.shape == (100, 100, 3)


def test_merge_channels_hsv(setup_test_image):
    image = cv2.imread(str(setup_test_image))
    channels = extract_channels(image, color_space='HSV')
    merged_image = merge_channels(channels)
    assert merged_image is not None
    assert merged_image.shape == (100, 100, 3)


@pytest.fixture
def setup_test_directory(tmp_path):
    # Create a temporary directory with test images
    img_dir = tmp_path / "images"
    img_dir.mkdir()
    for i in range(3):
        image = np.ones((100, 100, 3), dtype=np.uint8) * (i * 85)
        img_path = img_dir / f"test_image_{i}.png"
        cv2.imwrite(str(img_path), image)
    return img_dir


def test_process_directory(setup_test_directory, tmp_path):
    output_dir = tmp_path / "output"
    process_directory(setup_test_directory, output_dir, color_space='RGB')
    assert len(list(output_dir.glob("*.png"))) == 9  # 3 images * 3 channels


def test_process_directory_invalid_input(tmp_path):
    invalid_dir = tmp_path / "invalid"
    output_dir = tmp_path / "output"
    with pytest.raises(NotADirectoryError):
        process_directory(invalid_dir, output_dir, color_space='RGB')
