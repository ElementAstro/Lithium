import concurrent.futures
from dataclasses import dataclass, field
from enum import Enum
from pathlib import Path
from typing import Optional, Tuple, Literal, List
import cv2
import numpy as np
from loguru import logger
import argparse
import sys


# Configure Loguru logger with file rotation and different log levels
logger.remove()  # Remove the default logger
logger.add(sys.stderr, level="INFO",
           format="{time:YYYY-MM-DD HH:mm:ss} | {level} | {message}")
logger.add("adaptive_stretch.log", rotation="10 MB", retention="10 days",
           level="DEBUG", format="{time:YYYY-MM-DD HH:mm:ss} | {level} | {message}")


class ImageFormat(Enum):
    PNG = "png"
    JPEG = "jpg"
    TIFF = "tiff"
    BMP = "bmp"

    @staticmethod
    def list():
        return [fmt.value for fmt in ImageFormat]


@dataclass
class AdaptiveStretch:
    noise_threshold: float = 1e-4
    contrast_protection: Optional[float] = None
    max_curve_points: int = 106
    roi: Optional[Tuple[int, int, int, int]] = None  # (x, y, width, height)
    save_intermediate: bool = False  # 是否保存中间结果
    intermediate_dir: Optional[Path] = None  # 保存中间结果的目录

    def __post_init__(self):
        """
        Initialize the logger and log the initialization parameters.
        """
        if self.save_intermediate:
            if self.intermediate_dir is None:
                self.intermediate_dir = Path("intermediate_results")
            self.intermediate_dir.mkdir(parents=True, exist_ok=True)
            logger.debug(
                f"Intermediate results will be saved to {self.intermediate_dir}")
        logger.info("AdaptiveStretch initialized with noise_threshold={}, contrast_protection={}, max_curve_points={}, roi={}, save_intermediate={}, intermediate_dir={}",
                    self.noise_threshold, self.contrast_protection, self.max_curve_points,
                    self.roi, self.save_intermediate, self.intermediate_dir)

    def compute_brightness_diff(self, image: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
        """
        Compute the brightness differences in the x and y directions.

        :param image: Input image as a numpy array.
        :return: Tuple containing the brightness differences in the x and y directions.
        """
        logger.debug("Computing brightness differences")
        diff_x = np.diff(image, axis=1)
        diff_y = np.diff(image, axis=0)
        diff_x = np.pad(diff_x, ((0, 0), (0, 1)), mode='constant')
        diff_y = np.pad(diff_y, ((0, 1), (0, 0)), mode='constant')
        return diff_x, diff_y

    def stretch(self, image: np.ndarray) -> np.ndarray:
        """
        Apply adaptive stretch transformation to the image.

        :param image: Input image as a numpy array (grayscale or color).
        :return: Stretched image as a numpy array.
        """
        logger.info("Starting stretch operation")
        if len(image.shape) == 3:
            channels = cv2.split(image)
        else:
            channels = [image]

        stretched_channels = []

        for idx, channel in enumerate(channels):
            logger.debug(f"Processing channel {idx}")
            channel = channel.astype(np.float32) / 255.0

            if self.roi:
                x, y, w, h = self.roi
                channel_roi = channel[y:y+h, x:x+w]
                logger.debug(f"Applying ROI: x={x}, y={y}, w={w}, h={h}")
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
                logger.debug(
                    f"Applied contrast protection: {self.contrast_protection}")

            resampled_curve = cv2.resize(
                transformation_curve, (self.max_curve_points, 1), interpolation=cv2.INTER_LINEAR)
            interpolated_curve = cv2.resize(
                resampled_curve, (channel_roi.shape[1], channel_roi.shape[0]), interpolation=cv2.INTER_LINEAR)

            stretched_channel = channel_roi + interpolated_curve
            stretched_channel = np.clip(
                stretched_channel * 255, 0, 255).astype(np.uint8)

            if self.roi:
                channel[y:y+h, x:x+w] = stretched_channel
                stretched_channel = channel
                logger.debug(
                    f"Replaced ROI with stretched channel for channel {idx}")

            stretched_channels.append(stretched_channel)

            if self.save_intermediate and self.intermediate_dir:
                intermediate_path = self.intermediate_dir / \
                    f"channel_{idx}_stretched.png"
                cv2.imwrite(str(intermediate_path), stretched_channel)
                logger.debug(
                    f"Saved intermediate stretched channel to {intermediate_path}")

        if len(stretched_channels) > 1:
            stretched_image = cv2.merge(stretched_channels)
        else:
            stretched_image = stretched_channels[0]

        logger.info("Stretch operation completed")
        return stretched_image

    def stretch_channel(self, o: np.ndarray, s: np.ndarray, c: int, bg: float, sigma: float, median: float, mad: float) -> None:
        """
        Stretch a single channel of the image.

        :param o: Original image.
        :param s: Stretched image.
        :param c: Channel index.
        :param bg: Background level.
        :param sigma: Sigma value for clipping.
        :param median: Median value of the channel.
        :param mad: Median absolute deviation of the channel.
        """
        o_channel = o[:, :, c]
        s_channel = s[:, :, c]

        shadow_clipping = np.clip(median - sigma * mad, 0, 1.0)
        highlight_clipping = 1.0

        midtone = self.MTF((median - shadow_clipping) /
                           (highlight_clipping - shadow_clipping), bg)

        o_channel[o_channel <= shadow_clipping] = 0.0
        o_channel[o_channel >= highlight_clipping] = 1.0

        s_channel[s_channel <= shadow_clipping] = 0.0
        s_channel[s_channel >= highlight_clipping] = 1.0

        indx_inside_o = np.logical_and(
            o_channel > shadow_clipping, o_channel < highlight_clipping)
        indx_inside_s = np.logical_and(
            s_channel > shadow_clipping, s_channel < highlight_clipping)

        o_channel[indx_inside_o] = (
            o_channel[indx_inside_o] - shadow_clipping) / (highlight_clipping - shadow_clipping)
        s_channel[indx_inside_s] = (
            s_channel[indx_inside_s] - shadow_clipping) / (highlight_clipping - shadow_clipping)

        o_channel = self.MTF(o_channel, midtone)
        s_channel = self.MTF(s_channel, midtone)
        o[:, :, c] = o_channel[:, :]
        s[:, :, c] = s_channel[:, :]

    def stretch(self, o: np.ndarray, s: np.ndarray, bg: float, sigma: float, median: List[float], mad: List[float]) -> Tuple[np.ndarray, np.ndarray]:
        """
        Stretch the image using the specified parameters.

        :param o: Original image.
        :param s: Stretched image.
        :param bg: Background level.
        :param sigma: Sigma value for clipping.
        :param median: List of median values for each channel.
        :param mad: List of median absolute deviations for each channel.
        :return: Tuple of stretched images.
        """
        o_copy = np.copy(o)
        s_copy = np.copy(s)

        for c in range(o_copy.shape[-1]):
            self.stretch_channel(o_copy, s_copy, c, bg,
                                 sigma, median[c], mad[c])

        return o_copy, s_copy

    def MTF(self, data: np.ndarray, midtone: float) -> np.ndarray:
        """
        Apply midtone transfer function (MTF) to the data.

        :param data: Input data.
        :param midtone: Midtone value.
        :return: Transformed data.
        """
        if isinstance(data, np.ndarray):
            data[:] = (midtone - 1) * data[:] / \
                ((2 * midtone - 1) * data[:] - midtone)
        else:
            data = (midtone - 1) * data / ((2 * midtone - 1) * data - midtone)

        return data

    def MTF_inverse(self, data: np.ndarray, midtone: float) -> np.ndarray:
        """
        Apply inverse midtone transfer function (MTF) to the data.

        :param data: Input data.
        :param midtone: Midtone value.
        :return: Transformed data.
        """
        if isinstance(data, np.ndarray):
            data[:] = midtone * data[:] / \
                ((2 * midtone - 1) * data[:] - (midtone - 1))
        else:
            data = midtone * data / ((2 * midtone - 1) * data - (midtone - 1))

        return data


def parse_arguments() -> argparse.Namespace:
    """
    Parse command-line arguments for the stretch script.

    :return: Parsed arguments.
    """
    parser = argparse.ArgumentParser(
        description="Adaptive Image Stretching Tool")
    parser.add_argument('--input', type=Path, required=True,
                        help='Path to the input image.')
    parser.add_argument('--output', type=Path, required=True,
                        help='Path to save the stretched image.')
    parser.add_argument('--noise_threshold', type=float,
                        default=1e-4, help='Noise threshold for stretching.')
    parser.add_argument('--contrast_protection', type=float,
                        default=None, help='Contrast protection limit.')
    parser.add_argument('--max_curve_points', type=int,
                        default=106, help='Maximum number of curve points.')
    parser.add_argument('--roi', type=int, nargs=4, metavar=('X', 'Y', 'W', 'H'), default=None,
                        help='Region of interest as four integers: x y width height.')
    parser.add_argument('--save_intermediate', action='store_true',
                        help='Save intermediate stretched channels.')
    parser.add_argument('--intermediate_dir', type=Path, default=None,
                        help='Directory to save intermediate results.')

    return parser.parse_args()


def main():
    """
    Main function to parse arguments and execute the stretch operation.
    """
    args = parse_arguments()

    if not args.input.exists():
        logger.error(f"Input file does not exist: {args.input}")
        sys.exit(1)

    image = cv2.imread(str(args.input), cv2.IMREAD_UNCHANGED)
    if image is None:
        logger.error(f"Failed to load image: {args.input}")
        sys.exit(1)

    stretcher = AdaptiveStretch(
        noise_threshold=args.noise_threshold,
        contrast_protection=args.contrast_protection,
        max_curve_points=args.max_curve_points,
        roi=tuple(args.roi) if args.roi else None,
        save_intermediate=args.save_intermediate,
        intermediate_dir=args.intermediate_dir
    )

    stretched_image = stretcher.stretch(image)

    success = cv2.imwrite(str(args.output), stretched_image)
    if success:
        logger.info(f"Stretched image saved to: {args.output}")
    else:
        logger.error(f"Failed to save stretched image to: {args.output}")


if __name__ == "__main__":
    main()
