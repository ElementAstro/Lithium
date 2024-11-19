import pytest
import numpy as np
from pathlib import Path
from ..debayer import Debayer, DebayerConfig

# FILE: modules/lithium.pyimage/image/debayer/test_debayer.py


@pytest.fixture
def sample_cfa_image():
    # Create a dummy CFA image (10x10) with a simple pattern
    return np.tile(np.array([[0, 1], [1, 0]], dtype=np.uint8), (5, 5))


@pytest.fixture
def debayer_config():
    # Provide a default DebayerConfig instance
    return DebayerConfig()


@pytest.fixture
def debayer_instance(debayer_config):
    # Provide a Debayer instance initialized with the default configuration
    return Debayer(config=debayer_config)


def test_initialization_default():
    debayer = Debayer()
    assert debayer.config.method == 'bilinear'
    assert debayer.config.pattern is None
    assert debayer.config.num_threads == 4
    assert not debayer.config.visualize_intermediate
    assert debayer.config.visualization_save_path is None
    assert not debayer.config.save_debayered_images


def test_initialization_custom():
    config = DebayerConfig(method='vng', pattern='RGGB', num_threads=2)
    debayer = Debayer(config=config)
    assert debayer.config.method == 'vng'
    assert debayer.config.pattern == 'RGGB'
    assert debayer.config.num_threads == 2


def test_detect_bayer_pattern(debayer_instance, sample_cfa_image):
    pattern = debayer_instance.detect_bayer_pattern(sample_cfa_image)
    assert pattern in ['BGGR', 'RGGB', 'GBRG', 'GRBG']


def test_debayer_superpixel(debayer_instance, sample_cfa_image):
    debayer_instance.config.pattern = 'BGGR'
    rgb_image = debayer_instance.debayer_superpixel(sample_cfa_image)
    assert rgb_image.shape == (5, 5, 3)  # Superpixel reduces size by half


def test_debayer_bilinear(debayer_instance, sample_cfa_image):
    debayer_instance.config.pattern = 'BGGR'
    rgb_image = debayer_instance.debayer_bilinear(sample_cfa_image)
    assert rgb_image.shape == (10, 10, 3)


def test_debayer_vng(debayer_instance, sample_cfa_image):
    debayer_instance.config.pattern = 'BGGR'
    rgb_image = debayer_instance.debayer_vng(sample_cfa_image)
    assert rgb_image.shape == (10, 10, 3)


def test_parallel_debayer_ahd(debayer_instance, sample_cfa_image):
    debayer_instance.config.pattern = 'BGGR'
    rgb_image = debayer_instance.parallel_debayer_ahd(sample_cfa_image)
    assert rgb_image.shape == (10, 10, 3)


def test_debayer_laplacian_harmonization(debayer_instance, sample_cfa_image):
    debayer_instance.config.pattern = 'BGGR'
    rgb_image = debayer_instance.debayer_laplacian_harmonization(
        sample_cfa_image)
    assert rgb_image.shape == (10, 10, 3)


def test_unsupported_bayer_pattern(debayer_instance, sample_cfa_image):
    debayer_instance.config.pattern = 'INVALID'
    with pytest.raises(ValueError):
        debayer_instance.debayer_bilinear(sample_cfa_image)


def test_unsupported_debayer_method(debayer_instance, sample_cfa_image):
    debayer_instance.config.method = 'unsupported_method'
    with pytest.raises(ValueError):
        debayer_instance.debayer_image(sample_cfa_image)


def test_visualize_intermediate_steps(debayer_instance, sample_cfa_image, tmp_path):
    debayer_instance.config.visualize_intermediate = True
    debayer_instance.config.visualization_save_path = tmp_path / "intermediate_steps.png"
    debayer_instance.config.pattern = 'BGGR'
    debayered_image = debayer_instance.debayer_bilinear(sample_cfa_image)
    Debayer.visualize_intermediate_steps(
        "dummy_path", debayered_image, debayer_instance.config)
    assert debayer_instance.config.visualization_save_path.exists()


def test_save_debayered_images(debayer_instance, sample_cfa_image, tmp_path):
    debayer_instance.config.save_debayered_images = True
    debayer_instance.config.debayered_save_path_template = str(
        tmp_path / "{original_name}_{method}.png")
    debayer_instance.config.pattern = 'BGGR'
    debayer_instance.debayer_image(sample_cfa_image)
    expected_path = tmp_path / "debayered_image_bilinear.png"
    assert expected_path.exists()
