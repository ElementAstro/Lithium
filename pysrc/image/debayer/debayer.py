import numpy as np
import cv2
from concurrent.futures import ThreadPoolExecutor
from typing import Optional, Tuple
import time


class Debayer:
    def __init__(self, method: str = 'bilinear', pattern: Optional[str] = None):
        """
        Initialize Debayer object with method and optional Bayer pattern.

        :param method: Debayering method ('superpixel', 'bilinear', 'vng', 'ahd', 'laplacian')
        :param pattern: Bayer pattern ('BGGR', 'RGGB', 'GBRG', 'GRBG'), None for auto-detection
        """
        self.method = method
        self.pattern = pattern

    def detect_bayer_pattern(self, image: np.ndarray) -> str:
        """
        Automatically detect Bayer pattern from the CFA image.
        """
        height, width = image.shape

        # 初始化统计信息
        patterns = {'BGGR': 0, 'RGGB': 0, 'GBRG': 0, 'GRBG': 0}

        # 检测边缘并增强检测精度
        edges = cv2.Canny(image, 50, 150)

        for i in range(0, height - 1, 2):
            for j in range(0, width - 1, 2):
                # BGGR
                patterns['BGGR'] += (image[i, j] + image[i+1, j+1]) + \
                    (edges[i, j] + edges[i+1, j+1])
                # RGGB
                patterns['RGGB'] += (image[i+1, j] + image[i, j+1]) + \
                    (edges[i+1, j] + edges[i, j+1])
                # GBRG
                patterns['GBRG'] += (image[i, j+1] + image[i+1, j]) + \
                    (edges[i, j+1] + edges[i+1, j])
                # GRBG
                patterns['GRBG'] += (image[i, j] + image[i+1, j+1]) + \
                    (edges[i, j] + edges[i+1, j+1])

        # 分析颜色通道的分布，进一步强化检测
        color_sums = {'BGGR': 0, 'RGGB': 0, 'GBRG': 0, 'GRBG': 0}

        # 遍历整个图像并分析色彩通道的强度分布
        for i in range(0, height - 1, 2):
            for j in range(0, width - 1, 2):
                block = image[i:i+2, j:j+2]
                color_sums['BGGR'] += block[0, 0] + block[1, 1]  # 蓝-绿对
                color_sums['RGGB'] += block[1, 0] + block[0, 1]  # 红-绿对
                color_sums['GBRG'] += block[0, 1] + block[1, 0]  # 绿-蓝对
                color_sums['GRBG'] += block[0, 0] + block[1, 1]  # 绿-红对

        # 综合所有信息，选择最有可能的Bayer模式
        for pattern in patterns.keys():
            patterns[pattern] += color_sums[pattern]

        detected_pattern = max(patterns, key=patterns.get)

        return detected_pattern

    def debayer_image(self, cfa_image: np.ndarray) -> np.ndarray:
        """
        Perform the debayering process using the specified method.
        """
        if self.pattern is None:
            self.pattern = self.detect_bayer_pattern(cfa_image)

        cfa_image = self.extend_image_edges(cfa_image, pad_width=2)
        print(f"Using Bayer pattern: {self.pattern}")

        if self.method == 'superpixel':
            return self.debayer_superpixel(cfa_image)
        elif self.method == 'bilinear':
            return self.debayer_bilinear(cfa_image)
        elif self.method == 'vng':
            return self.debayer_vng(cfa_image)
        elif self.method == 'ahd':
            return self.parallel_debayer_ahd(cfa_image)
        elif self.method == 'laplacian':
            return self.debayer_laplacian_harmonization(cfa_image)
        else:
            raise ValueError(f"Unknown debayer method: {self.method}")

    def debayer_superpixel(self, cfa_image: np.ndarray) -> np.ndarray:
        red = cfa_image[0::2, 0::2]
        green = (cfa_image[0::2, 1::2] + cfa_image[1::2, 0::2]) / 2
        blue = cfa_image[1::2, 1::2]

        rgb_image = np.stack((red, green, blue), axis=-1)
        return rgb_image

    def debayer_bilinear(self, cfa_image, pattern='BGGR'):
        """
        使用双线性插值法进行去拜耳处理。

        :param cfa_image: 输入的CFA图像
        :param pattern: Bayer模式 ('BGGR', 'RGGB', 'GBRG', 'GRBG')
        :return: 去拜耳处理后的RGB图像
        """
        if pattern == 'BGGR':
            return cv2.cvtColor(cfa_image, cv2.COLOR_BayerBG2BGR)
        elif pattern == 'RGGB':
            return cv2.cvtColor(cfa_image, cv2.COLOR_BayerRG2BGR)
        elif pattern == 'GBRG':
            return cv2.cvtColor(cfa_image, cv2.COLOR_BayerGB2BGR)
        elif pattern == 'GRBG':
            return cv2.cvtColor(cfa_image, cv2.COLOR_BayerGR2BGR)
        else:
            raise ValueError(f"Unsupported Bayer pattern: {pattern}")

    def debayer_vng(self, cfa_image: np.ndarray) -> np.ndarray:
        code = cv2.COLOR_BayerBG2BGR_VNG if pattern == 'BGGR' else cv2.COLOR_BayerRG2BGR_VNG
        rgb_image = cv2.cvtColor(cfa_image, code)
        return rgb_image

    def parallel_debayer_ahd(self, cfa_image: np.ndarray, num_threads: int = 4) -> np.ndarray:
        height, width = cfa_image.shape
        chunk_size = height // num_threads

        # 用于存储每个线程处理的部分图像
        results = [None] * num_threads

        def process_chunk(start_row, end_row, index):
            chunk = cfa_image[start_row:end_row, :]
            gradient_x, gradient_y = calculate_gradients(chunk)
            green_channel = interpolate_green_channel(
                chunk, gradient_x, gradient_y)
            red_channel, blue_channel = interpolate_red_blue_channel(
                chunk, green_channel, pattern)
            rgb_chunk = np.stack(
                (red_channel, green_channel, blue_channel), axis=-1)
            results[index] = np.clip(rgb_chunk, 0, 255).astype(np.uint8)

        with concurrent.futures.ThreadPoolExecutor(max_workers=num_threads) as executor:
            futures = []
            for i in range(num_threads):
                start_row = i * chunk_size
                end_row = (i + 1) * chunk_size if i < num_threads - \
                    1 else height
                futures.append(executor.submit(
                    process_chunk, start_row, end_row, i))

            concurrent.futures.wait(futures)

        # 合并处理后的块
        rgb_image = np.vstack(results)
        return rgb_image

    def calculate_laplacian(self, image):
        """
        计算图像的拉普拉斯算子，用于增强边缘检测。

        :param image: 输入的图像（灰度图像）
        :return: 拉普拉斯图像
        """
        laplacian = cv2.Laplacian(image, cv2.CV_64F)
        return laplacian

    def harmonize_edges(self, original, interpolated, laplacian):
        """
        使用拉普拉斯算子结果来调整插值后的图像，增强边缘细节。

        :param original: 原始CFA图像
        :param interpolated: 双线性插值后的图像
        :param laplacian: 计算的拉普拉斯图像
        :return: 经过拉普拉斯调和的图像
        """
        return np.clip(interpolated + 0.2 * laplacian, 0, 255).astype(np.uint8)

    def debayer_laplacian_harmonization(self, cfa_image, pattern='BGGR'):
        """
        使用简化的拉普拉斯调和方法进行去拜耳处理，以增强边缘处理。

        :param cfa_image: 输入的CFA图像
        :param pattern: Bayer模式 ('BGGR', 'RGGB', 'GBRG', 'GRBG')
        :return: 去拜耳处理后的RGB图像
        """
        # Step 1: 双线性插值
        interpolated_image = self.debayer_bilinear(cfa_image, pattern)

        # Step 2: 计算每个通道的拉普拉斯图像
        laplacian_b = self.calculate_laplacian(interpolated_image[:, :, 0])
        laplacian_g = self.calculate_laplacian(interpolated_image[:, :, 1])
        laplacian_r = self.calculate_laplacian(interpolated_image[:, :, 2])

        # Step 3: 使用拉普拉斯结果调和插值后的图像
        harmonized_b = self.harmonize_edges(cfa_image, interpolated_image[:, :, 0], laplacian_b)
        harmonized_g = self.harmonize_edges(cfa_image, interpolated_image[:, :, 1], laplacian_g)
        harmonized_r = self.harmonize_edges(cfa_image, interpolated_image[:, :, 2], laplacian_r)

        # Step 4: 合并调和后的通道
        harmonized_image = np.stack((harmonized_b, harmonized_g, harmonized_r), axis=-1)

        return harmonized_image

    def extend_image_edges(self, image: np.ndarray, pad_width: int) -> np.ndarray:
        """
        Extend image edges using mirror padding to handle boundary issues during interpolation.
        """
        return np.pad(image, pad_width, mode='reflect')

    def visualize_intermediate_steps(self, cfa_image: np.ndarray):
        """
        Visualize intermediate steps in the debayering process.
        """
        gradient_x, gradient_y = calculate_gradients(cfa_image)
        green_channel = interpolate_green_channel(
            cfa_image, gradient_x, gradient_y)
        red_channel, blue_channel = interpolate_red_blue_channel(
            cfa_image, green_channel, pattern)

        # 显示梯度和各通道图像
        cv2.imshow("Gradient X", gradient_x)
        cv2.imshow("Gradient Y", gradient_y)
        cv2.imshow("Green Channel", green_channel)
        cv2.imshow("Red Channel", red_channel)
        cv2.imshow("Blue Channel", blue_channel)
        cv2.waitKey(0)
        cv2.destroyAllWindows()

    def process_batch(self, image_paths: list, num_threads: int = 4):
        """
        Batch processing for multiple CFA images using multithreading.
        """
        start_time = time.time()
        with ThreadPoolExecutor(max_workers=num_threads) as executor:
            results = executor.map(
                lambda path: self.process_single_image(path), image_paths)

        elapsed_time = time.time() - start_time
        print(f"Batch processing completed in {elapsed_time:.2f} seconds.")

    def process_single_image(self, path: str) -> np.ndarray:
        """
        Helper function for processing a single image.
        """
        cfa_image = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
        rgb_image = self.debayer_image(cfa_image)
        output_path = path.replace('.png', f'_{self.method}.png')
        cv2.imwrite(output_path, rgb_image)
        print(f"Processed {path} -> {output_path}")
        return rgb_image
