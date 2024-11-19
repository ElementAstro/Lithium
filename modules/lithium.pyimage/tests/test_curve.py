import pytest
import numpy as np
from pathlib import Path
from ..curve import CurvesTransformation

# FILE: modules/lithium.pyimage/image/transformation/test_curve.py


@pytest.fixture
def curve_transformation():
    # Initialize CurvesTransformation with default interpolation
    return CurvesTransformation()


@pytest.fixture(params=['cubic', 'akima', 'linear'])
def curve_transformation_with_valid_interpolation(request):
    # Initialize CurvesTransformation with valid interpolation methods
    return CurvesTransformation(interpolation=request.param)


@pytest.fixture
def curve_transformation_with_invalid_interpolation():
    # Initialize CurvesTransformation with an invalid interpolation method
    return CurvesTransformation(interpolation='invalid_method')


@pytest.fixture
def sample_image_grayscale():
    # Create a sample 2D grayscale image
    return np.linspace(0, 255, 256).reshape(16, 16).astype(np.uint8)


@pytest.fixture
def sample_image_rgb():
    # Create a sample 3D RGB image
    grayscale = np.linspace(0, 255, 256).reshape(16, 16).astype(np.uint8)
    return np.stack([grayscale, grayscale, grayscale], axis=-1)


@pytest.fixture
def temp_json_file(tmp_path):
    # Provide a temporary JSON file path
    return tmp_path / "test_curve.json"


def test_initialization_default(curve_transformation):
    assert curve_transformation.interpolation == 'akima'
    assert curve_transformation.curve is None
    assert curve_transformation.points == []


def test_initialization_with_valid_interpolation(curve_transformation_with_valid_interpolation):
    assert curve_transformation_with_valid_interpolation.interpolation in [
        'cubic', 'akima', 'linear']


def test_initialization_with_invalid_interpolation(curve_transformation_with_invalid_interpolation):
    with pytest.raises(ValueError):
        curve_transformation_with_invalid_interpolation._update_curve()


def test_add_point(curve_transformation):
    curve_transformation.add_point(0.0, 0.0)
    curve_transformation.add_point(1.0, 1.0)
    assert curve_transformation.points == [(0.0, 0.0), (1.0, 1.0)]
    assert curve_transformation.curve is not None


def test_remove_point(curve_transformation):
    curve_transformation.add_point(0.0, 0.0)
    curve_transformation.add_point(1.0, 1.0)
    curve_transformation.remove_point(0)
    assert curve_transformation.points == [(1.0, 1.0)]
    assert curve_transformation.curve is None  # Not enough points


def test_remove_point_invalid_index(curve_transformation):
    curve_transformation.add_point(0.0, 0.0)
    with pytest.raises(IndexError):
        curve_transformation.remove_point(5)


def test_update_curve_with_insufficient_points(curve_transformation):
    curve_transformation.add_point(0.0, 0.0)
    assert curve_transformation.curve is None


def test_transform_grayscale_image(curve_transformation_with_valid_interpolation, sample_image_grayscale):
    curve = curve_transformation_with_valid_interpolation
    curve.add_point(0.0, 0.0)
    curve.add_point(1.0, 255.0)
    transformed_image = curve.transform(sample_image_grayscale)
    assert transformed_image.shape == sample_image_grayscale.shape
    assert transformed_image.dtype == np.uint8
    # Identity transformation
    assert np.all(transformed_image == sample_image_grayscale)


def test_transform_rgb_image_channel(curve_transformation_with_valid_interpolation, sample_image_rgb):
    curve = curve_transformation_with_valid_interpolation
    curve.add_point(0.0, 0.0)
    curve.add_point(1.0, 255.0)
    transformed_image = curve.transform(sample_image_rgb, channel=1)
    assert transformed_image.shape == sample_image_rgb.shape
    assert transformed_image.dtype == np.uint8
    # Unchanged channels
    assert np.all(transformed_image[:, :, 0] == sample_image_rgb[:, :, 0])
    assert np.all(transformed_image[:, :, 1] == sample_image_rgb[:, :, 1])
    assert np.all(transformed_image[:, :, 2] == sample_image_rgb[:, :, 2])


def test_transform_without_defining_curve(curve_transformation, sample_image_grayscale):
    curve = curve_transformation
    with pytest.raises(ValueError):
        curve.transform(sample_image_grayscale)


def test_invert_curve(curve_transformation_with_valid_interpolation, sample_image_grayscale):
    curve = curve_transformation_with_valid_interpolation
    curve.add_point(0.0, 0.0)
    curve.add_point(1.0, 1.0)
    curve.invert_curve()
    transformed_image = curve.transform(sample_image_grayscale)
    expected_image = 255 - sample_image_grayscale
    assert np.array_equal(transformed_image, expected_image)


def test_store_and_restore_curve(curve_transformation_with_valid_interpolation, sample_image_grayscale):
    curve = curve_transformation_with_valid_interpolation
    curve.add_point(0.0, 0.0)
    curve.add_point(0.5, 128.0)
    curve.add_point(1.0, 255.0)
    curve.store_curve()
    curve.add_point(0.25, 64.0)
    assert len(curve.points) == 4
    curve.restore_curve()
    assert len(curve.points) == 3
    assert curve.points == [(0.0, 0.0), (0.5, 128.0), (1.0, 255.0)]
    transformed_image = curve.transform(sample_image_grayscale)
    assert transformed_image.shape == sample_image_grayscale.shape


def test_reset_curve(curve_transformation_with_valid_interpolation, sample_image_grayscale):
    curve = curve_transformation_with_valid_interpolation
    curve.add_point(0.0, 0.0)
    curve.add_point(0.5, 200.0)
    curve.reset_curve()
    assert curve.points == [(0.0, 0.0), (1.0, 1.0)]
    transformed_image = curve.transform(sample_image_grayscale)
    normalized = sample_image_grayscale.astype(np.float32) / 255.0
    expected = (normalized * 255).astype(np.uint8)
    # Identity transformation
    assert np.array_equal(transformed_image, expected)


def test_save_curve(curve_transformation_with_valid_interpolation, temp_json_file):
    curve = curve_transformation_with_valid_interpolation
    curve.add_point(0.0, 0.0)
    curve.add_point(1.0, 255.0)
    curve.save_curve(temp_json_file)
    assert temp_json_file.exists()
    with open(temp_json_file, 'r') as f:
        data = json.load(f)
    assert data['interpolation'] == curve.interpolation
    assert data['points'] == curve.points


def test_load_curve(curve_transformation_with_valid_interpolation, temp_json_file):
    # Prepare a curve file
    data = {
        'interpolation': 'linear',
        'points': [(0.0, 0.0), (0.5, 128.0), (1.0, 255.0)]
    }
    with open(temp_json_file, 'w') as f:
        json.dump(data, f, indent=4)
    curve = curve_transformation_with_valid_interpolation
    curve.load_curve(temp_json_file)
    assert curve.interpolation == 'linear'
    assert curve.points == [(0.0, 0.0), (0.5, 128.0), (1.0, 255.0)]


def test_pixel_readout(curve_transformation_with_valid_interpolation):
    curve = curve_transformation_with_valid_interpolation
    curve.add_point(0.0, 0.0)
    curve.add_point(1.0, 255.0)
    value = curve.pixel_readout(0.5)
    assert value == 127.5  # Linear interpolation


def test_pixel_readout_no_curve(curve_transformation):
    value = curve_transformation.pixel_readout(0.5)
    assert value is None


def test_pixel_readout_invalid_input(curve_transformation_with_valid_interpolation):
    curve = curve_transformation_with_valid_interpolation
    curve.add_point(0.0, 0.0)
    curve.add_point(1.0, 255.0)
    with pytest.raises(ValueError):
        curve.pixel_readout("invalid_input")  # Non-float input


def test_export_curve_points(curve_transformation_with_valid_interpolation):
    curve = curve_transformation_with_valid_interpolation
    points = [(0.0, 0.0), (0.5, 128.0), (1.0, 255.0)]
    for x, y in points:
        curve.add_point(x, y)
    exported_points = curve.export_curve_points()
    assert exported_points == points


def test_import_curve_points(curve_transformation_with_valid_interpolation):
    curve = curve_transformation_with_valid_interpolation
    points = [(0.0, 0.0), (0.25, 64.0), (0.75, 192.0), (1.0, 255.0)]
    curve.import_curve_points(points)
    assert curve.points == sorted(points, key=lambda point: point[0])
    transformed_image = curve.transform(
        np.array([[0, 128, 255]], dtype=np.uint8))
    expected = np.array([[0, 128, 255]], dtype=np.uint8)
    assert np.array_equal(transformed_image, expected)


def test_get_interpolation_methods(curve_transformation_with_valid_interpolation):
    methods = curve_transformation_with_valid_interpolation.get_interpolation_methods()
    assert methods == ['cubic', 'akima', 'linear']


def test_transform_invalid_image_format(curve_transformation_with_valid_interpolation):
    curve = curve_transformation_with_valid_interpolation
    curve.add_point(0.0, 0.0)
    curve.add_point(1.0, 255.0)
    invalid_image = np.zeros((16, 16, 16), dtype=np.uint8)  # Unsupported shape
    with pytest.raises(ValueError):
        curve.transform(invalid_image)


def test_transform_rgb_image_without_channel(curve_transformation_with_valid_interpolation, sample_image_rgb):
    curve = curve_transformation_with_valid_interpolation
    curve.add_point(0.0, 0.0)
    curve.add_point(1.0, 255.0)
    with pytest.raises(ValueError):
        curve.transform(sample_image_rgb)  # Missing channel parameter


def test_load_curve_invalid_file(curve_transformation_with_valid_interpolation, tmp_path):
    invalid_file = tmp_path / "invalid_curve.json"
    with open(invalid_file, 'w') as f:
        f.write("Invalid JSON content")
    with pytest.raises(json.JSONDecodeError):
        curve_transformation_with_valid_interpolation.load_curve(invalid_file)


def test_load_curve_missing_file(curve_transformation_with_valid_interpolation):
    with pytest.raises(FileNotFoundError):
        curve_transformation_with_valid_interpolation.load_curve(
            "non_existent_curve.json")
