import concurrent.futures
from pathlib import Path
from typing import Optional, Tuple, Literal, List
from enum import Enum
import cv2
from loguru import logger
from PIL import Image, ImageDraw, ImageFont
import numpy as np
import argparse
import sys


# Configure Loguru logger with file rotation and different log levels
logger.remove()  # Remove the default logger
logger.add(sys.stderr, level="INFO",
           format="{time:YYYY-MM-DD HH:mm:ss} | {level} | {message}")
logger.add("resample_processor.log", rotation="10 MB", retention="10 days",
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
class Resampler:
    input_image_path: Path
    output_image_path: Path
    width: Optional[int] = None
    height: Optional[int] = None
    scale: Optional[float] = None
    resolution: Optional[Tuple[int, int]] = None
    interpolation: Literal[cv2.INTER_LINEAR,
                           cv2.INTER_CUBIC, cv2.INTER_NEAREST] = cv2.INTER_LINEAR
    preserve_aspect_ratio: bool = True
    crop_area: Optional[Tuple[int, int, int, int]] = None
    edge_detection: bool = False
    color_space: Literal['BGR', 'GRAY', 'HSV', 'RGB'] = 'BGR'
    brightness: float = 1.0
    contrast: float = 1.0
    sharpen: bool = False
    rotate_angle: float = 0.0
    blur: Optional[Tuple[int, int]] = None
    blur_method: Literal['gaussian', 'median', 'bilateral'] = 'gaussian'
    histogram_equalization: bool = False
    grayscale: bool = False
    output_format: ImageFormat = ImageFormat.JPEG
    jpeg_quality: int = 90

    def process(self) -> None:
        """
        Process the image with the specified parameters.
        """
        logger.info(f"Processing image: {self.input_image_path}")
        img = cv2.imread(str(self.input_image_path))
        if img is None:
            logger.error(f"Cannot load image from {self.input_image_path}")
            raise ValueError(f"Cannot load image from {self.input_image_path}")

        original_height, original_width = img.shape[:2]
        logger.debug(
            f"Original dimensions: width={original_width}, height={original_height}")

        # Crop if needed
        if self.crop_area:
            x, y, w, h = self.crop_area
            logger.debug(f"Cropping image: x={x}, y={y}, w={w}, h={h}")
            img = img[y:y+h, x:x+w]

        # Edge detection
        if self.edge_detection:
            logger.debug("Applying edge detection.")
            img = cv2.Canny(img, 100, 200)

        # Convert color space if needed
        img = self.change_color_space(img, self.color_space)

        # Adjust brightness and contrast
        if self.brightness != 1.0 or self.contrast != 1.0:
            logger.debug(
                f"Adjusting brightness by {self.brightness} and contrast by {self.contrast}")
            img = self.adjust_brightness_contrast(
                img, self.brightness, self.contrast)

        # Apply sharpening
        if self.sharpen:
            logger.debug("Applying sharpening.")
            img = self.apply_sharpening(img)

        # Apply blur if specified
        if self.blur:
            logger.debug(
                f"Applying {self.blur_method} blur with kernel size={self.blur}")
            img = self.apply_blur(img, self.blur, self.blur_method)

        # Histogram equalization
        if self.histogram_equalization:
            logger.debug("Applying histogram equalization.")
            img = self.histogram_equalization_func(img)

        # Convert to grayscale if needed
        if self.grayscale:
            logger.debug("Converting image to grayscale.")
            img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

        # Calculate new dimensions
        new_width, new_height = self.calculate_new_dimensions(
            original_width, original_height, img)
        logger.debug(
            f"Resizing image to width={new_width}, height={new_height} with interpolation={self.interpolation}")

        # Perform resizing
        resized_img = cv2.resize(
            img, (new_width, new_height), interpolation=self.interpolation)

        # Rotate image if needed
        if self.rotate_angle != 0.0:
            logger.debug(f"Rotating image by {self.rotate_angle} degrees.")
            resized_img = self.rotate_image(resized_img, self.rotate_angle)

        # Convert back to BGR if color_space was changed to RGB for saving with OpenCV
        if self.color_space == 'RGB' and len(resized_img.shape) == 3:
            logger.debug("Converting image from RGB back to BGR for saving.")
            resized_img = cv2.cvtColor(resized_img, cv2.COLOR_RGB2BGR)
        elif self.color_space == 'HSV' and len(resized_img.shape) == 3:
            logger.debug("Converting image from HSV back to BGR for saving.")
            resized_img = cv2.cvtColor(resized_img, cv2.COLOR_HSV2BGR)

        # Save the image with specified resolution if provided
        if self.resolution:
            logger.debug(
                f"Saving image with resolution: {self.resolution} DPI")
            pil_img = Image.fromarray(
                cv2.cvtColor(resized_img, cv2.COLOR_BGR2RGB) if len(resized_img.shape) == 3 else resized_img)
            pil_img.save(str(self.output_image_path), dpi=self.resolution,
                         format=self.output_format.name.upper())
        else:
            logger.debug(
                f"Saving image without changing resolution to format: {self.output_format.value}")
            if self.output_format == ImageFormat.JPEG:
                cv2.imwrite(str(self.output_image_path), resized_img, [
                            cv2.IMWRITE_JPEG_QUALITY, self.jpeg_quality])
            else:
                cv2.imwrite(str(self.output_image_path), resized_img)

        logger.info(f"Image saved successfully to: {self.output_image_path}")

    @staticmethod
    def change_color_space(img: np.ndarray, color_space: str) -> np.ndarray:
        """
        Change the color space of the image.

        :param img: Input image in BGR format.
        :param color_space: Target color space.
        :return: Image in the target color space.
        """
        if color_space == 'GRAY':
            return cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        elif color_space == 'HSV':
            return cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
        elif color_space == 'RGB':
            return cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        return img

    @staticmethod
    def adjust_brightness_contrast(img: np.ndarray, brightness: float, contrast: float) -> np.ndarray:
        """
        Adjust the brightness and contrast of the image.

        :param img: Input image.
        :param brightness: Brightness factor.
        :param contrast: Contrast factor.
        :return: Adjusted image.
        """
        return cv2.convertScaleAbs(img, alpha=contrast, beta=brightness * 50)

    @staticmethod
    def apply_sharpening(img: np.ndarray) -> np.ndarray:
        """
        Apply sharpening to the image.

        :param img: Input image.
        :return: Sharpened image.
        """
        kernel = np.array([[0, -1, 0],
                           [-1, 5, -1],
                           [0, -1, 0]])
        return cv2.filter2D(img, -1, kernel)

    @staticmethod
    def apply_blur(img: np.ndarray, ksize: Tuple[int, int], method: str) -> np.ndarray:
        """
        Apply blur to the image.

        :param img: Input image.
        :param ksize: Kernel size for the blur.
        :param method: Blurring method ('gaussian', 'median', 'bilateral').
        :return: Blurred image.
        """
        if method.lower() == "gaussian":
            return cv2.GaussianBlur(img, ksize, 0)
        elif method.lower() == "median":
            return cv2.medianBlur(img, ksize[0])
        elif method.lower() == "bilateral":
            return cv2.bilateralFilter(img, d=9, sigmaColor=75, sigmaSpace=75)
        else:
            logger.error(f"Unsupported blur method: {method}")
            raise ValueError(f"Unsupported blur method: {method}")

    @staticmethod
    def histogram_equalization_func(img: np.ndarray) -> np.ndarray:
        """
        Apply histogram equalization to the image.

        :param img: Input image.
        :return: Image after histogram equalization.
        """
        if len(img.shape) == 2:
            return cv2.equalizeHist(img)
        else:
            ycrcb = cv2.cvtColor(img, cv2.COLOR_BGR2YCrCb)
            ycrcb[:, :, 0] = cv2.equalizeHist(ycrcb[:, :, 0])
            return cv2.cvtColor(ycrcb, cv2.COLOR_YCrCb2BGR)

    @staticmethod
    def rotate_image(img: np.ndarray, angle: float) -> np.ndarray:
        """
        Rotate the image by the given angle.

        :param img: Input image.
        :param angle: Angle to rotate the image.
        :return: Rotated image.
        """
        (h, w) = img.shape[:2]
        center = (w // 2, h // 2)
        M = cv2.getRotationMatrix2D(center, angle, 1.0)
        return cv2.warpAffine(img, M, (w, h))


def parse_arguments() -> argparse.Namespace:
    """
    Parse command-line arguments for the resample script.

    :return: Parsed arguments.
    """
    parser = argparse.ArgumentParser(
        description="Image Resampling and Processing Tool")
    subparsers = parser.add_subparsers(
        dest='command', required=True, help='Available commands')

    # Subparser for single image processing
    process_parser = subparsers.add_parser(
        'process', help='Process a single image.')
    process_parser.add_argument(
        '--input', type=Path, required=True, help='Path to the input image.')
    process_parser.add_argument(
        '--output', type=Path, required=True, help='Path to save the processed image.')
    process_parser.add_argument(
        '--width', type=int, help='Desired width in pixels.')
    process_parser.add_argument(
        '--height', type=int, help='Desired height in pixels.')
    process_parser.add_argument(
        '--scale', type=float, help='Scale factor for resizing (e.g., 0.5 for half size).')
    process_parser.add_argument('--resolution', type=int, nargs=2, metavar=('X_RES', 'Y_RES'),
                                help='Output resolution in DPI (e.g., 300 300).')
    process_parser.add_argument('--interpolation', type=str, choices=['linear', 'cubic', 'nearest'],
                                default='linear', help='Interpolation method.')
    process_parser.add_argument('--preserve_aspect_ratio', action='store_true',
                                help='Preserve aspect ratio when resizing.')
    process_parser.add_argument('--crop_area', type=int, nargs=4, metavar=('X', 'Y', 'W', 'H'),
                                help='Crop area as four integers: x y width height.')
    process_parser.add_argument('--edge_detection', action='store_true',
                                help='Apply edge detection before resizing.')
    process_parser.add_argument('--color_space', type=str, choices=['BGR', 'GRAY', 'HSV', 'RGB'],
                                default='BGR', help='Color space to convert the image to.')
    process_parser.add_argument('--brightness', type=float, default=1.0,
                                help='Brightness adjustment factor (1.0 = no change).')
    process_parser.add_argument('--contrast', type=float, default=1.0,
                                help='Contrast adjustment factor (1.0 = no change).')
    process_parser.add_argument('--sharpen', action='store_true',
                                help='Apply sharpening to the image.')
    process_parser.add_argument('--rotate_angle', type=float, default=0.0,
                                help='Angle to rotate the image in degrees.')
    process_parser.add_argument('--blur', type=int, nargs=2, metavar=('WIDTH', 'HEIGHT'),
                                help='Kernel size for blurring.')
    process_parser.add_argument('--blur_method', type=str, choices=['gaussian', 'median', 'bilateral'],
                                default='gaussian', help='Method to use for blurring.')
    process_parser.add_argument('--histogram_equalization', action='store_true',
                                help='Apply histogram equalization to enhance contrast.')
    process_parser.add_argument('--grayscale', action='store_true',
                                help='Convert image to grayscale.')
    process_parser.add_argument('--output_format', type=str, choices=ImageFormat.list(),
                                default='jpg', help='Output image format.')
    process_parser.add_argument('--jpeg_quality', type=int, default=90,
                                help='JPEG quality (only applicable if output format is JPEG).')

    # Subparser for batch processing
    batch_parser = subparsers.add_parser(
        'batch', help='Batch process multiple images.')
    batch_parser.add_argument(
        '--input_dir', type=Path, required=True, help='Directory containing input images.')
    batch_parser.add_argument(
        '--output_dir', type=Path, required=True, help='Directory to save processed images.')
    batch_parser.add_argument(
        '--width', type=int, help='Desired width in pixels.')
    batch_parser.add_argument('--height', type=int,
                              help='Desired height in pixels.')
    batch_parser.add_argument(
        '--scale', type=float, help='Scale factor for resizing (e.g., 0.5 for half size).')
    batch_parser.add_argument('--resolution', type=int, nargs=2, metavar=('X_RES', 'Y_RES'),
                              help='Output resolution in DPI (e.g., 300 300).')
    batch_parser.add_argument('--interpolation', type=str, choices=['linear', 'cubic', 'nearest'],
                              default='linear', help='Interpolation method.')
    batch_parser.add_argument('--preserve_aspect_ratio', action='store_true',
                              help='Preserve aspect ratio when resizing.')
    batch_parser.add_argument('--crop_area', type=int, nargs=4, metavar=('X', 'Y', 'W', 'H'),
                              help='Crop area as four integers: x y width height.')
    batch_parser.add_argument('--edge_detection', action='store_true',
                              help='Apply edge detection before resizing.')
    batch_parser.add_argument('--color_space', type=str, choices=['BGR', 'GRAY', 'HSV', 'RGB'],
                              default='BGR', help='Color space to convert the image to.')
    batch_parser.add_argument('--brightness', type=float, default=1.0,
                              help='Brightness adjustment factor (1.0 = no change).')
    batch_parser.add_argument('--contrast', type=float, default=1.0,
                              help='Contrast adjustment factor (1.0 = no change).')
    batch_parser.add_argument('--sharpen', action='store_true',
                              help='Apply sharpening to the images.')
    batch_parser.add_argument('--rotate_angle', type=float, default=0.0,
                              help='Angle to rotate the images in degrees.')
    batch_parser.add_argument('--blur', type=int, nargs=2, metavar=('WIDTH', 'HEIGHT'),
                              help='Kernel size for blurring.')
    batch_parser.add_argument('--blur_method', type=str, choices=['gaussian', 'median', 'bilateral'],
                              default='gaussian', help='Method to use for blurring.')
    batch_parser.add_argument('--histogram_equalization', action='store_true',
                              help='Apply histogram equalization to enhance contrast.')
    batch_parser.add_argument('--grayscale', action='store_true',
                              help='Convert images to grayscale.')
    batch_parser.add_argument('--output_format', type=str, choices=ImageFormat.list(),
                              default='jpg', help='Output image format.')
    batch_parser.add_argument('--jpeg_quality', type=int, default=90,
                              help='JPEG quality (only applicable if output format is JPEG).')

    return parser.parse_args()


def process_image(resampler: Resampler) -> None:
    """
    Wrapper function to process an image using the Resampler class.

    :param resampler: Resampler instance with all parameters set.
    """
    try:
        resampler.process()
    except Exception as e:
        logger.error(
            f"Error processing image {resampler.input_image_path}: {e}")


def main():
    """
    Main function to parse arguments and execute processing.
    """
    args = parse_arguments()

    if args.command == 'process':
        resampler = Resampler(
            input_image_path=args.input,
            output_image_path=args.output,
            width=args.width,
            height=args.height,
            scale=args.scale,
            resolution=tuple(args.resolution) if args.resolution else None,
            interpolation={
                'linear': cv2.INTER_LINEAR,
                'cubic': cv2.INTER_CUBIC,
                'nearest': cv2.INTER_NEAREST
            }[args.interpolation],
            preserve_aspect_ratio=args.preserve_aspect_ratio,
            crop_area=tuple(args.crop_area) if args.crop_area else None,
            edge_detection=args.edge_detection,
            color_space=args.color_space,
            brightness=args.brightness,
            contrast=args.contrast,
            sharpen=args.sharpen,
            rotate_angle=args.rotate_angle,
            blur=tuple(args.blur) if args.blur else None,
            blur_method=args.blur_method,
            histogram_equalization=args.histogram_equalization,
            grayscale=args.grayscale,
            output_format=ImageFormat(args.output_format),
            jpeg_quality=args.jpeg_quality
        )
        process_image(resampler)

    elif args.command == 'batch':
        input_dir = args.input_dir
        output_dir = args.output_dir

        if not input_dir.exists() or not input_dir.is_dir():
            logger.error(
                f"Input directory does not exist or is not a directory: {input_dir}")
            sys.exit(1)

        output_dir.mkdir(parents=True, exist_ok=True)
        # You might want to filter specific extensions
        image_files = list(input_dir.glob('*'))

        logger.info(f"Found {len(image_files)} files to process in batch.")

        resamplers = [
            Resampler(
                input_image_path=file,
                output_image_path=output_dir /
                f"{file.stem}.{args.output_format}",
                width=args.width,
                height=args.height,
                scale=args.scale,
                resolution=tuple(args.resolution) if args.resolution else None,
                interpolation={
                    'linear': cv2.INTER_LINEAR,
                    'cubic': cv2.INTER_CUBIC,
                    'nearest': cv2.INTER_NEAREST
                }[args.interpolation],
                preserve_aspect_ratio=args.preserve_aspect_ratio,
                crop_area=tuple(args.crop_area) if args.crop_area else None,
                edge_detection=args.edge_detection,
                color_space=args.color_space,
                brightness=args.brightness,
                contrast=args.contrast,
                sharpen=args.sharpen,
                rotate_angle=args.rotate_angle,
                blur=tuple(args.blur) if args.blur else None,
                blur_method=args.blur_method,
                histogram_equalization=args.histogram_equalization,
                grayscale=args.grayscale,
                output_format=ImageFormat(args.output_format),
                jpeg_quality=args.jpeg_quality
            )
            for file in image_files
            if file.suffix.lower() in ['.jpg', '.jpeg', '.png', '.tiff', '.bmp', '.gif']
        ]

        logger.info(
            f"Starting batch processing with {len(resamplers)} images.")

        with concurrent.futures.ThreadPoolExecutor() as executor:
            executor.map(process_image, resamplers)

        logger.info("Batch processing completed successfully.")


if __name__ == "__main__":
    main()
