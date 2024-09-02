import rawpy
import cv2
import numpy as np

class RawImageProcessor:
    def __init__(self, raw_path):
        """初始化并读取RAW图像"""
        self.raw_path = raw_path
        self.raw = rawpy.imread(raw_path)
        self.rgb_image = self.raw.postprocess(
            gamma=(1.0, 1.0),
            no_auto_bright=True,
            use_camera_wb=True,
            output_bps=8
        )
        # 转换为OpenCV使用的BGR格式
        self.bgr_image = cv2.cvtColor(self.rgb_image, cv2.COLOR_RGB2BGR)
    
    def adjust_contrast(self, alpha=1.0):
        """调整图像对比度"""
        self.bgr_image = cv2.convertScaleAbs(self.bgr_image, alpha=alpha)
    
    def adjust_brightness(self, beta=0):
        """调整图像亮度"""
        self.bgr_image = cv2.convertScaleAbs(self.bgr_image, beta=beta)
    
    def apply_sharpening(self):
        """应用图像锐化"""
        kernel = np.array([[0, -1, 0], 
                           [-1, 5,-1], 
                           [0, -1, 0]])
        self.bgr_image = cv2.filter2D(self.bgr_image, -1, kernel)
    
    def apply_gamma_correction(self, gamma=1.0):
        """应用Gamma校正"""
        inv_gamma = 1.0 / gamma
        table = np.array([((i / 255.0) ** inv_gamma) * 255
                          for i in np.arange(0, 256)]).astype("uint8")
        self.bgr_image = cv2.LUT(self.bgr_image, table)
    
    def save_image(self, output_path, file_format="png", jpeg_quality=90):
        """保存图像为指定格式"""
        if file_format.lower() == "jpg" or file_format.lower() == "jpeg":
            cv2.imwrite(output_path, self.bgr_image, [cv2.IMWRITE_JPEG_QUALITY, jpeg_quality])
        else:
            cv2.imwrite(output_path, self.bgr_image)
    
    def show_image(self, window_name="Image"):
        """显示处理后的图像"""
        cv2.imshow(window_name, self.bgr_image)
        cv2.waitKey(0)
        cv2.destroyAllWindows()

    def get_bgr_image(self):
        """返回处理后的BGR图像"""
        return self.bgr_image

    def reset(self):
        """重置图像到最初的状态"""
        self.bgr_image = cv2.cvtColor(self.rgb_image, cv2.COLOR_RGB2BGR)

# 使用示例
if __name__ == "__main__":
    # 初始化RAW图像处理器
    processor = RawImageProcessor('path_to_your_image.raw')
    
    # 调整对比度
    processor.adjust_contrast(alpha=1.3)
    
    # 调整亮度
    processor.adjust_brightness(beta=20)
    
    # 应用锐化
    processor.apply_sharpening()
    
    # 应用Gamma校正
    processor.apply_gamma_correction(gamma=1.2)
    
    # 显示处理后的图像
    processor.show_image()

    # 保存处理后的图像
    processor.save_image('output_image.png')

    # 重置图像
    processor.reset()

    # 进行其他处理并保存为JPEG
    processor.adjust_contrast(alpha=1.1)
    processor.save_image('output_image.jpg', file_format="jpg", jpeg_quality=85)
