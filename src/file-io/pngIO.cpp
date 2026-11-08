#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "pngIO.hpp"

ImageDetails* readin_png_image(const std::string& filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 4); 

    if (!data) {
        // Handle error
        return nullptr;
    }

    return new ImageDetails(data, width, height);
}

int writeout_png_image(const std::string& filename, const ImageDetails* image_details) {
    int result = stbi_write_png(
        filename.c_str(), 
        image_details->width, 
        image_details->height, 
        4, 
        image_details->image_data, 
        image_details->width * 4
    );
    
    return result; // Returns 1 on success, 0 on failure
}

ImageDetails::~ImageDetails() {
    if (image_data) {
        stbi_image_free(image_data);
    }
}