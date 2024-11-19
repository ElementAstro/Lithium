from pathlib import Path
from typing import Optional, Tuple
from dataclasses import dataclass, field
from loguru import logger
from enum import Enum
import rawpy
import cv2
import numpy as np
import argparse
import sys
import concurrent.futures


# Configure Loguru logger with file rotation and different log levels
logger.remove()  # Remove the default logger
logger.add(sys.stderr, level="INFO",
           format="{time:YYYY-MM-DD HH:mm:ss} | {level} | {message}")
logger.add("raw_processor.log", rotation="10 MB", retention="10 days",
           level="DEBUG", format="{time:YYYY-MM-DD HH:mm:ss} | {level} | {message}")


class ImageFormat(Enum):
    PNG = "png"
    JPEG = "jpg"
    TIFF = "tiff"
    BMP = "bmp"

    @staticmethod
    def list():
        return list(map(lambda c: c.value, ImageFormat))


@dataclass
class RawImageProcessor:
    raw_path: Path
    raw: rawpy.RawPy = field(init=False)
    rgb_image: np.ndarray = field(init=False)
    bgr_image: np.ndarray = field(init=False)

    def __post_init__(self):
        """
        Initialize and read the RAW image.
        """
        logger.debug(
            f"Initializing RawImageProcessor with RAW file: {self.raw_path}")
        self._load_raw_image()
        self._postprocess_raw()

    def _load_raw_image(self) -> None:
        """
        Load the RAW image from the given path.
        """
        try:
            self.raw = rawpy.imread(str(self.raw_path))
            logger.info(f"Successfully read RAW image: {self.raw_path}")
        except Exception as e:
            logger.error(f"Failed to read RAW image at {self.raw_path}: {e}")
            raise IOError(f"Cannot read RAW image: {self.raw_path}") from e

    def _postprocess_raw(self) -> None:
        """
        Post-process the RAW image to obtain an RGB image.
        """
        try:
            self.rgb_image = self.raw.postprocess(
                gamma=(1.0, 1.0),
                no_auto_bright=True,
                use_camera_wb=True,
                output_bps=16  # Use higher bit depth for better quality
            )
            logger.debug("Successfully post-processed RAW image to RGB.")
        except Exception as e:
            logger.error(f"Failed to post-process RAW image: {e}")
            raise ValueError("RAW image post-processing failed") from e

        try:
            self.bgr_image = cv2.cvtColor(self.rgb_image, cv2.COLOR_RGB2BGR)
            logger.debug("Converted RGB image to BGR format for OpenCV.")
        except Exception as e:
            logger.error(f"Failed to convert RGB to BGR: {e}")
            raise RuntimeError("Image format conversion failed") from e

    def adjust_contrast(self, alpha: float = 1.0) -> None:
        """
        Adjust the contrast of the image.

        :param alpha: Contrast control (1.0-3.0). Default is 1.0.
        """
        logger.debug(f"Adjusting contrast with alpha={alpha}")
        try:
            self.bgr_image = cv2.convertScaleAbs(self.bgr_image, alpha=alpha)
            logger.info(f"Contrast adjusted successfully with alpha={alpha}")
        except Exception as e:
            logger.error(f"Failed to adjust contrast: {e}")
            raise RuntimeError("Contrast adjustment failed") from e

    def adjust_brightness(self, beta: int = 0) -> None:
        """
        Adjust the brightness of the image.

        :param beta: Brightness control (0-100). Default is 0.
        """
        logger.debug(f"Adjusting brightness with beta={beta}")
        try:
            self.bgr_image = cv2.convertScaleAbs(self.bgr_image, beta=beta)
            logger.info(f"Brightness adjusted successfully with beta={beta}")
        except Exception as e:
            logger.error(f"Failed to adjust brightness: {e}")
            raise RuntimeError("Brightness adjustment failed") from e

    def apply_sharpening(self) -> None:
        """
        Apply sharpening to the image.
        """
        logger.debug("Applying sharpening filter.")
        try:
            kernel = np.array([[0, -1, 0],
                               [-1, 5, -1],
                               [0, -1, 0]])
            self.bgr_image = cv2.filter2D(self.bgr_image, -1, kernel)
            logger.info("Sharpening applied successfully.")
        except Exception as e:
            logger.error(f"Failed to apply sharpening: {e}")
            raise RuntimeError("Sharpening application failed") from e

    def apply_gamma_correction(self, gamma: float = 1.0) -> None:
        """
        Apply gamma correction to the image.

        :param gamma: Gamma value (0.1-3.0). Default is 1.0.
        """
        logger.debug(f"Applying gamma correction with gamma={gamma}")
        try:
            inv_gamma = 1.0 / gamma
            table = np.array([(i / 255.0) ** inv_gamma *
                             255 for i in np.arange(256)]).astype("uint8")
            self.bgr_image = cv2.LUT(self.bgr_image, table)
            logger.info(
                f"Gamma correction applied successfully with gamma={gamma}")
        except Exception as e:
            logger.error(f"Failed to apply gamma correction: {e}")
            raise RuntimeError("Gamma correction application failed") from e

    def rotate_image(self, angle: float, center: Optional[Tuple[int, int]] = None, scale: float = 1.0) -> None:
        """
        Rotate the image.

        :param angle: Angle to rotate the image.
        :param center: Center of rotation. If None, the center of the image is used.
        :param scale: Scale factor. Default is 1.0.
        """
        logger.debug(f"Rotating image by {angle} degrees with scale={scale}")
        try:
            (h, w) = self.bgr_image.shape[:2]
            if center is None:
                center = (w // 2, h // 2)
            M = cv2.getRotationMatrix2D(center, angle, scale)
            self.bgr_image = cv2.warpAffine(self.bgr_image, M, (w, h))
            logger.info(f"Image rotated successfully by {angle} degrees.")
        except Exception as e:
            logger.error(f"Failed to rotate image: {e}")
            raise RuntimeError("Image rotation failed") from e

    def resize_image(self, width: Optional[int] = None, height: Optional[int] = None, inter: int = cv2.INTER_LINEAR) -> None:
        """
        Resize the image.

        :param width: New width of the image. If None, the width is scaled proportionally.
        :param height: New height of the image. If None, the height is scaled proportionally.
        :param inter: Interpolation method. Default is cv2.INTER_LINEAR.
        """
        logger.debug(f"Resizing image with width={width}, height={height}")
        try:
            (h, w) = self.bgr_image.shape[:2]
            if width is None and height is None:
                logger.warning(
                    "No resizing performed as both width and height are None.")
                return
            if width is None:
                ratio = height / float(h)
                dim = (int(w * ratio), height)
            elif height is None:
                ratio = width / float(w)
                dim = (width, int(h * ratio))
            else:
                dim = (width, height)
            self.bgr_image = cv2.resize(
                self.bgr_image, dim, interpolation=inter)
            logger.info(f"Image resized successfully to dimensions: {dim}")
        except Exception as e:
            logger.error(f"Failed to resize image: {e}")
            raise RuntimeError("Image resize failed") from e

    def adjust_color_balance(self, red: float = 1.0, green: float = 1.0, blue: float = 1.0) -> None:
        """
        Adjust the color balance of the image.

        :param red: Red channel multiplier. Default is 1.0.
        :param green: Green channel multiplier. Default is 1.0.
        :param blue: Blue channel multiplier. Default is 1.0.
        """
        logger.debug(
            f"Adjusting color balance with R={red}, G={green}, B={blue}")
        try:
            b, g, r = cv2.split(self.bgr_image)
            r = cv2.convertScaleAbs(r, alpha=red)
            g = cv2.convertScaleAbs(g, alpha=green)
            b = cv2.convertScaleAbs(b, alpha=blue)
            self.bgr_image = cv2.merge([b, g, r])
            logger.info(
                f"Color balance adjusted successfully with R={red}, G={green}, B={blue}")
        except Exception as e:
            logger.error(f"Failed to adjust color balance: {e}")
            raise RuntimeError("Color balance adjustment failed") from e

    def apply_blur(self, ksize: Tuple[int, int] = (5, 5), method: str = "gaussian") -> None:
        """
        Apply blur to the image.

        :param ksize: Kernel size for the blur. Default is (5, 5).
        :param method: Blurring method ('gaussian', 'median', 'bilateral'). Default is 'gaussian'.
        """
        logger.debug(f"Applying {method} blur with kernel size={ksize}")
        try:
            if method.lower() == "gaussian":
                self.bgr_image = cv2.GaussianBlur(self.bgr_image, ksize, 0)
            elif method.lower() == "median":
                self.bgr_image = cv2.medianBlur(self.bgr_image, ksize[0])
            elif method.lower() == "bilateral":
                self.bgr_image = cv2.bilateralFilter(
                    self.bgr_image, d=9, sigmaColor=75, sigmaSpace=75)
            else:
                logger.error(f"Unsupported blur method: {method}")
                raise ValueError(f"Unsupported blur method: {method}")
            logger.info(
                f"{method.capitalize()} blur applied successfully with kernel size={ksize}")
        except Exception as e:
            logger.error(f"Failed to apply {method} blur: {e}")
            raise RuntimeError(
                f"{method.capitalize()} blur application failed") from e

    def histogram_equalization(self) -> None:
        """
        Apply histogram equalization to the image.
        Enhances the contrast of the image using histogram equalization.
        """
        logger.debug("Applying histogram equalization.")
        try:
            if len(self.bgr_image.shape) == 2:
                self.bgr_image = cv2.equalizeHist(self.bgr_image)
                logger.debug(
                    "Applied histogram equalization on grayscale image.")
            else:
                ycrcb = cv2.cvtColor(self.bgr_image, cv2.COLOR_BGR2YCrCb)
                ycrcb[:, :, 0] = cv2.equalizeHist(ycrcb[:, :, 0])
                self.bgr_image = cv2.cvtColor(ycrcb, cv2.COLOR_YCrCb2BGR)
                logger.debug(
                    "Applied histogram equalization on Y channel of color image.")
            logger.info("Histogram equalization applied successfully.")
        except Exception as e:
            logger.error(f"Failed to apply histogram equalization: {e}")
            raise RuntimeError("Histogram equalization failed") from e

    def convert_to_grayscale(self) -> None:
        """
        Convert the image to grayscale.
        """
        logger.debug("Converting image to grayscale.")
        try:
            self.bgr_image = cv2.cvtColor(self.bgr_image, cv2.COLOR_BGR2GRAY)
            logger.info("Image converted to grayscale successfully.")
        except Exception as e:
            logger.error(f"Failed to convert image to grayscale: {e}")
            raise RuntimeError("Grayscale conversion failed") from e

    def get_bgr_image(self) -> np.ndarray:
        """
        Return the processed BGR image.

        :return: Processed BGR image as a NumPy array.
        """
        logger.debug("Retrieving processed BGR image.")
        return self.bgr_image

    def to_rgb_image(self) -> np.ndarray:
        """
        Return the processed RGB image.

        :return: Processed RGB image as a NumPy array.
        """
        logger.debug("Converting BGR image to RGB format.")
        try:
            rgb_image = cv2.cvtColor(self.bgr_image, cv2.COLOR_BGR2RGB)
            logger.info("Successfully converted BGR image to RGB format.")
            return rgb_image
        except Exception as e:
            logger.error(f"Failed to convert BGR to RGB: {e}")
            raise RuntimeError("RGB conversion failed") from e

    def save_image(self, output_path: Path, file_format: ImageFormat = ImageFormat.PNG, jpeg_quality: int = 90) -> None:
        """
        Save the image to the specified path.

        :param output_path: Path to save the image.
        :param file_format: Format to save the image. Default is PNG.
        :param jpeg_quality: Quality for JPEG format (0-100). Default is 90.
        """
        logger.debug(
            f"Saving image to {output_path} with format={file_format.value} and JPEG quality={jpeg_quality}")
        try:
            if file_format in [ImageFormat.JPEG, ImageFormat.PNG, ImageFormat.TIFF, ImageFormat.BMP]:
                if file_format == ImageFormat.JPEG:
                    cv2.imwrite(str(output_path), self.bgr_image, [
                                cv2.IMWRITE_JPEG_QUALITY, jpeg_quality])
                    logger.info(
                        f"Image saved successfully as JPEG: {output_path} with quality={jpeg_quality}")
                else:
                    cv2.imwrite(str(output_path), self.bgr_image)
                    logger.info(
                        f"Image saved successfully as {file_format.value.upper()}: {output_path}")
            else:
                logger.error(f"Unsupported file format: {file_format}")
                raise ValueError(f"Unsupported file format: {file_format}")
        except Exception as e:
            logger.error(f"Failed to save image: {e}")
            raise IOError("Image save failed") from e

    def show_image(self, window_name: str = "Image", delay: int = 0) -> None:
        """
        Display the processed image.

        :param window_name: Name of the window in which to display the image. Default is "Image".
        :param delay: Delay in milliseconds. 0 means wait indefinitely. Default is 0.
        """
        logger.debug(
            f"Displaying image in window: {window_name} with delay={delay}")
        try:
            cv2.imshow(window_name, self.bgr_image)
            logger.info("Image display window opened.")
            cv2.waitKey(delay)
            cv2.destroyAllWindows()
            logger.debug("Image display window closed.")
        except Exception as e:
            logger.error(f"Failed to display image: {e}")
            raise RuntimeError("Image display failed") from e

    def reset(self) -> None:
        """
        Reset the image to its original state after post-processing.
        """
        logger.debug("Resetting image to original post-processed state.")
        try:
            self.bgr_image = cv2.cvtColor(self.rgb_image, cv2.COLOR_RGB2BGR)
            logger.info("Image reset successfully.")
        except Exception as e:
            logger.error(f"Failed to reset image: {e}")
            raise RuntimeError("Image reset failed") from e

    def to_raw_image(self) -> rawpy.RawPy:
        """
        Return the original RAW image.

        :return: Original RAW image as a rawpy.RawPy object.
        """
        logger.debug("Retrieving original RAW image.")
        return self.raw


def parse_cli_arguments() -> argparse.Namespace:
    """
    Parse command-line arguments for the RAW image processor.

    :return: Parsed arguments.
    """
    parser = argparse.ArgumentParser(description="RAW Image Processor")
    subparsers = parser.add_subparsers(
        dest='command', required=True, help='Available commands')

    # Subparser for processing a single image
    process_parser = subparsers.add_parser(
        'process', help='Process a single RAW image.')
    process_parser.add_argument(
        '--input', type=Path, required=True, help='Path to the input RAW image.')
    process_parser.add_argument(
        '--output', type=Path, required=True, help='Path to save the processed image.')
    process_parser.add_argument('--format', type=ImageFormat, default=ImageFormat.PNG,
                                choices=ImageFormat, help='Output image format.')
    process_parser.add_argument(
        '--jpeg_quality', type=int, default=90, help='JPEG quality (if applicable).')
    process_parser.add_argument('--actions', nargs='+', default=[], choices=[
        'adjust_contrast', 'adjust_brightness', 'apply_sharpening', 'apply_gamma_correction',
        'rotate_image', 'resize_image', 'adjust_color_balance', 'apply_blur',
        'histogram_equalization', 'convert_to_grayscale'
    ], help='Image processing actions to perform.')

    # Subparser for batch processing
    batch_parser = subparsers.add_parser(
        'batch', help='Batch process RAW images in a directory.')
    batch_parser.add_argument(
        '--input_dir', type=Path, required=True, help='Directory containing RAW images.')
    batch_parser.add_argument(
        '--output_dir', type=Path, required=True, help='Directory to save processed images.')
    batch_parser.add_argument('--format', type=ImageFormat, default=ImageFormat.PNG,
                              choices=ImageFormat, help='Output image format.')
    batch_parser.add_argument(
        '--jpeg_quality', type=int, default=90, help='JPEG quality (if applicable).')
    batch_parser.add_argument('--actions', nargs='+', default=[], choices=[
        'adjust_contrast', 'adjust_brightness', 'apply_sharpening', 'apply_gamma_correction',
        'rotate_image', 'resize_image', 'adjust_color_balance', 'apply_blur',
        'histogram_equalization', 'convert_to_grayscale'
    ], help='Image processing actions to perform.')

    return parser.parse_args()


def process_single_image(args: argparse.Namespace) -> None:
    """
    Process a single RAW image based on the provided arguments.

    :param args: Parsed command-line arguments.
    """
    processor = RawImageProcessor(raw_path=args.input)

    # Execute actions in the order provided
    for action in args.actions:
        if action == 'adjust_contrast':
            processor.adjust_contrast(alpha=1.2)
        elif action == 'adjust_brightness':
            processor.adjust_brightness(beta=30)
        elif action == 'apply_sharpening':
            processor.apply_sharpening()
        elif action == 'apply_gamma_correction':
            processor.apply_gamma_correction(gamma=1.3)
        elif action == 'rotate_image':
            processor.rotate_image(angle=30)
        elif action == 'resize_image':
            processor.resize_image(width=1024)
        elif action == 'adjust_color_balance':
            processor.adjust_color_balance(red=1.1, green=1.0, blue=0.9)
        elif action == 'apply_blur':
            processor.apply_blur(ksize=(5, 5), method="gaussian")
        elif action == 'histogram_equalization':
            processor.histogram_equalization()
        elif action == 'convert_to_grayscale':
            processor.convert_to_grayscale()
        else:
            logger.warning(f"Unknown action: {action}")

    # Save the processed image
    processor.save_image(output_path=args.output,
                         file_format=args.format, jpeg_quality=args.jpeg_quality)
    logger.info(f"Processed image saved to {args.output}")


def batch_process_images(args: argparse.Namespace) -> None:
    """
    Batch process RAW images in the specified directory.

    :param args: Parsed command-line arguments.
    """
    logger.info(
        f"Starting batch processing from {args.input_dir} to {args.output_dir}")
    if not args.input_dir.exists() or not args.input_dir.is_dir():
        logger.error(
            f"Input directory does not exist or is not a directory: {args.input_dir}")
        raise NotADirectoryError(
            f"Input directory does not exist or is not a directory: {args.input_dir}")

    args.output_dir.mkdir(parents=True, exist_ok=True)
    raw_files = list(args.input_dir.glob(
        '*.raw')) + list(args.input_dir.glob('*.CR2')) + list(args.input_dir.glob('*.NEF'))

    if not raw_files:
        logger.warning(f"No RAW images found in directory: {args.input_dir}")
        return

    def process_image(file_path: Path):
        """
        Process a single image file.
        """
        try:
            output_file = args.output_dir / \
                f"{file_path.stem}.{args.format.value}"
            processor = RawImageProcessor(raw_path=file_path)

            # Execute actions in the order provided
            for action in args.actions:
                if action == 'adjust_contrast':
                    processor.adjust_contrast(alpha=1.2)
                elif action == 'adjust_brightness':
                    processor.adjust_brightness(beta=30)
                elif action == 'apply_sharpening':
                    processor.apply_sharpening()
                elif action == 'apply_gamma_correction':
                    processor.apply_gamma_correction(gamma=1.3)
                elif action == 'rotate_image':
                    processor.rotate_image(angle=30)
                elif action == 'resize_image':
                    processor.resize_image(width=1024)
                elif action == 'adjust_color_balance':
                    processor.adjust_color_balance(
                        red=1.1, green=1.0, blue=0.9)
                elif action == 'apply_blur':
                    processor.apply_blur(ksize=(5, 5), method="gaussian")
                elif action == 'histogram_equalization':
                    processor.histogram_equalization()
                elif action == 'convert_to_grayscale':
                    processor.convert_to_grayscale()
                else:
                    logger.warning(f"Unknown action: {action}")

            # Save the processed image
            processor.save_image(
                output_path=output_file, file_format=args.format, jpeg_quality=args.jpeg_quality)
            logger.info(f"Processed image saved to {output_file}")
        except Exception as e:
            logger.error(f"Failed to process image {file_path}: {e}")

    with concurrent.futures.ThreadPoolExecutor() as executor:
        executor.map(process_image, raw_files)

    logger.info("Batch processing completed successfully.")


def main():
    """
    Main function to parse arguments and execute appropriate functions.
    """
    args = parse_cli_arguments()

    if args.command == 'process':
        process_single_image(args)
    elif args.command == 'batch':
        batch_process_images(args)
    else:
        logger.error(f"Unknown command: {args.command}")
        sys.exit(1)


if __name__ == "__main__":
    main()
