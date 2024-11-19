import numpy as np
from scipy.interpolate import griddata

def interpolate_defects(image: np.ndarray, mask: np.ndarray, method: str = 'linear') -> np.ndarray:
    """
    Interpolate defective pixels in an image.

    Parameters:
    - image: np.ndarray, Image data.
    - mask: np.ndarray, Defect mask indicating defective pixels.
    - method: str, Interpolation method ('linear', 'cubic').

    Returns:
    - interpolated_image: np.ndarray, Image with interpolated defective pixels.
    """
    x, y = np.indices(image.shape)
    points = np.column_stack((x[~mask], y[~mask]))
    values = image[~mask]
    return griddata(points, values, (x, y), method=method)
