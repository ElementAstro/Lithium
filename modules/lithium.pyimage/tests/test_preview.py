import pytest
import numpy as np
import cv2
from pathlib import Path
from ..image.adaptive_stretch.preview import apply_real_time_preview

# FILE: modules/lithium.pyimage/image/adaptive_stretch/test_preview.py


def test_apply_real_time_preview_grayscale():
    image = np.ones((100, 100), dtype=np.uint8) * 128
    processed_image = apply_real_time_preview(image)
    assert processed_image.shape == image.shape
    assert processed_image.dtype == np.uint8


def test_apply_real_time_preview_color():
    image = np.ones((100, 100, 3), dtype=np.uint8) * 128
    processed_image = apply_real_time_preview(image)
    assert processed_image.shape == image.shape
    assert processed_image.dtype == np.uint8


def test_apply_real_time_preview_with_noise_threshold():
    image = np.ones((100, 100), dtype=np.uint8) * 128
    processed_image = apply_real_time_preview(image, noise_threshold=0.01)
    assert processed_image.shape == image.shape
    assert processed_image.dtype == np.uint8


def test_apply_real_time_preview_with_contrast_protection():
    image = np.ones((100, 100), dtype=np.uint8) * 128
    processed_image = apply_real_time_preview(image, contrast_protection=0.5)
    assert processed_image.shape == image.shape
    assert processed_image.dtype == np.uint8


def test_apply_real_time_preview_with_roi():
    image = np.ones((100, 100), dtype=np.uint8) * 128
    processed_image = apply_real_time_preview(image, roi=(10, 10, 50, 50))
    assert processed_image.shape == image.shape
    assert processed_image.dtype == np.uint8
