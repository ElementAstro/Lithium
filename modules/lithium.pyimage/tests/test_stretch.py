import pytest
import numpy as np
import cv2
from pathlib import Path
from ..image.adaptive_stretch.stretch import AdaptiveStretch

# FILE: modules/lithium.pyimage/image/adaptive_stretch/test_stretch.py


def test_adaptive_stretch_initialization():
    stretcher = AdaptiveStretch(noise_threshold=0.01, contrast_protection=0.5, max_curve_points=50, roi=(
        10, 10, 50, 50), save_intermediate=True, intermediate_dir=Path("test_intermediate"))
    assert stretcher.noise_threshold == 0.01
    assert stretcher.contrast_protection == 0.5
    assert stretcher.max_curve_points == 50
    assert stretcher.roi == (10, 10, 50, 50)
    assert stretcher.save_intermediate is True
    assert stretcher.intermediate_dir == Path("test_intermediate")


def test_compute_brightness_diff():
    stretcher = AdaptiveStretch()
    image = np.array([[1, 2, 3], [4, 5, 6], [7, 8, 9]], dtype=np.uint8)
    diff_x, diff_y = stretcher.compute_brightness_diff(image)
    expected_diff_x = np.array(
        [[1, 1, 0], [1, 1, 0], [1, 1, 0]], dtype=np.int8)
    expected_diff_y = np.array(
        [[3, 3, 3], [3, 3, 3], [0, 0, 0]], dtype=np.int8)
    assert np.array_equal(diff_x, expected_diff_x)
    assert np.array_equal(diff_y, expected_diff_y)


def test_stretch_grayscale_image():
    stretcher = AdaptiveStretch()
    image = np.ones((100, 100), dtype=np.uint8) * 128
    stretched_image = stretcher.stretch(image)
    assert stretched_image.shape == image.shape
    assert stretched_image.dtype == np.uint8


def test_stretch_color_image():
    stretcher = AdaptiveStretch()
    image = np.ones((100, 100, 3), dtype=np.uint8) * 128
    stretched_image = stretcher.stretch(image)
    assert stretched_image.shape == image.shape
    assert stretched_image.dtype == np.uint8


def test_stretch_with_roi():
    stretcher = AdaptiveStretch(roi=(10, 10, 50, 50))
    image = np.ones((100, 100), dtype=np.uint8) * 128
    stretched_image = stretcher.stretch(image)
    assert stretched_image.shape == image.shape
    assert stretched_image.dtype == np.uint8


def test_save_intermediate_results(tmp_path):
    intermediate_dir = tmp_path / "intermediate"
    stretcher = AdaptiveStretch(
        save_intermediate=True, intermediate_dir=intermediate_dir)
    image = np.ones((100, 100), dtype=np.uint8) * 128
    stretcher.stretch(image)
    assert intermediate_dir.exists()
    assert len(list(intermediate_dir.glob("*.png"))) > 0
