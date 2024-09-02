import numpy as np
import cv2
from typing import Optional, Tuple


class AdaptiveStretch:
    def __init__(self, noise_threshold: float = 1e-4, contrast_protection: Optional[float] = None, max_curve_points: int = 106):
        """
        Initialize the AdaptiveStretch object with specific parameters.

        :param noise_threshold: Threshold for treating brightness differences as noise.
        :param contrast_protection: Optional contrast protection parameter.
        :param max_curve_points: Maximum points for the transformation curve.
        """
        self.noise_threshold = noise_threshold
        self.contrast_protection = contrast_protection
        self.max_curve_points = max_curve_points

    def compute_brightness_diff(self, image: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
        """
        Compute brightness differences between adjacent pixels.
        Returns matrices of differences along the x and y axes.

        :param image: Input image as a numpy array (grayscale).
        :return: Tuple of differences along x and y axes.
        """
        diff_x = np.diff(image, axis=1)  # differences between columns
        diff_y = np.diff(image, axis=0)  # differences between rows

        # Pad the differences to match the original image size
        diff_x = np.pad(diff_x, ((0, 0), (0, 1)), mode='constant')
        diff_y = np.pad(diff_y, ((0, 1), (0, 0)), mode='constant')

        return diff_x, diff_y

    def stretch(self, image: np.ndarray, roi: Optional[Tuple[int, int, int, int]] = None) -> np.ndarray:
        """
        Apply the AdaptiveStretch transformation to the image.

        :param image: Input image as a numpy array (grayscale or color).
        :param roi: Tuple (x, y, width, height) defining the region of interest.
        :return: Stretched image.
        """
        if len(image.shape) == 3:
            channels = cv2.split(image)
        else:
            channels = [image]

        stretched_channels = []

        for channel in channels:
            # Normalize the channel to the range [0, 1]
            channel = channel.astype(np.float32) / 255.0

            if roi is not None:
                x, y, w, h = roi
                channel_roi = channel[y:y+h, x:x+w]
            else:
                channel_roi = channel

            diff_x, diff_y = self.compute_brightness_diff(channel_roi)

            positive_forces = np.maximum(diff_x, 0) + np.maximum(diff_y, 0)
            negative_forces = np.minimum(diff_x, 0) + np.minimum(diff_y, 0)

            positive_forces[positive_forces < self.noise_threshold] = 0
            negative_forces[negative_forces > -self.noise_threshold] = 0

            transformation_curve = positive_forces + negative_forces

            if self.contrast_protection is not None:
                transformation_curve = np.clip(
                    transformation_curve, -self.contrast_protection, self.contrast_protection)

            resampled_curve = cv2.resize(
                transformation_curve, (self.max_curve_points, 1), interpolation=cv2.INTER_LINEAR)

            interpolated_curve = cv2.resize(
                resampled_curve, (channel_roi.shape[1], channel_roi.shape[0]), interpolation=cv2.INTER_LINEAR)

            stretched_channel = channel_roi + interpolated_curve

            stretched_channel = np.clip(
                stretched_channel * 255, 0, 255).astype(np.uint8)

            if roi is not None:
                channel[y:y+h, x:x+w] = stretched_channel
                stretched_channel = channel

            stretched_channels.append(stretched_channel)

        if len(stretched_channels) > 1:
            stretched_image = cv2.merge(stretched_channels)
        else:
            stretched_image = stretched_channels[0]

        return stretched_image
