python
import rawpy
import cv2
import numpy as np
from typing import Optional, Tuple
from loguru import logger
from dataclasses import dataclass, field

@dataclass
class RawImageProcessor:
    raw_path: str
    raw: rawpy.RawPy = field(init=False)
    rgb_image: np.ndarray = field(init=False)
    bgr_image: np.ndarray = field(init=False)

    def __post_init__(self):
        """
        Initialize and read the RAW image.

        This method is called automatically after the dataclass is initialized.
        It reads the RAW image from the specified path and processes it into an RGB image.
        """
        logger.debug(f"Initializing RawImageProcessor, RAW file path: {self.raw_path}")
        try:
            self.raw = rawpy.imread(self.raw_path)
            logger.info(f"Successfully read RAW image: {self.raw_path}")
        except Exception as e:
            logger.error(f"Failed to read RAW image: {e}")
            raise IOError(f"Cannot read RAW image: {self.raw_path}")

        try:
            self.rgb_image = self.raw.postprocess(
                gamma=(1.0, 1.0),
                no_auto_bright=True,
                use_camera_wb=True,
                output_bps=16  # Use higher bit depth
            )
            logger.debug("Successfully post-processed RAW image")
        except Exception as e:
            logger.error(f"Failed to post-process RAW image: {e}")
            raise ValueError("RAW image post-processing failed")

        self.bgr_image = cv2.cvtColor(self.rgb_image, cv2.COLOR_RGB2BGR)
        logger.debug("Converted to OpenCV BGR format")

    def adjust_contrast(self, alpha: float = 1.0) -> None:
        """
        Adjust the contrast of the image.

        :param alpha: Contrast control (1.0-3.0). Default is 1.0.
        """
        logger.debug(f"Adjusting contrast, alpha={alpha}")
        try:
            self.bgr_image = cv2.convertScaleAbs(self.bgr_image, alpha=alpha)
            logger.info(f"Contrast adjusted successfully, alpha={alpha}")
        except Exception as e:
            logger.error(f"Failed to adjust contrast: {e}")
            raise RuntimeError("Contrast adjustment failed")

    def adjust_brightness(self, beta: int = 0) -> None:
        """
        Adjust the brightness of the image.

        :param beta: Brightness control (0-100). Default is 0.
        """
        logger.debug(f"Adjusting brightness, beta={beta}")
        try:
            self.bgr_image = cv2.convertScaleAbs(self.bgr_image, beta=beta)
            logger.info(f"Brightness adjusted successfully, beta={beta}")
        except Exception as e:
            logger.error(f"Failed to adjust brightness: {e}")
            raise RuntimeError("Brightness adjustment failed")

    def apply_sharpening(self) -> None:
        """
        Apply sharpening to the image.

        This method uses a kernel to sharpen the image.
        """
        logger.debug("Applying sharpening")
        try:
            kernel = np.array([[0, -1, 0],
                               [-1, 5, -1],
                               [0, -1, 0]])
            self.bgr_image = cv2.filter2D(self.bgr_image, -1, kernel)
            logger.info("Image sharpening applied successfully")
        except Exception as e:
            logger.error(f"Failed to apply sharpening: {e}")
            raise RuntimeError("Sharpening application failed")

    def apply_gamma_correction(self, gamma: float = 1.0) -> None:
        """
        Apply gamma correction to the image.

        :param gamma: Gamma value (0.1-3.0). Default is 1.0.
        """
        logger.debug(f"Applying gamma correction, gamma={gamma}")
        try:
            inv_gamma = 1.0 / gamma
            table = np.array([((i / 255.0) ** inv_gamma) * 255
                              for i in np.arange(256)]).astype("uint8")
            self.bgr_image = cv2.LUT(self.bgr_image, table)
            logger.info(f"Gamma correction applied successfully, gamma={gamma}")
        except Exception as e:
            logger.error(f"Failed to apply gamma correction: {e}")
            raise RuntimeError("Gamma correction application failed")

    def rotate_image(self, angle: float, center: Optional[Tuple[int, int]] = None, scale: float = 1.0) -> None:
        """
        Rotate the image.

        :param angle: Angle to rotate the image.
        :param center: Center of rotation. If None, the center of the image is used.
        :param scale: Scale factor. Default is 1.0.
        """
        logger.debug(f"Rotating image, angle={angle}, scale={scale}")
        try:
            (h, w) = self.bgr_image.shape[:2]
            if center is None:
                center = (w // 2, h // 2)
            M = cv2.getRotationMatrix2D(center, angle, scale)
            self.bgr_image = cv2.warpAffine(self.bgr_image, M, (w, h))
            logger.info(f"Image rotated successfully, angle={angle}, scale={scale}")
        except Exception as e:
            logger.error(f"Failed to rotate image: {e}")
            raise RuntimeError("Image rotation failed")

    def resize_image(self, width: Optional[int] = None, height: Optional[int] = None, inter: int = cv2.INTER_LINEAR) -> None:
        """
        Resize the image.

        :param width: New width of the image. If None, the width is scaled proportionally.
        :param height: New height of the image. If None, the height is scaled proportionally.
        :param inter: Interpolation method. Default is cv2.INTER_LINEAR.
        """
        logger.debug(f"Resizing image, width={width}, height={height}")
        try:
            (h, w) = self.bgr_image.shape[:2]
            if width is None and height is None:
                logger.warning("Both width and height are not specified, skipping resize")
                return
            if width is None:
                r = height / float(h)
                dim = (int(w * r), height)
            elif height is None:
                r = width / float(w)
                dim = (width, int(h * r))
            else:
                dim = (width, height)
            self.bgr_image = cv2.resize(self.bgr_image, dim, interpolation=inter)
            logger.info(f"Image resized successfully, new dimensions={dim}")
        except Exception as e:
            logger.error(f"Failed to resize image: {e}")
            raise RuntimeError("Image resize failed")

    def adjust_color_balance(self, red: float = 1.0, green: float = 1.0, blue: float = 1.0) -> None:
        """
        Adjust the color balance of the image.

        :param red: Red channel multiplier. Default is 1.0.
        :param green: Green channel multiplier. Default is 1.0.
        :param blue: Blue channel multiplier. Default is 1.0.
        """
        logger.debug(f"Adjusting color balance, red={red}, green={green}, blue={blue}")
        try:
            b, g, r = cv2.split(self.bgr_image)
            r = cv2.convertScaleAbs(r, alpha=red)
            g = cv2.convertScaleAbs(g, alpha=green)
            b = cv2.convertScaleAbs(b, alpha=blue)
            self.bgr_image = cv2.merge([b, g, r])
            logger.info(f"Color balance adjusted successfully, red={red}, green={green}, blue={blue}")
        except Exception as e:
            logger.error(f"Failed to adjust color balance: {e}")
            raise RuntimeError("Color balance adjustment failed")

    def save_image(self, output_path: str, file_format: str = "png", jpeg_quality: int = 90) -> None:
        """
        Save the image to the specified path.

        :param output_path: Path to save the image.
        :param file_format: Format to save the image. Default is "png".
        :param jpeg_quality: Quality for JPEG format. Default is 90.
        """
        logger.debug(f"Saving image, path={output_path}, format={file_format}, JPEG quality={jpeg_quality}")
        try:
            if file_format.lower() in ["jpg", "jpeg"]:
                cv2.imwrite(output_path, self.bgr_image, [cv2.IMWRITE_JPEG_QUALITY, jpeg_quality])
                logger.info(f"Image saved successfully as JPEG: {output_path}, quality={jpeg_quality}")
            else:
                cv2.imwrite(output_path, self.bgr_image)
                logger.info(f"Image saved successfully as {file_format.upper()}: {output_path}")
        except Exception as e:
            logger.error(f"Failed to save image: {e}")
            raise IOError("Image save failed")

    def show_image(self, window_name: str = "Image") -> None:
        """
        Display the processed image.

        :param window_name: Name of the window in which to display the image. Default is "Image".
        """
        logger.debug(f"Displaying image window, name={window_name}")
        try:
            cv2.imshow(window_name, self.bgr_image)
            cv2.waitKey(0)
            cv2.destroyAllWindows()
            logger.info("Image displayed successfully")
        except Exception as e:
            logger.error(f"Failed to display image: {e}")
            raise RuntimeError("Image display failed")

    def get_bgr_image(self) -> np.ndarray:
        """
        Return the processed BGR image.

        :return: Processed BGR image as a NumPy array.
        """
        logger.debug("Getting processed BGR image")
        return self.bgr_image

    def reset(self) -> None:
        """
        Reset the image to its original state.

        This method resets the image to the state it was in after initial processing.
        """
        logger.debug("Resetting image to original state")
        try:
            self.bgr_image = cv2.cvtColor(self.rgb_image, cv2.COLOR_RGB2BGR)
            logger.info("Image reset successfully")
        except Exception as e:
            logger.error(f"Failed to reset image: {e}")
            raise RuntimeError("Image reset failed")

    def convert_to_grayscale(self) -> None:
        """
        Convert the image to grayscale.

        This method converts the processed BGR image to a grayscale image.
        """
        logger.debug("Converting image to grayscale")
        try:
            self.bgr_image = cv2.cvtColor(self.bgr_image, cv2.COLOR_BGR2GRAY)
            logger.info("Image converted to grayscale successfully")
        except Exception as e:
            logger.error(f"Failed to convert image to grayscale: {e}")
            raise RuntimeError("Grayscale conversion failed")

    def apply_blur(self, ksize: Tuple[int, int] = (5, 5)) -> None:
        """
        Apply blur to the image.

        :param ksize: Kernel size for the blur. Default is (5, 5).
        """
        logger.debug(f"Applying blur, kernel size={ksize}")
        try:
            self.bgr_image = cv2.GaussianBlur(self.bgr_image, ksize, 0)
            logger.info(f"Blur applied successfully, kernel size={ksize}")
        except Exception as e:
            logger.error(f"Failed to apply blur: {e}")
            raise RuntimeError("Blur application failed")

    def histogram_equalization(self) -> None:
        """
        Apply histogram equalization to the image.

        This method enhances the contrast of the image using histogram equalization.
        """
        logger.debug("Applying histogram equalization")
        try:
            if len(self.bgr_image.shape) == 2:
                self.bgr_image = cv2.equalizeHist(self.bgr_image)
            else:
                ycrcb = cv2.cvtColor(self.bgr_image, cv2.COLOR_BGR2YCrCb)
                ycrcb[:, :, 0] = cv2.equalizeHist(ycrcb[:, :, 0])
                self.bgr_image = cv2.cvtColor(ycrcb, cv2.COLOR_YCrCb2BGR)
            logger.info("Histogram equalization applied successfully")
        except Exception as e:
            logger.error(f"Failed to apply histogram equalization: {e}")
            raise RuntimeError("Histogram equalization application failed")

    def to_rgb_image(self) -> np.ndarray:
        """
        Return the processed RGB image.

        :return: Processed RGB image as a NumPy array.
        """
        logger.debug("Converting and getting processed RGB image")
        try:
            rgb_image = cv2.cvtColor(self.bgr_image, cv2.COLOR_BGR2RGB)
            logger.info("Successfully converted to RGB image")
            return rgb_image
        except Exception as e:
            logger.error(f"Failed to convert to RGB image: {e}")
            raise RuntimeError("RGB conversion failed")

    def to_raw_image(self) -> rawpy.RawPy:
        """
        Return the original RAW image.

        :return: Original RAW image as a rawpy.RawPy object.
        """
        logger.debug("Getting original RAW image")
        return self.raw

# Usage example
if __name__ == "__main__":
    # Initialize RAW image processor
    processor = RawImageProcessor('path_to_your_image.raw')

    # Adjust contrast
    processor.adjust_contrast(alpha=1.3)

    # Adjust brightness
    processor.adjust_brightness(beta=20)

    # Apply sharpening
    processor.apply_sharpening()

    # Apply gamma correction
    processor.apply_gamma_correction(gamma=1.2)

    # Rotate image
    processor.rotate_image(angle=45)

    # Resize image
    processor.resize_image(width=800)

    # Adjust color balance
    processor.adjust_color_balance(red=1.1, green=1.0, blue=0.9)

    # Apply blur
    processor.apply_blur(ksize=(5, 5))

    # Apply histogram equalization
    processor.histogram_equalization()

    # Display processed image
    processor.show_image()

    # Save processed image
    processor.save_image('output_image.png')

    # Reset image
    processor.reset()

    # Perform other processing and save as JPEG
    processor.adjust_contrast(alpha=1.1)
    processor.save_image('output_image.jpg', file_format="jpg", jpeg_quality=85)