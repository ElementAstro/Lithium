import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import CubicSpline, Akima1DInterpolator


class CurvesTransformation:
    def __init__(self, interpolation='akima'):
        self.points = []
        self.interpolation = interpolation
        self.curve = None
        self.stored_curve = None

    def add_point(self, x, y):
        self.points.append((x, y))
        self.points.sort()  # Sort points by x value
        self._update_curve()

    def remove_point(self, index):
        if 0 <= index < len(self.points):
            self.points.pop(index)
            self._update_curve()

    def _update_curve(self):
        if len(self.points) < 2:
            self.curve = None
            return

        x, y = zip(*self.points)

        if self.interpolation == 'cubic':
            self.curve = CubicSpline(x, y)
        elif self.interpolation == 'akima':
            self.curve = Akima1DInterpolator(x, y)
        elif self.interpolation == 'linear':
            self.curve = lambda x_new: np.interp(x_new, x, y)
        else:
            raise ValueError("Unsupported interpolation method")

    def transform(self, image, channel=None):
        if self.curve is None:
            raise ValueError("No valid curve defined")

        if len(image.shape) == 2:  # Grayscale image
            transformed_image = self.curve(image)
        elif len(image.shape) == 3:  # RGB image
            if channel is None:
                raise ValueError("Channel must be specified for color images")
            transformed_image = image.copy()
            transformed_image[:, :, channel] = self.curve(image[:, :, channel])
        else:
            raise ValueError("Unsupported image format")

        transformed_image = np.clip(transformed_image, 0, 1)
        return transformed_image

    def plot_curve(self):
        if self.curve is None:
            print("No curve to plot")
            return

        x_vals = np.linspace(0, 1, 100)
        y_vals = self.curve(x_vals)

        plt.plot(x_vals, y_vals, label=f'Interpolation: {self.interpolation}')
        plt.scatter(*zip(*self.points), color='red')
        plt.title('Curves Transformation')
        plt.xlabel('Input')
        plt.ylabel('Output')
        plt.grid(True)
        plt.legend()
        plt.show()

    def store_curve(self):
        self.stored_curve = self.points.copy()
        print("Curve stored.")

    def restore_curve(self):
        if self.stored_curve:
            self.points = self.stored_curve.copy()
            self._update_curve()
            print("Curve restored.")
        else:
            print("No stored curve to restore.")

    def invert_curve(self):
        if self.curve is None:
            print("No curve to invert")
            return

        self.points = [(x, 1 - y) for x, y in self.points]
        self._update_curve()
        print("Curve inverted.")

    def reset_curve(self):
        self.points = [(0, 0), (1, 1)]
        self._update_curve()
        print("Curve reset to default.")

    def pixel_readout(self, x):
        if self.curve is None:
            print("No curve defined")
            return None
        return self.curve(x)


# Example Usage
if __name__ == "__main__":
    # Create a CurvesTransformation object
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

    # Generate a test image
    test_image = np.linspace(0, 1, 256).reshape(16, 16)

    # Apply the transformation
    transformed_image = curve_transform.transform(test_image)

    # Plot original and transformed images
    plt.figure(figsize=(8, 4))
    plt.subplot(1, 2, 1)
    plt.title("Original Image")
    plt.imshow(test_image, cmap='gray')

    plt.subplot(1, 2, 2)
    plt.title("Transformed Image")
    plt.imshow(transformed_image, cmap='gray')
    plt.show()

    # Pixel readout
    readout_value = curve_transform.pixel_readout(0.5)
    print(f"Pixel readout at x=0.5: {readout_value}")
