#include "modules/plugin/plugin.hpp"
#include "cimg/CImg.h"

using namespace cimg_library;

class ImageProcessingPlugin : public Plugin
{
public:
    ImageProcessingPlugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description)
        : Plugin(path, version, author, description)
    {
        // 在构造函数中注册图像处理函数
        RegisterFunc("blur", &ImageProcessingPlugin::Blur, this);
        RegisterFunc("rotate", &ImageProcessingPlugin::Rotate, this);
        RegisterFunc("crop", &ImageProcessingPlugin::Crop, this);
        RegisterFunc("sharpen", &ImageProcessingPlugin::Sharpen, this);
        RegisterFunc("white_balance", &ImageProcessingPlugin::WhiteBalance, this);
        RegisterFunc("resize", &ImageProcessingPlugin::Resize, this);
        RegisterFunc("blur", &ImageProcessingPlugin::Blur, this);
        RegisterFunc("rotate", &ImageProcessingPlugin::Rotate, this);
    }

    void Execute(const std::vector<std::string> &args) override
    {
        // 这里可以根据传入的参数决定要执行的具体图像处理操作
        // 例如，根据 args[0] 来判断要执行哪个函数，然后将参数传递给对应的函数
        // 这里只是一个示例，具体实现取决于您的需求
        if (args[0] == "blur")
        {
            // 调用图像处理函数进行模糊处理
            json params = {args[1], std::stoi(args[2])};
            RunFunc("blur", params);
        }
        else if (args[0] == "rotate")
        {
            // 调用图像处理函数进行旋转
            json params = {args[1], std::stoi(args[2])};
            RunFunc("rotate", params);
        }
        else if (args[0] == "crop")
        {
            // 调用图像处理函数进行裁剪
            json params = {args[1], std::stoi(args[2]), std::stoi(args[3]), std::stoi(args[4]), std::stoi(args[5])};
            RunFunc("crop", params);
        }
        else if (args[0] == "sharpen")
        {
            // 调用图像处理函数进行锐化
            json params = {args[1], std::stoi(args[2])};
            RunFunc("sharpen", params);
        }
        else if (args[0] == "white_balance")
        {
            // 调用图像处理函数进行白平衡调整
            json params = {args[1]};
            RunFunc("white_balance", params);
        }
        else if (args[0] == "resize")
        {
            // 调用图像处理函数进行大小调整
            json params = {args[1], std::stoi(args[2]), std::stoi(args[3])};
            RunFunc("resize", params);
        }
        else
        {
            // 未知的操作，可以根据实际需求进行处理
        }
    }

    void Blur(const json &params)
    {
        std::string imagePath = params[0].get<std::string>();
        int radius = params[1].get<int>();

        // 检查缓存中是否存在图像
        if (imageCache.find(imagePath) != imageCache.end())
        {
            // 从缓存中获取图像并进行模糊处理
            CImg<unsigned char> image = imageCache[imagePath];
            image.blur(radius);
            imageCache[imagePath] = image;
        }
        else
        {
            // 加载图像，并进行模糊处理
            CImg<unsigned char> image(imagePath.c_str());
            image.blur(radius);
            imageCache[imagePath] = image;
        }

        // 在这里可以根据实际需求对图像进行进一步处理或保存
        // ...

        // 输出处理后的图像信息
        std::cout << "Image blurred: " << imagePath << std::endl;
    }

    void Rotate(const json &params)
    {
        std::string imagePath = params[0].get<std::string>();
        int angle = params[1].get<int>();

        // 检查缓存中是否存在图像
        if (imageCache.find(imagePath) != imageCache.end())
        {
            // 从缓存中获取图像并进行旋转
            CImg<unsigned char> image = imageCache[imagePath];
            image.rotate(angle);
            imageCache[imagePath] = image;
        }
        else
        {
            // 加载图像，并进行旋转
            CImg<unsigned char> image(imagePath.c_str());
            image.rotate(angle);
            imageCache[imagePath] = image;
        }

        // 在这里可以根据实际需求对图像进行进一步处理或保存
        // ...

        // 输出处理后的图像信息
        std::cout << "Image rotated: " << imagePath << std::endl;
    }

    void Crop(const json &params)
    {
        std::string imagePath = params[0].get<std::string>();
        int x = params[1].get<int>();
        int y = params[2].get<int>();
        int width = params[3].get<int>();
        int height = params[4].get<int>();

        // 检查缓存中是否存在图像
        if (imageCache.find(imagePath) != imageCache.end())
        {
            // 从缓存中获取图像并进行裁剪
            CImg<unsigned char> image = imageCache[imagePath];
            image.crop(x, y, x + width - 1, y + height - 1);
            imageCache[imagePath] = image;
        }
        else
        {
            // 加载图像，并进行裁剪
            CImg<unsigned char> image(imagePath.c_str());
            image.crop(x, y, x + width - 1, y + height - 1);
            imageCache[imagePath] = image;
        }

        // 在这里可以根据实际需求对图像进行进一步处理或保存
        // ...

        // 输出处理后的图像信息
        std::cout << "Image cropped: " << imagePath << std::endl;
    }

    void Sharpen(const json &params)
    {
        std::string imagePath = params[0].get<std::string>();
        int factor = params[1].get<int>();

        // 检查缓存中是否存在图像
        if (imageCache.find(imagePath) != imageCache.end())
        {
            // 从缓存中获取图像并进行锐化
            CImg<unsigned char> image = imageCache[imagePath];
            image.sharpen(factor);
            imageCache[imagePath] = image;
        }
        else
        {
            // 加载图像，并进行锐化
            CImg<unsigned char> image(imagePath.c_str());
            image.sharpen(factor);
            imageCache[imagePath] = image;
        }

        // 在这里可以根据实际需求对图像进行进一步处理或保存
        // ...

        // 输出处理后的图像信息
        std::cout << "Image sharpened: " << imagePath << std::endl;
    }

    void WhiteBalance(const json &params)
    {
        std::string imagePath = params[0].get<std::string>();

        // 检查缓存中是否存在图像
        if (imageCache.find(imagePath) != imageCache.end())
        {
            // 从缓存中获取图像并进行白平衡调整
            CImg<unsigned char> image = imageCache[imagePath];

            // 计算每个通道的平均值
            double r = 0, g = 0, b = 0;
            cimg_forXY(image, x, y)
            {
                r += image(x, y, 0);
                g += image(x, y, 1);
                b += image(x, y, 2);
            }
            int size = image.width() * image.height();
            r /= size;
            g /= size;
            b /= size;

            // 调整每个通道的白平衡
            cimg_forXY(image, x, y)
            {
                double factor_r = r / image(x, y, 0);
                double factor_g = g / image(x, y, 1);
                double factor_b = b / image(x, y, 2);
                image(x, y, 0) = cimg::cut(image(x, y, 0) * factor_r, 0, 255);
                image(x, y, 1) = cimg::cut(image(x, y, 1) * factor_g, 0, 255);
                image(x, y, 2) = cimg::cut(image(x, y, 2) * factor_b, 0, 255);
            }

            imageCache[imagePath] = image;
        }
        else
        {
            // 加载图像，并进行白平衡调整
            CImg<unsigned char> image(imagePath.c_str());

            // 计算每个通道的平均值
            double r = 0, g = 0, b = 0;
            cimg_forXY(image, x, y)
            {
                r += image(x, y, 0);
                g += image(x, y, 1);
                b += image(x, y, 2);
            }
            int size = image.width() * image.height();
            r /= size;
            g /= size;
            b /= size;

            // 调整每个通道的白平衡
            cimg_forXY(image, x, y)
            {
                double factor_r = r / image(x, y, 0);
                double factor_g = g / image(x, y, 1);
                double factor_b = b / image(x, y, 2);
                image(x, y, 0) = cimg::cut(image(x, y, 0) * factor_r, 0, 255);
                image(x, y, 1) = cimg::cut(image(x, y, 1) * factor_g, 0, 255);
                image(x, y, 2) = cimg::cut(image(x, y, 2) * factor_b, 0, 255);
            }

            imageCache[imagePath] = image;
        }

        // 在这里可以根据实际需求对图像进行进一步处理或保存
        // ...

        // 输出处理后的图像信息
        std::cout << "Image white balanced: " << imagePath << std::endl;
    }

    void Resize(const json &params)
    {
        std::string imagePath = params[0].get<std::string>();
        int width = params[1].get<int>();
        int height = params[2].get<int>();

        // 检查缓存中是否存在图像
        if (imageCache.find(imagePath) != imageCache.end())
        {
            // 从缓存中获取图像并进行大小调整
            CImg<unsigned char> image = imageCache[imagePath];
            image.resize(width, height);
            imageCache[imagePath] = image;
        }
        else
        {
            // 加载图像，并进行大小调整
            CImg<unsigned char> image(imagePath.c_str());
            image.resize(width, height);
            imageCache[imagePath] = image;
        }

        // 在这里可以根据实际需求对图像进行进一步处理或保存
        // ...

        // 输出处理后的图像信息
        std::cout << "Image resized: " << imagePath << std::endl;
    }

private:
    mutable std::unordered_map<std::string, CImg<unsigned char>> imageCache;
};

int main()
{
    // 创建图像处理插件对象
    ImageProcessingPlugin imageProcessingPlugin("path/to/plugin", "1.0", "Author", "Image processing plugin");

    // 执行图像处理操作
    std::vector<std::string> args = {"blur", "image.jpg", "5"};
    imageProcessingPlugin.Execute(args);

    return 0;
}
