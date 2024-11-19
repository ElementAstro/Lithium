import pytest
import numpy as np
from numpy.testing import assert_array_almost_equal
from scipy.spatial import KDTree
from skimage.transform import estimate_transform
from .astroalign import find_transform, MaxIterError

# FILE: modules/lithium.pyimage/image/astroalign/test_astroalign.py

def test_find_transform_valid():
    source = np.array([[10, 10], [20, 20], [30, 30], [40, 40], [50, 50]])
    target = np.array([[12, 12], [22, 22], [32, 32], [42, 42], [52, 52]])
    transform, (src_pts, tgt_pts) = find_transform(source, target)
    assert transform is not None
    assert len(src_pts) == len(tgt_pts)
    assert_array_almost_equal(src_pts, source)
    assert_array_almost_equal(tgt_pts, target)

def test_find_transform_insufficient_control_points():
    source = np.array([[10, 10], [20, 20]])
    target = np.array([[12, 12], [22, 22]])
    with pytest.raises(ValueError, match="Reference stars in source or target image are less than the minimum value (3)."):
        find_transform(source, target)

def test_find_transform_minimal_control_points():
    source = np.array([[10, 10], [20, 20], [30, 30]])
    target = np.array([[12, 12], [22, 22], [32, 32]])
    transform, (src_pts, tgt_pts) = find_transform(source, target)
    assert transform is not None
    assert len(src_pts) == len(tgt_pts)
    assert_array_almost_equal(src_pts, source)
    assert_array_almost_equal(tgt_pts, target)

def test_find_transform_invalid_input_type():
    source = "invalid input"
    target = np.array([[12, 12], [22, 22], [32, 32]])
    with pytest.raises(TypeError, match="Input type for source not supported."):
        find_transform(source, target)

def test_find_transform_no_matches():
    source = np.array([[10, 10], [20, 20], [30, 30], [40, 40], [50, 50]])
    target = np.array([[100, 100], [200, 200], [300, 300], [400, 400], [500, 500]])
    with pytest.raises(MaxIterError, match="List of matching triangles exhausted before an acceptable transformation was found"):
        find_transform(source, target)