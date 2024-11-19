import pytest
import numpy as np
from ..defect_correction import defect_map_enhanced

# FILE: modules/lithium.pyimage/image/defect_map/test_defect_correction.py


@pytest.fixture
def setup_test_image():
    # Create a test image and defect map
    image = np.ones((10, 10), dtype=np.float32) * 100
    defect_map = np.ones((10, 10), dtype=np.float32)
    defect_map[5, 5] = 0  # Introduce a defect
    return image, defect_map


def test_defect_map_enhanced_mean(setup_test_image):
    image, defect_map = setup_test_image
    corrected_image = defect_map_enhanced(image, defect_map, operation='mean')
    assert corrected_image[5, 5] != 100  # Defect should be corrected


def test_defect_map_enhanced_gaussian(setup_test_image):
    image, defect_map = setup_test_image
    corrected_image = defect_map_enhanced(
        image, defect_map, operation='gaussian')
    assert corrected_image[5, 5] != 100  # Defect should be corrected


def test_defect_map_enhanced_minimum(setup_test_image):
    image, defect_map = setup_test_image
    corrected_image = defect_map_enhanced(
        image, defect_map, operation='minimum')
    assert corrected_image[5, 5] != 100  # Defect should be corrected


def test_defect_map_enhanced_maximum(setup_test_image):
    image, defect_map = setup_test_image
    corrected_image = defect_map_enhanced(
        image, defect_map, operation='maximum')
    assert corrected_image[5, 5] != 100  # Defect should be corrected


def test_defect_map_enhanced_median(setup_test_image):
    image, defect_map = setup_test_image
    corrected_image = defect_map_enhanced(
        image, defect_map, operation='median')
    assert corrected_image[5, 5] != 100  # Defect should be corrected


def test_defect_map_enhanced_bilinear(setup_test_image):
    image, defect_map = setup_test_image
    corrected_image = defect_map_enhanced(
        image, defect_map, operation='bilinear')
    assert corrected_image[5, 5] != 100  # Defect should be corrected


def test_defect_map_enhanced_bicubic(setup_test_image):
    image, defect_map = setup_test_image
    corrected_image = defect_map_enhanced(
        image, defect_map, operation='bicubic')
    assert corrected_image[5, 5] != 100  # Defect should be corrected


def test_defect_map_enhanced_square_structure(setup_test_image):
    image, defect_map = setup_test_image
    corrected_image = defect_map_enhanced(
        image, defect_map, structure='square')
    assert corrected_image[5, 5] != 100  # Defect should be corrected


def test_defect_map_enhanced_circular_structure(setup_test_image):
    image, defect_map = setup_test_image
    corrected_image = defect_map_enhanced(
        image, defect_map, structure='circular')
    assert corrected_image[5, 5] != 100  # Defect should be corrected


def test_defect_map_enhanced_horizontal_structure(setup_test_image):
    image, defect_map = setup_test_image
    corrected_image = defect_map_enhanced(
        image, defect_map, structure='horizontal')
    assert corrected_image[5, 5] != 100  # Defect should be corrected


def test_defect_map_enhanced_vertical_structure(setup_test_image):
    image, defect_map = setup_test_image
    corrected_image = defect_map_enhanced(
        image, defect_map, structure='vertical')
    assert corrected_image[5, 5] != 100  # Defect should be corrected


def test_defect_map_enhanced_with_edge_protection(setup_test_image):
    image, defect_map = setup_test_image
    corrected_image = defect_map_enhanced(
        image, defect_map, protect_edges=True)
    assert corrected_image[5, 5] != 100  # Defect should be corrected


def test_defect_map_enhanced_without_edge_protection(setup_test_image):
    image, defect_map = setup_test_image
    corrected_image = defect_map_enhanced(
        image, defect_map, protect_edges=False)
    assert corrected_image[5, 5] != 100  # Defect should be corrected


def test_defect_map_enhanced_with_adaptive_structure(setup_test_image):
    image, defect_map = setup_test_image
    corrected_image = defect_map_enhanced(
        image, defect_map, adaptive_structure=True)
    assert corrected_image[5, 5] != 100  # Defect should be corrected


def test_defect_map_enhanced_without_adaptive_structure(setup_test_image):
    image, defect_map = setup_test_image
    corrected_image = defect_map_enhanced(
        image, defect_map, adaptive_structure=False)
    assert corrected_image[5, 5] != 100  # Defect should be corrected


def test_defect_map_enhanced_with_cfa(setup_test_image):
    image, defect_map = setup_test_image
    # Create a dummy RGB image
    image = np.stack([image, image, image], axis=-1)
    defect_map = np.stack([defect_map, defect_map, defect_map], axis=-1)
    corrected_image = defect_map_enhanced(image, defect_map, is_cfa=True)
    assert corrected_image[5, 5, 0] != 100  # Defect should be corrected
    assert corrected_image[5, 5, 1] != 100  # Defect should be corrected
    assert corrected_image[5, 5, 2] != 100  # Defect should be corrected
