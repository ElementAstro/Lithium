#include <iostream>
#include <vector>
#include <string>

#include "Plugin.h"
#include "stb_image.h"

class ImageProcessingPlugin : public Plugin
{
public:
    ImageProcessingPlugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description)
        : Plugin(path, version, author, description)
    {
    }

    void Execute(const std::vector<std::string> &args) override
    {
        if (args.empty())
        {
            std::cout << "Usage: image_processing_plugin <image_path>" << std::endl;
            return;
        }

        std::string imagePath = args[0];

        // Load the image using stb_image
        int width, height, channels;
        unsigned char *imageData = stbi_load(imagePath.c_str(), &width, &height, &channels, 0);
        if (!imageData)
        {
            std::cout << "Failed to load image: " << imagePath << std::endl;
            return;
        }

        // Perform image processing operations here
        // Example: Convert the image to grayscale
        unsigned char *grayImageData = convertToGrayscale(imageData, width, height, channels);

        // Save the processed image
        std::string outputPath = "processed_image.jpg";
        if (stbi_write_jpg(outputPath.c_str(), width, height, 1, grayImageData, 100) == 0)
        {
            std::cout << "Failed to save processed image." << std::endl;
        }
        else
        {
            std::cout << "Processed image saved to: " << outputPath << std::endl;
        }

        // Free the allocated memory
        stbi_image_free(imageData);
        stbi_image_free(grayImageData);
    }

private:
    unsigned char *convertToGrayscale(unsigned char *imageData, int width, int height, int channels)
    {
        unsigned char *grayImageData = new unsigned char[width * height];

        if (channels == 1)
        {
            memcpy(grayImageData, imageData, width * height);
        }
        else if (channels == 3)
        {
            for (int i = 0; i < width * height; ++i)
            {
                unsigned char r = imageData[i * channels];
                unsigned char g = imageData[i * channels + 1];
                unsigned char b = imageData[i * channels + 2];
                unsigned char gray = static_cast<unsigned char>(0.2989 * r + 0.587 * g + 0.114 * b);
                grayImageData[i] = gray;
            }
        }
        else if (channels == 4)
        {
            for (int i = 0; i < width * height; ++i)
            {
                unsigned char r = imageData[i * channels];
                unsigned char g = imageData[i * channels + 1];
                unsigned char b = imageData[i * channels + 2];
                unsigned char a = imageData[i * channels + 3];
                unsigned char gray = static_cast<unsigned char>(0.2989 * r + 0.587 * g + 0.114 * b);
                grayImageData[i] = gray;
            }
        }

        return grayImageData;
    }
};

// Example usage
int main()
{
    ImageProcessingPlugin plugin("path/to/plugin", "1.0", "Author", "Image processing plugin");
    std::vector<std::string> args = {"path/to/image.jpg"};
    plugin.Execute(args);

    return 0;
}
