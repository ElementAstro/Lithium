import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import CubicSpline, Akima1DInterpolator, interp1d
from typing import Optional, Tuple, List, Callable, Union
from pathlib import Path
import json
from loguru import logger


class CurvesTransformation:
    def __init__(self, interpolation: str = 'akima') -> None:
        self.points: List[Tuple[float, float]] = []
        self.interpolation: str = interpolation
        self.curve: Optional[Callable[[float], float]] = None
        self.stored_curve: Optional[List[Tuple[float, float]]] = None

        # Configure Loguru logger
        logger.add("curve_transformation.log", rotation="10 MB", retention="10 days",
                   level="DEBUG", format="{time:YYYY-MM-DD HH:mm:ss} | {level} | {message}")
        logger.debug(
            f"Initialized CurvesTransformation with interpolation='{self.interpolation}'")

    def add_point(self, x: float, y: float) -> None:
        logger.debug(f"Adding point ({x}, {y})")
        self.points.append((x, y))
        self.points.sort(key=lambda point: point[0])  # Sort points by x value
        self._update_curve()

    def remove_point(self, index: int) -> None:
        if 0 <= index < len(self.points):
            removed = self.points.pop(index)
            logger.debug(f"Removed point at index {index}: {removed}")
            self._update_curve()
        else:
            logger.error(f"Index out of range: {index}")
            raise IndexError("Point index out of range.")

    def _update_curve(self) -> None:
        if len(self.points) < 2:
            self.curve = None
            logger.warning("Not enough points to define a curve.")
            return

        x, y = zip(*self.points)

        logger.debug(
            f"Updating curve with interpolation method: {self.interpolation}")

        try:
            if self.interpolation == 'cubic':
                self.curve = CubicSpline(x, y)
                logger.debug("CubicSpline interpolation applied.")
            elif self.interpolation == 'akima':
                self.curve = Akima1DInterpolator(x, y)
                logger.debug("Akima1DInterpolator interpolation applied.")
            elif self.interpolation == 'linear':
                self.curve = interp1d(
                    x, y, kind='linear', fill_value="extrapolate")
                logger.debug("Linear interpolation applied.")
            else:
                logger.error(
                    f"Unsupported interpolation method: {self.interpolation}")
                raise ValueError("Unsupported interpolation method.")
        except Exception as e:
            logger.error(f"Failed to update curve: {e}")
            self.curve = None

    def transform(self, image: np.ndarray, channel: Optional[int] = None) -> np.ndarray:
        if self.curve is None:
            logger.error("No valid curve defined for transformation.")
            raise ValueError("No valid curve defined.")

        logger.info(
            f"Applying curve transformation{' on channel ' + str(channel) if channel is not None else ''}")

        try:
            transformed_image = image.astype(
                np.float32) / 255.0  # Normalize to [0, 1]

            if len(image.shape) == 2:  # Grayscale image
                logger.debug("Transforming grayscale image.")
                transformed_image = self.curve(transformed_image)
            elif len(image.shape) == 3:  # RGB image
                if channel is None:
                    logger.error("Channel must be specified for color images.")
                    raise ValueError(
                        "Channel must be specified for color images.")
                logger.debug(f"Transforming channel {channel} of RGB image.")
                transformed_image[:, :, channel] = self.curve(
                    transformed_image[:, :, channel])
            else:
                logger.error("Unsupported image format.")
                raise ValueError("Unsupported image format.")

            transformed_image = np.clip(transformed_image, 0, 1)
            transformed_image = (transformed_image * 255).astype(np.uint8)
            logger.info("Curve transformation applied successfully.")
            return transformed_image
        except Exception as e:
            logger.error(f"Error during transformation: {e}")
            raise

    def plot_curve(self, title: str = "Curves Transformation") -> None:
        if self.curve is None:
            logger.warning("No curve to plot.")
            print("No curve to plot.")
            return

        logger.debug(f"Plotting curve with title: {title}")
        x_vals = np.linspace(0, 1, 500)
        y_vals = self.curve(x_vals)

        plt.figure(figsize=(8, 6))
        plt.plot(x_vals, y_vals,
                 label=f'Interpolation: {self.interpolation}', color='blue')
        plt.scatter(*zip(*self.points), color='red', label='Control Points')
        plt.title(title)
        plt.xlabel('Input Intensity')
        plt.ylabel('Output Intensity')
        plt.grid(True)
        plt.legend()
        plt.tight_layout()
        plt.show()
        logger.debug("Curve plotted successfully.")

    def store_curve(self) -> None:
        self.stored_curve = self.points.copy()
        logger.info("Curve stored successfully.")

    def restore_curve(self) -> None:
        if self.stored_curve:
            self.points = self.stored_curve.copy()
            self._update_curve()
            logger.info("Curve restored successfully.")
        else:
            logger.warning("No stored curve to restore.")
            print("No stored curve to restore.")

    def invert_curve(self) -> None:
        if self.curve is None:
            logger.warning("No curve to invert.")
            print("No curve to invert.")
            return

        logger.info("Inverting curve.")
        self.points = [(x, 1 - y) for x, y in self.points]
        self._update_curve()
        logger.debug("Curve inverted successfully.")

    def reset_curve(self) -> None:
        self.points = [(0.0, 0.0), (1.0, 1.0)]
        self._update_curve()
        logger.info("Curve reset to default.")

    def pixel_readout(self, x: float) -> Optional[float]:
        if self.curve is None:
            logger.warning("No curve defined for pixel readout.")
            print("No curve defined.")
            return None
        try:
            value = self.curve(x)
            logger.debug(f"Pixel readout at x={x}: {value}")
            return float(value)
        except Exception as e:
            logger.error(f"Error in pixel readout: {e}")
            return None

    def save_curve(self, filepath: Union[str, Path]) -> None:
        logger.info(f"Saving curve to {filepath}")
        try:
            curve_data = {
                'interpolation': self.interpolation,
                'points': self.points
            }
            with open(filepath, 'w') as file:
                json.dump(curve_data, file, indent=4)
            logger.debug("Curve saved successfully.")
        except Exception as e:
            logger.error(f"Failed to save curve: {e}")
            raise

    def load_curve(self, filepath: Union[str, Path]) -> None:
        logger.info(f"Loading curve from {filepath}")
        try:
            with open(filepath, 'r') as file:
                curve_data = json.load(file)
            self.interpolation = curve_data.get('interpolation', 'akima')
            self.points = curve_data.get('points', [])
            self._update_curve()
            logger.debug("Curve loaded successfully.")
        except Exception as e:
            logger.error(f"Failed to load curve: {e}")
            raise

    def export_curve_points(self) -> List[Tuple[float, float]]:
        logger.debug("Exporting curve points.")
        return self.points.copy()

    def import_curve_points(self, points: List[Tuple[float, float]]) -> None:
        logger.debug(f"Importing curve points: {points}")
        self.points = sorted(points, key=lambda point: point[0])
        self._update_curve()

    def get_interpolation_methods(self) -> List[str]:
        methods = ['cubic', 'akima', 'linear']
        logger.debug(f"Available interpolation methods: {methods}")
        return methods


# Example Usage
if __name__ == "__main__":
    import cv2
    import sys

    # Initialize CurvesTransformation object
    curve_transform = CurvesTransformation(interpolation='akima')

    # Add points to the curve
    curve_transform.add_point(0.0, 0.0)
    curve_transform.add_point(0.3, 0.5)
    curve_transform.add_point(0.7, 0.8)
    curve_transform.add_point(1.0, 1.0)

    # Plot the curve
    curve_transform.plot_curve()

    # Store the curve
    curve_transform.store_curve()

    # Invert the curve
    curve_transform.invert_curve()
    curve_transform.plot_curve()

    # Restore the original curve
    curve_transform.restore_curve()
    curve_transform.plot_curve()

    # Reset the curve to default
    curve_transform.reset_curve()
    curve_transform.plot_curve()

    # Save the curve
    curve_transform.save_curve("default_curve.json")

    # Load the curve
    curve_transform.load_curve("default_curve.json")
    curve_transform.plot_curve()

    # Generate a test image
    test_image = np.linspace(0, 1, 256).reshape(16, 16).astype(np.float32)

    # Apply the transformation
    transformed_image = curve_transform.transform(test_image)

    # Plot original and transformed images
    plt.figure(figsize=(12, 6))
    plt.subplot(1, 2, 1)
    plt.title("Original Image")
    plt.imshow(test_image, cmap='gray', vmin=0, vmax=1)

    plt.subplot(1, 2, 2)
    plt.title("Transformed Image")
    plt.imshow(transformed_image, cmap='gray', vmin=0, vmax=1)
    plt.tight_layout()
    plt.show()

    # Pixel readout
    readout_value = curve_transform.pixel_readout(0.5)
    print(f"Pixel readout at x=0.5: {readout_value}")
