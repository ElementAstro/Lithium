import matplotlib.pyplot as plt
import cv2
import numpy as np
from .stretch import AdaptiveStretch
from typing import Optional, Tuple

def apply_real_time_preview(image: np.ndarray, noise_threshold: float = 1e-4, contrast_protection: Optional[float] = None, max_curve_points: int = 106, roi: Optional[Tuple[int, int, int, int]] = None):
    """
    Simulate real-time preview by iteratively applying the adaptive stretch
    transformation and displaying the result.

    :param image: Input image as a numpy array (grayscale or color).
    :param noise_threshold: Threshold for treating brightness differences as noise.
    :param contrast_protection: Optional contrast protection parameter.
    :param max_curve_points: Maximum points for the transformation curve.
    :param roi: Tuple (x, y, width, height) defining the region of interest.
    """
    adaptive_stretch = AdaptiveStretch(noise_threshold, contrast_protection, max_curve_points)
    preview_image = adaptive_stretch.stretch(image, roi)

    if len(preview_image.shape) == 3:
        preview_image = cv2.cvtColor(preview_image, cv2.COLOR_BGR2RGB)

    plt.imshow(preview_image, cmap='gray' if len(preview_image.shape) == 2 else None)
    plt.title(f"Noise Threshold: {noise_threshold}, Contrast Protection: {contrast_protection}")
    plt.show()
