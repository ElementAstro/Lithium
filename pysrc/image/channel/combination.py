from PIL import Image
import numpy as np
from skimage import color
import cv2

def resize_to_match(image, target_size):
    return image.resize(target_size, Image.ANTIALIAS)

def load_image_as_gray(path):
    return Image.open(path).convert('L')

def combine_channels(channels, color_space='RGB'):
    match color_space:
        case 'RGB':
            return Image.merge("RGB", channels)
        
        case 'LAB':
            lab_image = Image.merge("LAB", channels)
            return lab_image.convert('RGB')
        
        case 'HSV':
            hsv_image = Image.merge("HSV", channels)
            return hsv_image.convert('RGB')

        case 'HSI':
            hsi_image = np.dstack([np.array(ch) / 255.0 for ch in channels])
            rgb_image = color.hsv2rgb(hsi_image)  # Scikit-image doesn't have direct HSI support; using HSV as proxy
            return Image.fromarray((rgb_image * 255).astype(np.uint8))

        case _:
            raise ValueError(f"Unsupported color space: {color_space}")

def channel_combination(src1_path, src2_path, src3_path, color_space='RGB'):
    # Load and resize images to match
    channel_1 = load_image_as_gray(src1_path)
    channel_2 = load_image_as_gray(src2_path)
    channel_3 = load_image_as_gray(src3_path)
    
    # Automatically resize images to match the size of the first image
    size = channel_1.size
    channel_2 = resize_to_match(channel_2, size)
    channel_3 = resize_to_match(channel_3, size)

    # Combine the channels
    combined_image = combine_channels([channel_1, channel_2, channel_3], color_space=color_space)
    
    return combined_image

# 示例用法
if __name__ == "__main__":
    # 指定通道对应的图像路径
    src1_path = 'channel_R.png'  # 对应于RGB空间的R通道或Lab空间的L通道
    src2_path = 'channel_G.png'  # 对应于RGB空间的G通道或Lab空间的a*通道
    src3_path = 'channel_B.png'  # 对应于RGB空间的B通道或Lab空间的b*通道
    
    # 执行通道组合，保存结果
    combined_rgb = channel_combination(src1_path, src2_path, src3_path, color_space='RGB')
    combined_rgb.save('combined_rgb.png')
    
    combined_lab = channel_combination(src1_path, src2_path, src3_path, color_space='LAB')
    combined_lab.save('combined_lab.png')
    
    combined_hsv = channel_combination(src1_path, src2_path, src3_path, color_space='HSV')
    combined_hsv.save('combined_hsv.png')

    combined_hsi = channel_combination(src1_path, src2_path, src3_path, color_space='HSI')
    combined_hsi.save('combined_hsi.png')
