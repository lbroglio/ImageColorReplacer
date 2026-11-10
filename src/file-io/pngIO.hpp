#pragma once

#include <string>
#include <cstdint>


struct ImageDetails {
    std::uint32_t width;
    std::uint32_t height;
    unsigned char* image_data;

    ImageDetails(unsigned char* data, std::uint32_t w, std::uint32_t h) : width(w), height(h), image_data(data) {}

    ImageDetails(const ImageDetails&) = delete;
    ImageDetails& operator=(const ImageDetails&) = delete;

    ~ImageDetails();
};


/**
 * @brief Loads a PNG image from a file
 * 
 * @param filename The path to the PNG file
 * @return ImageDetails* A pointer to the details of the loaded image
 */
ImageDetails* readin_png_image(const std::string& filename);

/**
 * @brief Saves an image to a PNG file
 * 
 * @param filename The path where the PNG file will be saved
 * @param image_details A pointer to the details of the image to be saved
 * @return int Returns 1 on success, 0 on failure
 */
int writeout_png_image(const std::string& filename, const ImageDetails* image_details);