# FILE: modules/lithium.pytarget/target/test_calc_fov.py


def test_calc_fov_points_valid():
    x_pixels = 4000
    x_pixel_size = 5.0  # micrometers
    y_pixels = 3000
    y_pixel_size = 5.0  # micrometers
    focal_length = 1000  # millimeters
    target_ra = 10.0  # degrees
    target_dec = 20.0  # degrees
    camera_rotation = 0.0  # degrees

    corners = calc_fov_points(x_pixels, x_pixel_size, y_pixels, y_pixel_size,
                              focal_length, target_ra, target_dec, camera_rotation)

    assert len(corners) == 4
    for corner in corners:
        assert isinstance(corner, tuple)
        assert len(corner) == 2
        assert isinstance(corner[0], float)
        assert isinstance(corner[1], float)


def test_calc_fov_points_negative_focal_length():
    x_pixels = 4000
    x_pixel_size = 5.0  # micrometers
    y_pixels = 3000
    y_pixel_size = 5.0  # micrometers
    focal_length = -1000  # millimeters (invalid)
    target_ra = 10.0  # degrees
    target_dec = 20.0  # degrees
    camera_rotation = 0.0  # degrees

    with pytest.raises(ValueError, match="Focal length must be a positive value."):
        calc_fov_points(x_pixels, x_pixel_size, y_pixels, y_pixel_size,
                        focal_length, target_ra, target_dec, camera_rotation)


def test_calc_fov_points_zero_focal_length():
    x_pixels = 4000
    x_pixel_size = 5.0  # micrometers
    y_pixels = 3000
    y_pixel_size = 5.0  # micrometers
    focal_length = 0  # millimeters (invalid)
    target_ra = 10.0  # degrees
    target_dec = 20.0  # degrees
    camera_rotation = 0.0  # degrees

    with pytest.raises(ValueError, match="Focal length must be a positive value."):
        calc_fov_points(x_pixels, x_pixel_size, y_pixels, y_pixel_size,
                        focal_length, target_ra, target_dec, camera_rotation)


def test_calc_fov_points_edge_case():
    x_pixels = 1
    x_pixel_size = 1.0  # micrometers
    y_pixels = 1
    y_pixel_size = 1.0  # micrometers
    focal_length = 1  # millimeters
    target_ra = 0.0  # degrees
    target_dec = 0.0  # degrees
    camera_rotation = 0.0  # degrees

    corners = calc_fov_points(x_pixels, x_pixel_size, y_pixels, y_pixel_size,
                              focal_length, target_ra, target_dec, camera_rotation)

    assert len(corners) == 4
    for corner in corners:
        assert isinstance(corner, tuple)
        assert len(corner) == 2
        assert isinstance(corner[0], float)
        assert isinstance(corner[1], float)


def test_calc_fov_points_with_rotation():
    x_pixels = 4000
    x_pixel_size = 5.0  # micrometers
    y_pixels = 3000
    y_pixel_size = 5.0  # micrometers
    focal_length = 1000  # millimeters
    target_ra = 10.0  # degrees
    target_dec = 20.0  # degrees
    camera_rotation = 45.0  # degrees

    corners = calc_fov_points(x_pixels, x_pixel_size, y_pixels, y_pixel_size,
                              focal_length, target_ra, target_dec, camera_rotation)

    assert len(corners) == 4
    for corner in corners:
        assert isinstance(corner, tuple)
        assert len(corner) == 2
