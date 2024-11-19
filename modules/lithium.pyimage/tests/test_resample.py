import pytest
import numpy as np
import cv2
from pathlib import Path
from ..resample import Resampler, ImageFormat

# FILE: modules/lithium.pyimage/image/resample/test_resample.py


@pytest.fixture
def setup_test_image(tmp_path):
    # Create a temporary directory with a test image
    img_dir = tmp_path / "images"
    img_dir.mkdir()
    image = np.ones((100, 100, 3), dtype=np.uint8) * 128
    img_path = img_dir / "test_image.jpg"
    cv2.imwrite(str(img_path), image)
    return img_path


def test_resampler_initialization(setup_test_image, tmp_path):
    output_path = tmp_path / "output_image.jpg"
    resampler = Resampler(
        input_image_path=setup_test_image,
        output_image_path=output_path
    )
    assert resampler.input_image_path == setup_test_image
    assert resampler.output_image_path == output_path


def test_adjust_brightness_contrast(setup_test_image, tmp_path):
    output_path = tmp_path / "output_image.jpg"
    resampler = Resampler(
        input_image_path=setup_test_image,
        output_image_path=output_path,
        brightness=1.5,
        contrast=1.5
    )
    resampler.process()
    processed_image = cv2.imread(str(output_path))
    assert processed_image is not None


def test_apply_sharpening(setup_test_image, tmp_path):
    output_path = tmp_path / "output_image.jpg"
    resampler = Resampler(
        input_image_path=setup_test_image,
        output_image_path=output_path,
        sharpen=True
    )
    resampler.process()
    processed_image = cv2.imread(str(output_path))
    assert processed_image is not None


def test_apply_blur(setup_test_image, tmp_path):
    output_path = tmp_path / "output_image.jpg"
    resampler = Resampler(
        input_image_path=setup_test_image,
        output_image_path=output_path,
        blur=(5, 5),
        blur_method='gaussian'
    )
    resampler.process()
    processed_image = cv2.imread(str(output_path))
    assert processed_image is not None


def test_histogram_equalization(setup_test_image, tmp_path):
    output_path = tmp_path / "output_image.jpg"
    resampler = Resampler(
        input_image_path=setup_test_image,
        output_image_path=output_path,
        histogram_equalization=True
    )
    resampler.process()
    processed_image = cv2.imread(str(output_path))
    assert processed_image is not None


def test_convert_to_grayscale(setup_test_image, tmp_path):
    output_path = tmp_path / "output_image.jpg"
    resampler = Resampler(
        input_image_path=setup_test_image,
        output_image_path=output_path,
        grayscale=True
    )
    resampler.process()
    processed_image = cv2.imread(str(output_path), cv2.IMREAD_GRAYSCALE)
    assert processed_image is not None
    assert len(processed_image.shape) == 2


def test_rotate_image(setup_test_image, tmp_path):
    output_path = tmp_path / "output_image.jpg"
    resampler = Resampler(
        input_image_path=setup_test_image,
        output_image_path=output_path,
        rotate_angle=45
    )
    resampler.process()
    processed_image = cv2.imread(str(output_path))
    assert processed_image is not None


def test_resize_image(setup_test_image, tmp_path):
    output_path = tmp_path / "output_image.jpg"
    resampler = Resampler(
        input_image_path=setup_test_image,
        output_image_path=output_path,
        width=200,
        height=200
    )
    resampler.process()
    processed_image = cv2.imread(str(output_path))
    assert processed_image.shape[1] == 200
    assert processed_image.shape[0] == 200


def test_process_invalid_input(tmp_path):
    invalid_image_path = tmp_path / "invalid_image.jpg"
    output_path = tmp_path / "output_image.jpg"
    resampler = Resampler(
        input_image_path=invalid_image_path,
        output_image_path=output_path
    )
    with pytest.raises(ValueError):
        resampler.process()
