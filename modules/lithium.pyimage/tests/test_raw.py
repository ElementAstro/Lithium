import pytest
import numpy as np
import cv2
from pathlib import Path
from ..raw import RawImageProcessor, ImageFormat

# FILE: modules/lithium.pyimage/image/raw/test_raw.py


@pytest.fixture
def setup_raw_image(tmp_path):
    # Create a temporary directory with a test RAW image
    raw_image_path = tmp_path / "test_image.raw"
    # Create a dummy RAW image file (this should be replaced with an actual RAW file for real tests)
    with open(raw_image_path, 'wb') as f:
        f.write(b'\x00' * 1024)  # Dummy content
    return raw_image_path


def test_raw_image_processor_initialization(setup_raw_image):
    processor = RawImageProcessor(raw_path=setup_raw_image)
    assert processor.raw_path == setup_raw_image
    assert processor.rgb_image is not None
    assert processor.bgr_image is not None


def test_adjust_contrast(setup_raw_image):
    processor = RawImageProcessor(raw_path=setup_raw_image)
    original_image = processor.bgr_image.copy()
    processor.adjust_contrast(alpha=1.5)
    assert not np.array_equal(processor.bgr_image, original_image)


def test_adjust_brightness(setup_raw_image):
    processor = RawImageProcessor(raw_path=setup_raw_image)
    original_image = processor.bgr_image.copy()
    processor.adjust_brightness(beta=50)
    assert not np.array_equal(processor.bgr_image, original_image)


def test_apply_sharpening(setup_raw_image):
    processor = RawImageProcessor(raw_path=setup_raw_image)
    original_image = processor.bgr_image.copy()
    processor.apply_sharpening()
    assert not np.array_equal(processor.bgr_image, original_image)


def test_apply_gamma_correction(setup_raw_image):
    processor = RawImageProcessor(raw_path=setup_raw_image)
    original_image = processor.bgr_image.copy()
    processor.apply_gamma_correction(gamma=1.5)
    assert not np.array_equal(processor.bgr_image, original_image)


def test_rotate_image(setup_raw_image):
    processor = RawImageProcessor(raw_path=setup_raw_image)
    original_image = processor.bgr_image.copy()
    processor.rotate_image(angle=45)
    assert not np.array_equal(processor.bgr_image, original_image)


def test_resize_image(setup_raw_image):
    processor = RawImageProcessor(raw_path=setup_raw_image)
    original_image = processor.bgr_image.copy()
    processor.resize_image(width=200)
    assert processor.bgr_image.shape[1] == 200


def test_adjust_color_balance(setup_raw_image):
    processor = RawImageProcessor(raw_path=setup_raw_image)
    original_image = processor.bgr_image.copy()
    processor.adjust_color_balance(red=1.2, green=1.1, blue=0.9)
    assert not np.array_equal(processor.bgr_image, original_image)


def test_apply_blur(setup_raw_image):
    processor = RawImageProcessor(raw_path=setup_raw_image)
    original_image = processor.bgr_image.copy()
    processor.apply_blur(ksize=(5, 5), method="gaussian")
    assert not np.array_equal(processor.bgr_image, original_image)


def test_histogram_equalization(setup_raw_image):
    processor = RawImageProcessor(raw_path=setup_raw_image)
    original_image = processor.bgr_image.copy()
    processor.histogram_equalization()
    assert not np.array_equal(processor.bgr_image, original_image)


def test_convert_to_grayscale(setup_raw_image):
    processor = RawImageProcessor(raw_path=setup_raw_image)
    processor.convert_to_grayscale()
    assert len(processor.bgr_image.shape) == 2


def test_save_image(setup_raw_image, tmp_path):
    processor = RawImageProcessor(raw_path=setup_raw_image)
    output_path = tmp_path / "output_image.png"
    processor.save_image(output_path=output_path, file_format=ImageFormat.PNG)
    assert output_path.exists()


def test_reset(setup_raw_image):
    processor = RawImageProcessor(raw_path=setup_raw_image)
    processor.adjust_contrast(alpha=1.5)
    processor.reset()
    assert np.array_equal(processor.bgr_image, cv2.cvtColor(
        processor.rgb_image, cv2.COLOR_RGB2BGR))
