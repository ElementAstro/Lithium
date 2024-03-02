#include <iostream>

enum ImageArrayElementTypes { INT, FLOAT }; // 假设ImageArrayElementTypes是一个枚举类型

class ImageMetadata {
private:
    int metavers;
    ImageArrayElementTypes imgtype;
    ImageArrayElementTypes xmtype;
    int rank;
    int x_size;
    int y_size;
    int z_size;

public:
    explicit ImageMetadata(
        int metadata_version,
        ImageArrayElementTypes image_element_type,
        ImageArrayElementTypes transmission_element_type,
        int rank,
        int num_x,
        int num_y,
        int num_z
    ) :
        metavers(metadata_version),
        imgtype(image_element_type),
        xmtype(transmission_element_type),
        rank(rank),
        x_size(num_x),
        y_size(num_y),
        z_size(num_z)
    {}

    int get_MetadataVersion() {
        return metavers;
    }

    ImageArrayElementTypes get_ImageElementType() {
        return imgtype;
    }

    ImageArrayElementTypes get_TransmissionElementType() {
        return xmtype;
    }

    int get_Rank() {
        return rank;
    }

    int get_Dimension1() {
        return x_size;
    }

    int get_Dimension2() {
        return y_size;
    }

    int get_Dimension3() {
        return z_size;
    }
};

int main() {
    ImageMetadata metadata(1, INT, FLOAT, 2, 640, 480, 0);
    std::cout << "Metadata version: " << metadata.get_MetadataVersion() << std::endl;
    std::cout << "Image element type: " << metadata.get_ImageElementType() << std::endl;
    std::cout << "Transmission element type: " << metadata.get_TransmissionElementType() << std::endl;
    std::cout << "Rank: " << metadata.get_Rank() << std::endl;
    std::cout << "Dimension 1: " << metadata.get_Dimension1() << std::endl;
    std::cout << "Dimension 2: " << metadata.get_Dimension2() << std::endl;
    std::cout << "Dimension 3: " << metadata.get_Dimension3() << std::endl;

    return 0;
}
